#include "Evaluator.h"
#include "Rendering.h"

/*  Information about what's on this file:
    It consists of two major sections:

    1. Rendering helper functions only used in `Rendering.cpp`.
    2. Rendering functions that can be used externally, declared in `Rendering.h`.

    Rendering helper functions follow below.
*/

/* This is a helper function for visualizing the boundaries of each tile.
 *
 * It is responsible for drawing green borders around each tile.
 *
 * This function assumes the painter's transform has moved the origin of the
 * canvas to the tile's origin, and has not been scaled.
 *
 * The pen width needs to be set appropriately before using this function.
 *
 * The 'scale' parameter is a scalar that geometry should be multiplied
 * with such that coordinates in the range [0, 1] become correctly mapped
 * inside tile.
 */
static void paintSingleTileDebug(
    QPainter &painter,
    const TileCoord &tileCoord,
    double scale)
{
    painter.setPen(Qt::green);

    QTransform transform;
    transform.scale(scale, scale);

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

    // Draw the text that shows the coordinates of the tile itself.
    {
        // Text rendering has issues if our coordinate system is [0, 1].
        // So we reset the coordinate system back to unscaled and
        // just offset the text to where we need it.
        painter.drawText(10, 30, tileCoord.toString());
    }
}

// Determines whether this layer is hidden.
static bool isLayerHidden(const AbstractLayereStyle &layerStyle, int mapZoom)
{
    return
        layerStyle.m_visibility == "none" ||
        layerStyle.m_maxZoom < mapZoom ||
        layerStyle.m_minZoom >= mapZoom;
}

// Determines whether a feature should be included when rendering this layer-style.
// Returns true if the feature should be included.
static bool includeFeature(
    const AbstractLayereStyle &layerStyle,
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

/* This function takes care of rendering a single tile.
 *
 * This is called repeatedly from the 'paintTiles' function.
 *
 * It assumes the painter object has had its origin moved to the tiles origin.
 *
 * This does not handle background color.
 */
static void paintSingleTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoom,
    double vpZoom,
    const StyleSheet &styleSheet,
    const QTransform &transformIn,
    int tileSize)
{
    QVector<QPair<int, PointFeature>> labels; //Used to order text rendering operation based on "rank" property.
    QVector<QRect> laberRects; //Used to prevent text overlapping.
    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (auto const& abstractLayerStyle : styleSheet.m_layerStyles) {
        if (isLayerHidden(*abstractLayerStyle, mapZoom))
            continue;

        // Check if this layer style has an associated layer in the tile.
        auto layerIt = tileData.m_layers.find(abstractLayerStyle->m_sourceLayer);
        if (layerIt == tileData.m_layers.end()) {
            continue;
        }
        // If we find it, we dereference it to access it's data.
        auto const& layer = **layerIt;

        // We do different types of rendering based on whether the layer is a polygon, line, or symbol(text).
        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::fill) {
            auto const& layerStyle = *static_cast<FillLayerStyle const*>(abstractLayerStyle);

            // Iterate over all the features, and filter out anything that is not fill.
            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() != AbstractLayerFeature::featureType::polygon)
                    continue;
                const auto& feature = *static_cast<const PolygonFeature*>(abstractFeature);

                if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
                    continue;

                // Render the feature in question.
                painter.save();
                Bach::paintSingleTileFeature_Fill_Polygon(
                    painter,
                    feature,
                    layerStyle,
                    mapZoom,
                    vpZoom,
                    transformIn);
                painter.restore();
            }
        } else if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::line) {
            auto const& layerStyle = *static_cast<LineLayerStyle const*>(abstractLayerStyle);

            // Iterate over all the features, and filter out anything that is not line.
            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() != AbstractLayerFeature::featureType::line)
                    continue;
                const auto &feature = *static_cast<const LineFeature*>(abstractFeature);

                // Tests whether the feature should be rendered at all based on possible expression.
                if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
                    continue;

                // Render the feature in question.
                painter.save();
                Bach::paintSingleTileFeature_Line(
                    painter,
                    feature,
                    layerStyle,
                    mapZoom,
                    vpZoom,
                    transformIn);
                painter.restore();
            }
        } else if(abstractLayerStyle->type() == AbstractLayereStyle::LayerType::symbol){
             auto const& layerStyle = *static_cast<const SymbolLayerStyle *>(abstractLayerStyle);

             // Iterate over all the features, and filter out anything that is not point  (rendering of line features for curved text in the symbol layer is not yet implemented).
             for (auto const& abstractFeature : layer.m_features) {
                 if (abstractFeature->type() != AbstractLayerFeature::featureType::point) //For normal text (continents /countries / cities / places / ...)
                     continue;
                 const auto &feature = *static_cast<const PointFeature*>(abstractFeature);

                 // Tests whether the feature should be rendered at all based on possible expression.
                 if (!includeFeature(layerStyle, feature, mapZoom, vpZoom))
                     continue;
                 //Add the feature along with its "rank" (if present, defaults to 100) to the labels map.
                 if(feature.fetureMetaData.contains("rank")){
                     labels.append(QPair<int, PointFeature>(feature.fetureMetaData["rank"].toInt(), feature));
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
                     painter,
                     pair.second,
                     layerStyle,
                     mapZoom,
                     vpZoom,
                     transformIn,
                     tileSize,
                     laberRects);
                 painter.restore();
             }
        }
    }
}

// Assumes the painter is unchanged.
static void drawBackgroundColor(
    QPainter &painter,
    const StyleSheet &styleSheet,
    int mapZoom)
{
    bool styleFound = false;
    QColor color;

    // We start by iterating over each layer style, it determines the order
    // at which we draw the elements of the map.
    for (auto const& abstractLayerStyle : styleSheet.m_layerStyles) {
        // Background is a special case and has no associated layer.
        // We just draw it and move onto the next layer style.
        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::background) {
            // Fill the entire tile with a single color
            const auto& layerStyle = *static_cast<const BackgroundStyle*>(abstractLayerStyle);

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

// Exported rendering functionality.

/*!
 * \brief Bach::paintVectorTiles renders vector tiles to the map.
 *  The function will iterate over multiple tiles and place them correctly on screen.
 *
 * \param painter is a QPainter object that renders the tile data to the screen.
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
    // Start by drawing the background color on the entire canvas.
    drawBackgroundColor(painter, styleSheet, mapZoomLevel);

    // Gather width and height of the viewport, in pixels..
    auto vpWidth = painter.window().width();
    auto vpHeight = painter.window().height();

    // Aspect ratio of the viewport.
    auto vpAspect = (double)vpWidth / (double)vpHeight;

    // The longest length between vpWidth and vpHeight
    auto vpMaxDim = qMax(vpWidth, vpHeight);

    // Helper function to turn coordinates that are expressed as fraction of the viewport,
    // into pixel coordinates.
    auto toPixelSpace = [&](double in) { return (int)round(in * vpMaxDim); };

    // Calculate the set of visible tiles that fit in the viewport.
    auto visibleTiles = calcVisibleTiles(
        vpX,
        vpY,
        vpAspect,
        viewportZoomLevel,
        mapZoomLevel);

    // The scale of the world map as a fraction of the viewport.
    auto vpScale = pow(2, viewportZoomLevel);
    // The scale of a single tile as a fraction of the viewport.
    auto tileSizeNorm = vpScale / (1 << mapZoomLevel);

    // Calculate where the top-left origin of the world map is relative to the viewport.
    double worldOriginX = vpX * vpScale - 0.5;
    double worldOriginY = vpY * vpScale - 0.5;
    // The world such that our worldmap is still centered around our center-coordinate
    // when the aspect ratio changes.
    if (vpAspect < 1.0) {
        worldOriginX += -0.5 * vpAspect + 0.5;
    } else if (vpAspect > 1.0) {
        worldOriginY += -0.5 / vpAspect + 0.5;
    }

    // Iterate over all possible tiles that can possibly fit in this viewport.
    for (const auto& tileCoord : visibleTiles) {
        // Position the top-left of the tile inside the world map, as a fraction of the viewport size.
        // We use the tile index coords and the size of a tile to generate the offset.
        auto posNormX = (tileCoord.x * tileSizeNorm) - worldOriginX;
        auto posNormY = (tileCoord.y * tileSizeNorm) - worldOriginY;

        // Convert tile position and size to pixels.
        auto tilePixelPosX = toPixelSpace(posNormX);
        auto tilePixelPosY = toPixelSpace(posNormY);
        auto tileSizePixels = toPixelSpace(tileSizeNorm);

        painter.save();

        // We move the origin point of the painter to the top-left of the tile.
        // We do not apply scaling because it interferes with other sized elements,
        // like the size of pen width.
        QTransform transform;
        transform.translate(tilePixelPosX, tilePixelPosY);
        painter.setTransform(transform);

        // Temporary. Should likely just be passed as a single scalar to tile-drawing function.
        QTransform geometryTransform;
        geometryTransform.scale(vpMaxDim * tileSizeNorm, vpMaxDim * tileSizeNorm);

        // See if the tile being rendered has any tile-data associated with it.
        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt != tileContainer.end()) {
            auto const& tileData = **tileIt;
            painter.save();

            // We create a clip rect around our tile, as to only render into
            // the region on-screen the tile occupies.
            painter.setClipRect(
                0,
                0,
                tileSizePixels,
                tileSizePixels);

            // Run the rendering function for a single tile.
            paintSingleTile(
                tileData,
                painter,
                mapZoomLevel,
                viewportZoomLevel,
                styleSheet,
                geometryTransform,
                tileSizePixels);

            painter.restore();
        }

        // Paint debug lines around the tile.
        if (drawDebug) {
            paintSingleTileDebug(
                painter,
                tileCoord,
                vpMaxDim * tileSizeNorm);
        }
        painter.restore();
    }
}

void Bach::paintPngTiles(
    QPainter &painter,
    double vpX,
    double vpY,
    double viewportZoomLevel,
    int mapZoomLevel,
    const QMap<TileCoord, const QImage*> &tileContainer,
    const StyleSheet &styleSheet,
    bool drawDebug)
{
    // Start by drawing the background color on the entire canvas.
    drawBackgroundColor(painter, styleSheet, mapZoomLevel);

    // Gather width and height of the viewport, in pixels..
    auto vpWidth = painter.window().width();
    auto vpHeight = painter.window().height();

    // Aspect ratio of the viewport.
    auto vpAspect = (double)vpWidth / (double)vpHeight;

    // The longest length between vpWidth and vpHeight
    auto vpMaxDim = qMax(vpWidth, vpHeight);

    // Helper function to turn coordinates that are expressed as fraction of the viewport,
    // into pixel coordinates.
    auto toPixelSpace = [&](double in) { return (int)round(in * vpMaxDim); };

    // Calculate the set of visible tiles that fit in the viewport.
    auto visibleTiles = calcVisibleTiles(
        vpX,
        vpY,
        vpAspect,
        viewportZoomLevel,
        mapZoomLevel);

    // The scale of the world map as a fraction of the viewport.
    auto vpScale = pow(2, viewportZoomLevel);
    // The scale of a single tile as a fraction of the viewport.
    auto tileSizeNorm = vpScale / (1 << mapZoomLevel);

    // Calculate where the top-left origin of the world map is relative to the viewport.
    double worldOriginX = vpX * vpScale - 0.5;
    double worldOriginY = vpY * vpScale - 0.5;
    // The world such that our worldmap is still centered around our center-coordinate
    // when the aspect ratio changes.
    if (vpAspect < 1.0) {
        worldOriginX += -0.5 * vpAspect + 0.5;
    } else if (vpAspect > 1.0) {
        worldOriginY += -0.5 / vpAspect + 0.5;
    }

    // Iterate over all possible tiles that can possibly fit in this viewport.
    for (const auto& tileCoord : visibleTiles) {
        // Position the top-left of the tile inside the world map, as a fraction of the viewport size.
        // We use the tile index coords and the size of a tile to generate the offset.
        auto posNormX = (tileCoord.x * tileSizeNorm) - worldOriginX;
        auto posNormY = (tileCoord.y * tileSizeNorm) - worldOriginY;

        // Convert tile position and size to pixels.
        auto tilePixelPosX = toPixelSpace(posNormX);
        auto tilePixelPosY = toPixelSpace(posNormY);
        auto tileSizePixels = toPixelSpace(tileSizeNorm);

        painter.save();

        // We move the origin point of the painter to the top-left of the tile.
        // We do not apply scaling because it interferes with other sized elements,
        // like the size of pen width.
        QTransform transform;
        transform.translate(tilePixelPosX, tilePixelPosY);
        painter.setTransform(transform);

        // See if the tile being rendered has any tile-data associated with it.
       auto tileIt = tileContainer.find(tileCoord);
       if (tileIt != tileContainer.end()) {
            const QImage &tileData = **tileIt;

            painter.save();

            QRectF target(0.0, 0.0, tileSizePixels, tileSizePixels);
            QRectF source(0.0, 0.0, 512.0, 512.0);

            painter.drawImage(target, tileData, source);

            painter.restore();
        }

        // Paint debug lines around the tile.
        if (drawDebug) {
            paintSingleTileDebug(
                painter,
                tileCoord,
                vpMaxDim * tileSizeNorm);
        }

        painter.restore();
    }
}
