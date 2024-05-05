// STL header files
#include <functional>
#include <QTextLayout>
#include <QTextCharFormat>

// Other header files
#include "Evaluator.h"
#include "Rendering.h"

/*!
 * \internal
 *
 * \brief The TileScreenPlacement class is a class that describes a tiles
 * position and size within the viewport.
 */
struct TileScreenPlacement {
    double pixelPosX;
    double pixelPosY;
    double pixelWidth;
};

/*!
 * \internal
 *
 * \brief The TilePosCalculator class
 * is a helper class for positioning tiles within the viewport.
 */
struct TilePosCalculator {
    int vpWidth;
    int vpHeight;
    double vpX;
    double vpY;
    double vpZoom;
    int mapZoom;

private:
    // Largest dimension between viewport height and width, expressed in pixels.
    int VpMaxDim() const { return qMax(vpWidth, vpHeight); }

    // Aspect ratio of the viewport, as a scalar.
    double VpAspect() const { return (double)vpWidth / (double)vpHeight; }

    // The scale of the world map as a scalar fraction of the viewport.
    // Example: A value of 2 means the world map can fit 2 viewports in X and Y directions.
    double WorldmapScale() const { return pow(2, vpZoom); }

    // Size of an individual tile as a fraction of the world map.
    //
    // Exmaple: A value of 0.5 means the tile takes up half the length of the world map
    // in X and Y directions.
    double TileSizeNorm() const { return WorldmapScale() / (1 << mapZoom); }

public:
    /*!
     * \brief calcTileSizeData
     * Calculates the on-screen position information of a specific tile.
     *
     * \param coord The cooardinates of the tile wanted.
     * \return The TileScreenPlacement with the correct data.
     */
    TileScreenPlacement calcTileSizeData(TileCoord coord) const {
        // Calculate where the top-left origin of the world map is relative to the viewport.
        double worldOriginX = vpX * WorldmapScale() - 0.5;
        double worldOriginY = vpY * WorldmapScale() - 0.5;

        // Adjust the world such that our worldmap is still centered around our center-coordinate
        // when the aspect ratio changes.
        if (VpAspect() < 1.0) {
            worldOriginX += -0.5 * VpAspect() + 0.5;
        } else if (VpAspect() > 1.0) {
            worldOriginY += -0.5 / VpAspect() + 0.5;
        }

        // The position of this tile expressed in world-normalized coordinates.
        double posNormX = (coord.x * TileSizeNorm()) - worldOriginX;
        double posNormY = (coord.y * TileSizeNorm()) - worldOriginY;

        TileScreenPlacement out;
        out.pixelPosX = posNormX * VpMaxDim();
        out.pixelPosY = posNormY * VpMaxDim();

        // Calculate the width of a tile as it's displayed on-screen.
        // (Height is same as width, perfectly square)
        out.pixelWidth = TileSizeNorm() * VpMaxDim();

        return out;
    }
};

/*!
 * \internal
 * \brief createTilePosCalculator
 * Constructs a TilePosCalculator object based on the current state of the viewport.
 *
 * \param vpWidth The width of the viewport in pixels on screen.
 * \param vpHeight The height of the viewport in pixels on screen.
 * \param vpX The center X coordinate of the viewport in world-normalized coordinates.
 * \param vpY The center Y coordinate of the viewport in world-normalized coordinates.
 * \param vpZoom The current zoom-level of the viewport.
 * \param mapZoom The current zoom-level of the map.
 * \return
 */
static TilePosCalculator createTilePosCalculator(
    int vpWidth,
    int vpHeight,
    double vpX,
    double vpY,
    double vpZoom,
    int mapZoom)
{
    TilePosCalculator out;
    out.vpWidth = vpWidth;
    out.vpHeight = vpHeight;
    out.vpX = vpX;
    out.vpY = vpY;
    out.vpZoom = vpZoom;
    out.mapZoom = mapZoom;
    return out;
}

/*!
 * \brief Bach::PaintVectorTileSettings::getDefault
 * Builds the default settings for painting vector tiles.
 */
Bach::PaintVectorTileSettings Bach::PaintVectorTileSettings::getDefault()
{
    Bach::PaintVectorTileSettings out;
    out.drawFill = true;
    out.drawLines = true;
    out.drawText = true;
    return out;
}

/*!
 * \internal
 * \threadsafe
 *
 * \brief paintSingleTileDebug
 * This is a helper function for drawing visualization of the boundaries of a tile.
 *
 * \param painter
 * This function assumes the painter's transform has moved the origin of the
 * canvas to the tile's origin, and has NOT been scaled.
 *
 * \param tileCoord Coordinates of this particular tile. Only used for printing.
 * \param tileWidthPixels The size of one tile in terms of pixels on screen.
 */
static void paintSingleTileDebug(
    QPainter &painter,
    const TileCoord &tileCoord,
    double tileWidthPixels)
{
    painter.setPen(Qt::darkGreen);

    painter.drawText(10, 20, tileCoord.toString());

    // From here on we want to draw geometry with normalized coordinates.
    QTransform transform;
    transform.scale(tileWidthPixels, tileWidthPixels);

    // Draw the X in the middle of the tile first, by using tile-normalized coordinates
    // from 0.45 to 0.55.
    painter.drawLine(transform.map(QLineF{ QPointF(0.45, 0.45), QPointF(0.55, 0.55) }));
    painter.drawLine(transform.map(QLineF{ QPointF(0.55, 0.45), QPointF(0.45, 0.55) }));

    // Then draw the boundary of the tile.
    // We do a save and restore because we need to set the
    // brush to not draw anything.
    {
        painter.save();
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(transform.mapRect(QRectF(0, 0, 1, 1)));
        painter.restore();
    }
}

/*!
 * \internal
 *
 * \brief isLayerShow
 * Determines whether we should skip this layer during rendering.
 *
 * \param layerStyle The Layer we are considering.
 *
 * \param mapZoom The current zoom of the map.
 *
 * \return True if the layer should be rendered.
 */
static bool isLayerShown(const AbstractLayerStyle &layerStyle, int mapZoom)
{
    return
        layerStyle.m_visibility == "visible" &&
        mapZoom < layerStyle.m_maxZoom &&
        mapZoom >= layerStyle.m_minZoom;
}

/*!
 * \brief includeFeature
 * Determines whether a map feature should be rendered given the layer style.
 *
 * \param layerStyle The layer that the feature is part of.
 * \param feature The feature we are considering.
 * \param mapZoom Map zoom level
 * \param vpZoom Viewport zoom level
 * \return Returns true if this feature should be rendered.
 */
static bool includeFeature(
    const AbstractLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
    int mapZoom,
    double vpZoom)
{
    if (layerStyle.m_filter.isEmpty())
        return true;
    return Evaluator::resolveExpression(
        layerStyle.m_filter,
        &feature,
        mapZoom,
        vpZoom).toBool();
}

/*!
 * \brief paintVectorLayer_Fill
 * Call the polygon rendering function on all the layer's features that pass the layerStyle filter
 *
 * \param painter
 * The painter object to paint into.
 * It assumes the painter object has had its origin moved to the tiles origin, and is unscaled.
 * \param layerStyle the layerStyle to be used to filter/style this layer's features.
 * \param layer the TileLayer containing the features to be rendered.
 * \param mapZoom The map zoom level being rendered.
 * \param vpZoom The zoom level of the viewport.
 * \param geometryTransform the transform to be used to map the features into the correct position.
 */
static void paintVectorLayer_Fill(
    QPainter &painter,
    const FillLayerStyle &layerStyle,
    const TileLayer& layer,
    double vpZoom,
    int mapZoom,
    QTransform geometryTransform)
{
    // Iterate over all the features, and filter out anything that is not fill.
    for (const std::unique_ptr<AbstractLayerFeature> &abstractFeature : layer.m_features) {
        if (abstractFeature->type() != AbstractLayerFeature::featureType::polygon)
            continue;

        const auto &feature = *static_cast<const PolygonFeature*>(abstractFeature.get());

        if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
            continue;

        // Render the feature in question.
        painter.save();
        Bach::paintSingleTileFeature_Polygon({&painter, &layerStyle, &feature, mapZoom, vpZoom, geometryTransform});
        painter.restore();
    }
}

/*!
 * \brief paintVectorLayer_Line
  * Call the line rendering function on all the layer's features that pass the layerStyle filter
 *
 * \param painter
 * The painter object to paint into.
 * It assumes the painter object has had its origin moved to the tiles origin, and is unscaled.
 * \param layerStyle the layerStyle to be used to filter/style this layer's features.
 * \param layer the TileLayer containing the features to be rendered.
 * \param mapZoom The map zoom level being rendered.
 * \param vpZoom The zoom level of the viewport.
 * \param geometryTransform the transform to be used to map the features into the correct position.
 */
static void paintVectorLayer_Line(
    QPainter &painter,
    const LineLayerStyle &layerStyle,
    const TileLayer& layer,
    double vpZoom,
    int mapZoom,
    QTransform geometryTransform)
{
    // Iterate over all the features, and filter out anything that is not line.
    for (const std::unique_ptr<AbstractLayerFeature> &abstractFeature : layer.m_features) {
        if (abstractFeature->type() != AbstractLayerFeature::featureType::line)
            continue;
        const auto &feature = *static_cast<const LineFeature*>(abstractFeature.get());

        // Tests whether the feature should be rendered at all based on possible expression.
        if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
            continue;

        // Render the feature in question.
        painter.save();
        Bach::paintSingleTileFeature_Line({&painter, &layerStyle, &feature, mapZoom, vpZoom, geometryTransform});
        painter.restore();
    }
}

/*!
 * \brief processVectorLayer_Point
 * Call the text processing function on all the layer's features that pass the layerStyle filter.
 * For normal text, the processing function is called after that the features have been ordered according to the rank property.
 * This will update the vpTextList list with the text that passes the global collision filtering.
 * \param painter
 * The painter object to paint into.
 * It assumes the painter object has had its origin moved to the tiles origin, and is unscaled.
 * \param layerStyle the layerStyle to be used to filter/style this layer's features.
 * \param layer the TileLayer containing the features to be rendered.
 * \param mapZoom The map zoom level being rendered.
 * \param vpZoom The zoom level of the viewport.
 * \param geometryTransform the transform to be used to map the features into the correct position.
 * \param forceNoChangeFontType If set to true, the text font
 * rendered will be the one currently set by the QPainter object.
 * If set to false, it will try to use the font suggested by the stylesheet.
 * \param labelRects The list containing the bounding rectangles for all the text features that  have
 * been processed. This bounding rects have view port coordinates rather than tile coordinates, which means
 * that the collision detection will check for all the text in the map widget and not only the text in the current tile.
 * \param vpTextList a list of structs that contain all the texts that passed the collision filtering along with all the
 * details necessary to render the text.
 * \param vpCurvedTextList a list of structs that contain all the curved texts that passed the collision filtering along with all the
 * details necessary to render the text.
 */
static void processVectorLayer_Point(
    QPainter &painter,
    const SymbolLayerStyle &layerStyle,
    const TileLayer& layer,
    double vpZoom,
    int mapZoom,
    int tileWidthPixels,
    int tileOriginX,
    int tileOriginY,
    QTransform geometryTransform,
    bool forceNoChangeFontType,
    QVector<QRect> &labelRects,
    QVector<Bach::vpGlobalText> &vpTextList,
    QVector<Bach::vpGlobalCurvedText> &vpCurvedTextList)
{
    QVector<QPair<int, PointFeature>> labels; //Used to order text rendering operation based on "rank" property.
    // Iterate over all the features, and filter out anything that is not point.
    for (const std::unique_ptr<AbstractLayerFeature> &abstractFeature : layer.m_features) {
        if(abstractFeature->type() == AbstractLayerFeature::featureType::line){
            const LineFeature &feature = *static_cast<const LineFeature*>(abstractFeature.get());
            //Bach::paintSingleTileFeature_Point_Curved({&painter, &layerStyle, &feature, mapZoom, vpZoom, geometryTransform});
            Bach::processSingleTileFeature_Point_Curved(
                {&painter, &layerStyle, &feature, mapZoom, vpZoom, geometryTransform},
                tileWidthPixels,
                tileOriginX,
                tileOriginY,
                labelRects,
                vpCurvedTextList);
        }else if (abstractFeature->type() == AbstractLayerFeature::featureType::point){ //For normal text (continents /countries / cities / places / ...)
            const PointFeature &feature = *static_cast<const PointFeature*>(abstractFeature.get());
            // Tests whether the feature should be rendered at all based on possible expression.
            if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
                continue;

            //Add the feature along with its "rank" (if present, defaults to 100) to the labels map.
            if(feature.featureMetaData.contains("rank")){
                labels.append(QPair<int, PointFeature>(feature.featureMetaData["rank"].toInt(), feature));
            }else{
                labels.append(QPair<int, PointFeature>(100, feature));
            }
        }
    }

    //Sort the labels map in increasing order based on the laber's "rank"
    std::stable_sort(labels.begin(), labels.end(), [](const QPair<int, PointFeature>& a, const QPair<int, PointFeature>& b) {
        return a.first < b.first;
    });

    //Loop over the ordered label map and add the text that passes the collision filter to the vpTextList list.
    for(const auto &pair : labels){
        painter.save();
        Bach::processSingleTileFeature_Point(
            {&painter, &layerStyle, &pair.second, mapZoom, vpZoom, geometryTransform},
            tileWidthPixels,
            tileOriginX,
            tileOriginY,
            forceNoChangeFontType,
            labelRects,
            vpTextList);
        painter.restore();
    }
    labels.clear();
}

/*!
 * \brief paintText
 * Loop over all the text elements that passed the collision filter and render them on screen.
 * \param painter the painter to be used for text rendering.
 * \param vpTextList the list of structs containing the necessary elments to render the text.
 * \param params this containes the bool used to switch text rendering methods.
 */
static void paintText(
    QPainter &painter,
    QVector<Bach::vpGlobalText> &vpTextList,
    Bach::PaintVectorTileSettings params)
{

    QPen pen;
    QTextCharFormat charFormat;
    QTextLayout::FormatRange formatRange;
    QTextLayout textLayout;
    QString text;

    for(auto const &globalText : vpTextList){
        painter.save();
        //Remove any translations/scaling previously done on the painter's transform.
        painter.resetTransform();
        painter.setClipping(false);
        //move the painter to the origin of the tile that the text belongs to since the coordinates
        //of each text element is relative to its parent tile rather than the viewport.
        painter.translate(globalText.tileOrigin);
        QFontMetrics fmetrics(globalText.font);
        for(int i = 0; i < globalText.text.size(); i++){
            text = globalText.text.at(i);
            //Set the pen to be used for text outline
            pen.setWidth(globalText.outlineSize);
            pen.setColor(globalText.outlineColor);
            //Set the text layout parameters
            textLayout.setText(text);
            textLayout.setFont(globalText.font);
            //Set the formatRange parameters
            charFormat.setTextOutline(pen);
            formatRange.format = charFormat;
            formatRange.length = text.length();
            formatRange.start = 0;
            painter.setPen(globalText.textColor);
            textLayout.beginLayout();
            textLayout.createLine();
            textLayout.endLayout();
            //Corrected text position
            QPointF textPosition(globalText.position.at(i).x(), globalText.position.at(i).y() - fmetrics.height()/2);
            textLayout.draw(&painter, textPosition, {formatRange},QRect(0, 0, 0, 0));

        }
        painter.restore();

    }
}

/*!
 * \brief paintText_Curved
 * Loop over all the text elements in the curved text list that passed the collision filter and render them on screen.
 * \param painter the painter to be used for text rendering.
 * \param vpTextList the list of structs containing the necessary elments to render the text.
 * \param params this containes the bool used to switch text rendering methods.
 */
static void paintText_Curved(
    QPainter &painter,
    QVector<Bach::vpGlobalCurvedText> &vpCurvedTextList)
{

    QPen pen;
    QTextCharFormat charFormat;
    QTextLayout::FormatRange formatRange;
    QTextLayout textLayout;

    for(auto const &globalText : vpCurvedTextList){
        painter.save();
        //Remove any translations/scaling previously done on the painter's transform.
        painter.resetTransform();
        painter.setClipping(false);
        //move the painter to the origin of the tile that the text belongs to since the coordinates
        //of each text element is relative to its parent tile rather than the viewport.
        painter.translate(globalText.tileOrigin);
            for(const auto &text : globalText.textList){
                QFontMetrics fmetrics(globalText.font);
                //Set the pen to be used for text outline
                pen.setWidth(globalText.outlineSize);
                pen.setColor(globalText.outlineColor);
                //Set the formatRange parameters
                charFormat.setTextOutline(pen);
                formatRange.format = charFormat;
                formatRange.length = 1;
                formatRange.start = 0;
                //Set the painter rendering parameters
                painter.save();
                painter.setOpacity(globalText.opacity);
                painter.setPen(globalText.textColor);
                painter.translate(text.position);
                painter.rotate(text.angle);
                //Set the text layout parameters
                textLayout.setText(text.character);
                textLayout.setFont(globalText.font);
                textLayout.beginLayout();
                textLayout.createLine();
                textLayout.endLayout();
                //Get the corrected text position
                QPoint offsetTextPos(0, -fmetrics.height());
                textLayout.draw(&painter, offsetTextPos, {formatRange},QRect(0, 0, 0, 0));
                textLayout.clearLayout();
                painter.restore();
            }
        painter.restore();

    }
}


/*!
 * \internal
 *
 * \brief paintVectorTile
 * Paints a single tile using vector-graphics.
 *
 * This is called repeatedly from the 'paintTiles' function.
 *
 * This does not handle background color.
 *
 * \param tileData The vector-data for this tile.
 *
 * \param painter
 * The painter object to paint into.
 * It assumes the painter object has had its origin moved to the tiles origin, and is unscaled.
 *
 * \param mapZoom The map zoom level being rendered.
 * \param vpZoom The zoom level of the viewport.
 * \param styleSheet
 * \param tileWidthPixels The width of the tile in pixels.
 * \param tileOriginX the x component of the tile's origin (used for text collistion detection)
 * \param tileOriginY the y component of the tile's origin (used for text collistion detection)
 * \param settings
 * \param labelRects the list containing all the bounding rectangle for previously processed text feratures (used for text collistion detection)
 * \param vpTextList the list containing all the text elements for all the tiles currently visible on the view port
 */
static void paintVectorTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoom,
    double vpZoom,
    const StyleSheet &styleSheet,
    TileScreenPlacement tileScreenPlacement,
    const Bach::PaintVectorTileSettings &settings,
    QVector<QRect> &labelRects,
    QVector<Bach::vpGlobalText> &vpTextList,
    QVector<Bach::vpGlobalCurvedText> &vpCurvedTextList)
{
    QTransform geometryTransform;
    geometryTransform.scale(
        tileScreenPlacement.pixelWidth,
        tileScreenPlacement.pixelWidth);

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (const std::unique_ptr<AbstractLayerStyle> &abstractLayerStylePtr : styleSheet.m_layerStyles) {
        const AbstractLayerStyle *abstractLayerStyle = abstractLayerStylePtr.get();
        if (!isLayerShown(*abstractLayerStyle, mapZoom))
            continue;

        // Check if this layer style has an associated layer in the tile.
        auto layerIt = tileData.m_layers.find(abstractLayerStyle->m_sourceLayer);
        if (layerIt == tileData.m_layers.end()) {
            continue;
        }
        // If we find it, we dereference it to access it's data.
        const TileLayer& layer = *layerIt->second;

        // We do different types of rendering based on whether the layer is a polygon, line, or symbol(text).
        if (abstractLayerStyle->type() == AbstractLayerStyle::LayerType::fill) {
            if (!settings.drawFill)
                continue;
            paintVectorLayer_Fill(
                painter,
                *static_cast<const FillLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                geometryTransform);

        } else if (abstractLayerStyle->type() == AbstractLayerStyle::LayerType::line) {
            if (!settings.drawLines)
                continue;
            paintVectorLayer_Line(
                painter,
                *static_cast<const LineLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                geometryTransform);
        } else if(abstractLayerStyle->type() == AbstractLayerStyle::LayerType::symbol){
            if (!settings.drawText)
                continue;
            processVectorLayer_Point(
                painter,
                *static_cast<const SymbolLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                tileScreenPlacement.pixelWidth,
                tileScreenPlacement.pixelPosX,
                tileScreenPlacement.pixelPosY,
                geometryTransform,
                settings.forceNoChangeFontType,
                labelRects,
                vpTextList,
                vpCurvedTextList);
        }
    }
}

/*!
 * \brief drawBackgroundColor
 * Draws the background color of the stylesheet to the Painter object.
 *
 * \param painter The painter object we want to render into.
 * \param styleSheet The stylesheet of the map we are rendering.
 * \param mapZoom The current zoom of the map.
 */
static void drawBackgroundColor(
    QPainter &painter,
    const StyleSheet &styleSheet,
    int mapZoom)
{
    bool styleFound = false;
    QColor color;

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (const std::unique_ptr<AbstractLayerStyle> &abstractLayerStylePtr : styleSheet.m_layerStyles) {
        AbstractLayerStyle *abstractLayerStyle = abstractLayerStylePtr.get();
        // Background is a special case and has no associated layer.
        // We just draw it and move onto the next layer style.
        if (abstractLayerStyle->type() == AbstractLayerStyle::LayerType::background) {
            // Fill the entire tile with a single color
            const BackgroundStyle& layerStyle = *static_cast<const BackgroundStyle*>(abstractLayerStyle);

            color = layerStyle.getColorAtZoom(mapZoom).value<QColor>();
            styleFound = true;
            break;
        }
    }

    if (styleFound) {
        painter.fillRect(
            0,
            0,
            painter.window().width(),
            painter.window().height(),
            color);
    } else {
        qWarning() << "No background color found while drawing. Possible bug.\n";
    }
}

/*!
 * \internal
 * \brief A helper class for painting vector-tiles and raster-tiles while reusing code.
 *
 * Places tiles correctly on screen and also draws the debug boundaries.
 *
 * This function does not take care of rendering background.
 *
 * \param painter The painter object to draw into
 * \param vpX center-coordinate X of the viewport in world-normalized coordinates.
 * \param vpY center-coordinate Y of the viewport in world-normalized coordinates.
 * \param vpZoom Zoom level of the viewport.
 * \param mapZoom Zoom level of the map.
 * \param paintSingleTileFn The function to call to draw a single tile.
 */
static void paintTilesGeneric(
    QPainter &painter,
    double vpX,
    double vpY,
    double vpZoom,
    int mapZoom,
    const std::function<void(TileCoord, TileScreenPlacement)> &paintSingleTileFn,
    const StyleSheet &styleSheet,
    bool drawDebug)
{
    // Start by drawing the background color on the entire canvas.
    drawBackgroundColor(painter, styleSheet, mapZoom);

    // Gather viewport width and height, in pixels.
    int vpWidth = painter.window().width();
    int vpHeight = painter.window().height();

    TilePosCalculator tilePosCalc = createTilePosCalculator(
        vpWidth,
        vpHeight,
        vpX,
        vpY,
        vpZoom,
        mapZoom);

    // Aspect ratio of the viewport.
    double vpAspect = (double)vpWidth / (double)vpHeight;
    // Calculate the set of visible tiles that fit in the viewport.
    QVector<TileCoord> visibleTiles = Bach::calcVisibleTiles(
        vpX,
        vpY,
        vpAspect,
        vpZoom,
        mapZoom);

    // Iterate over all possible tiles that can possibly fit in this viewport.

    for (TileCoord tileCoord : visibleTiles) {
        TileScreenPlacement tilePlacement = tilePosCalc.calcTileSizeData(tileCoord);

        painter.save();

        // We move the origin point of the painter to the top-left of the tile.
        // We do not apply scaling because it interferes with other sized elements,
        // like the size of pen width.
        painter.translate(tilePlacement.pixelPosX, tilePlacement.pixelPosY);

        // We create a clip rect around our tile, as to only render into
        // the region on-screen the tile occupies.
        painter.setClipRect(QRectF{
            0,
            0,
            tilePlacement.pixelWidth,
            tilePlacement.pixelWidth });

        // Draw the single tile.
        paintSingleTileFn(tileCoord, tilePlacement);

        // Paint debug lines around the tile.
        if (drawDebug) {
            paintSingleTileDebug(
                painter,
                tileCoord,
                tilePlacement.pixelWidth);
        }
        painter.restore();
    }
}

/*!
 * \brief Bach::paintVectorTiles renders multiple vector tiles to a QPainter object.
 *
 *  The function will iterate over multiple tiles and place them correctly on screen
 *  and render them.
 *
 * \param painter is a QPainter object to draw into.
 * \param vpX
 * \param vpY
 * \param viewportZoomLevel
 * \param mapZoomLevel
 * \param tileContainer contains all the tile-data available at this point in time.
 * \param styleSheet contains layer styling data.
 * \param drawDebug determines if debug lines should be drawn or not.
 */
void Bach::paintVectorTiles(
    QPainter &painter,
    double vpX,
    double vpY,
    double viewportZoom,
    int mapZoom,
    const QMap<TileCoord, const VectorTile*> &tileContainer,
    const StyleSheet &styleSheet,
    const PaintVectorTileSettings &settings,
    bool drawDebug)
{
    QVector<QRect> labelRects;
    QVector<Bach::vpGlobalText> vpTextList;
    QVector<Bach::vpGlobalCurvedText> vpCurvedTextList;
    auto paintSingleTileFn = [&](TileCoord tileCoord, TileScreenPlacement tilePlacement) {
        // See if the tile being rendered has any tile-data associated with it.
        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt == tileContainer.end())
            return;

        const VectorTile &tileData = **tileIt;
        paintVectorTile(
            tileData,
            painter,
            mapZoom,
            viewportZoom,
            styleSheet,
            tilePlacement,
            settings,
            labelRects,
            vpTextList,
            vpCurvedTextList);
    };

    paintTilesGeneric(
        painter,
        vpX,
        vpY,
        viewportZoom,
        mapZoom,
        paintSingleTileFn,
        styleSheet,
        drawDebug);

    //After rendering all the other layers , we render all the text that should be currently visible on the viewport.
    paintText(painter, vpTextList, settings);
    paintText_Curved(painter, vpCurvedTextList);
}

/*!
 *  \brief paintRasterTiles
 *  Paints all tiles into a painter object, using raster-graphics.
 */
void Bach::paintRasterTiles(
    QPainter &painter,
    double vpX,
    double vpY,
    double viewportZoomLevel,
    int mapZoomLevel,
    const QMap<TileCoord, const QImage*> &tileContainer,
    const StyleSheet &styleSheet,
    bool drawDebug)
{
    auto paintSingleTileFn = [&](TileCoord tileCoord, TileScreenPlacement tilePlacement) {
        // See if the tile being rendered has any tile-data associated with it.
        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt == tileContainer.end())
            return;

        const QImage &tileData = **tileIt;
        QRectF target {
            0,
            0,
            tilePlacement.pixelWidth,
            tilePlacement.pixelWidth, };
        painter.drawImage(target, tileData);
    };

    paintTilesGeneric(
        painter,
        vpX,
        vpY,
        viewportZoomLevel,
        mapZoomLevel,
        paintSingleTileFn,
        styleSheet,
        drawDebug);
}
