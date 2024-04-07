#include <functional>

#include "Evaluator.h"
#include "Rendering.h"

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
    int tileWidthPixels)
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
 * \brief isLayerHidden
 * Determines whether we should skip this layer during rendering.
 *
 * \param layerStyle
 *
 * \param mapZoom
 *
 * \return True if the layer should NOT be rendered.
 */
static bool isLayerHidden(const AbstractLayerStyle &layerStyle, int mapZoom)
{
    return
        layerStyle.m_visibility == "none" ||
        layerStyle.m_maxZoom < mapZoom ||
        layerStyle.m_minZoom >= mapZoom;
}

/*!
 * \brief includeFeature
 * Determines whether a map feature should be included given the layer style.
 *
 * \param layerStyle
 * \param feature
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
    for (const AbstractLayerFeature *abstractFeature : layer.m_features) {
        if (abstractFeature->type() != AbstractLayerFeature::featureType::polygon)
            continue;

        const auto &feature = *static_cast<const PolygonFeature*>(abstractFeature);

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
    for (const AbstractLayerFeature *abstractFeature : layer.m_features) {
        if (abstractFeature->type() != AbstractLayerFeature::featureType::line)
            continue;
        const auto &feature = *static_cast<const LineFeature*>(abstractFeature);

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
 * \brief paintVectorLayer_Point
 * Call the text rendering function on all the layer's features that pass the layerStyle filter.
 * The rendering function is called after that the features have been ordered according to the rank property.
 * \param painter
 * The painter object to paint into.
 * It assumes the painter object has had its origin moved to the tiles origin, and is unscaled.
 * \param layerStyle the layerStyle to be used to filter/style this layer's features.
 * \param layer the TileLayer containing the features to be rendered.
 * \param mapZoom The map zoom level being rendered.
 * \param vpZoom The zoom level of the viewport.
 * \param geometryTransform the transform to be used to map the features into the correct position.
 * \param labelRects
 */
static void paintVectorLayer_Point(
    QPainter &painter,
    const SymbolLayerStyle &layerStyle,
    const TileLayer& layer,
    double vpZoom,
    int mapZoom,
    int tileWidthPixels,
    QTransform geometryTransform,
    QVector<QRect> &labelRects)
{
    QVector<QPair<int, PointFeature>> labels; //Used to order text rendering operation based on "rank" property.
    // Iterate over all the features, and filter out anything that is not point  (rendering of line features for curved text in the symbol layer is not yet implemented).
    for (auto const& abstractFeature : layer.m_features) {
        if (abstractFeature->type() != AbstractLayerFeature::featureType::point) //For normal text (continents /countries / cities / places / ...)
            continue;
        const auto &feature = *static_cast<const PointFeature*>(abstractFeature);

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

    //Sort the labels map in increasing order based on the laber's "rank"
    std::sort(labels.begin(), labels.end(), [](const QPair<int, PointFeature>& a, const QPair<int, PointFeature>& b) {
        return a.first < b.first;
    });

    //Loop over the ordered label map and render text ignoring labels that would cause an overlap.
    for(const auto &pair : labels){
        painter.save();
        Bach::paintSingleTileFeature_Point(
            {&painter, &layerStyle, &pair.second, mapZoom, vpZoom, geometryTransform},
            tileWidthPixels,
            labelRects);
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
 * \param tileSize The width of the tile in pixels.
 */
static void paintVectorTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoom,
    double vpZoom,
    const StyleSheet &styleSheet,
    int tileWidthPixels)
{
    QTransform geometryTransform;
    geometryTransform.scale(tileWidthPixels, tileWidthPixels);
    QVector<QRect> labelRects; //Used to prevent text overlapping.

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (const AbstractLayerStyle *abstractLayerStyle : styleSheet.m_layerStyles) {
        if (isLayerHidden(*abstractLayerStyle, mapZoom))
            continue;

        // Check if this layer style has an associated layer in the tile.
        auto layerIt = tileData.m_layers.find(abstractLayerStyle->m_sourceLayer);
        if (layerIt == tileData.m_layers.end()) {
            continue;
        }
        // If we find it, we dereference it to access it's data.
        const TileLayer& layer = **layerIt;

        // We do different types of rendering based on whether the layer is a polygon, line, or symbol(text).
        if (abstractLayerStyle->type() == AbstractLayerStyle::LayerType::fill) {
            paintVectorLayer_Fill(
                painter,
                *static_cast<const FillLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                geometryTransform);

        } else if (abstractLayerStyle->type() == AbstractLayerStyle::LayerType::line) {
            paintVectorLayer_Line(
                painter,
                *static_cast<const LineLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                geometryTransform);
        } else if(abstractLayerStyle->type() == AbstractLayerStyle::LayerType::symbol){
            const auto &layerStyle = *static_cast<const SymbolLayerStyle*>(abstractLayerStyle);
            paintVectorLayer_Point(
                painter,
                *static_cast<const SymbolLayerStyle*>(abstractLayerStyle),
                layer,
                vpZoom,
                mapZoom,
                tileWidthPixels,
                geometryTransform,
                labelRects);

        }
    }
}

static void drawBackgroundColor(
    QPainter &painter,
    const StyleSheet &styleSheet,
    int mapZoom)
{
    bool styleFound = false;
    QColor color;

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (const AbstractLayerStyle *abstractLayerStyle : styleSheet.m_layerStyles) {
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
 * \brief The TileScreenPlacement class is a class that describes a tiles
 * position and size within the viewport.
 */
struct TileScreenPlacement {
    int pixelPosX;
    int pixelPosY;
    int pixelWidth;
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
        out.pixelPosX = (int)floor(posNormX * VpMaxDim());
        out.pixelPosY = (int)floor(posNormY * VpMaxDim());

        // We figure out the size of one tile by measuring the distance
        // to the position of the next one.
        double posNormX2 = ((coord.x + 1) * TileSizeNorm()) - worldOriginX;
        out.pixelWidth = (int)floor(posNormX2 * VpMaxDim()) - (int)floor(posNormX * VpMaxDim());
        return out;
    }
};

TilePosCalculator createTilePosCalculator(
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
 *
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
        painter.setClipRect(
            0,
            0,
            tilePlacement.pixelWidth,
            tilePlacement.pixelWidth);

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
    double viewportZoomLevel,
    int mapZoomLevel,
    const QMap<TileCoord, const VectorTile*> &tileContainer,
    const StyleSheet &styleSheet,
    bool drawDebug)
{
    auto paintSingleTileFn = [&](TileCoord tileCoord, TileScreenPlacement tilePlacement) {
        // See if the tile being rendered has any tile-data associated with it.
        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt == tileContainer.end())
            return;

        const VectorTile &tileData = **tileIt;
        paintVectorTile(
            tileData,
            painter,
            mapZoomLevel,
            viewportZoomLevel,
            styleSheet,
            tilePlacement.pixelWidth);
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
        QRect target {
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
