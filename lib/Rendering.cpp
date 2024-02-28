#include "Rendering.h"

QPair<double, double> Bach::lonLatToWorldNormCoord(double lon, double lat)
{
    constexpr double webMercatorPhiCutoff = 1.4844222297;

    // Function to normalize a value from its original range to [0, 1]
    auto normalize = [](double value, double min, double max) {
        return (value - min) / (max - min);
    };

    // Convert longitude and latitude to radians
    auto lambda = lon;
    auto phi = lat;

    // Convert to Web Mercator
    auto x = lambda;
    auto y = std::log(std::tan(M_PI / 4.0 + phi / 2.0));

    // Normalize x and y to [0, 1]
    // Assuming the Web Mercator x range is [-π, π] and y range is calculated from latitude range
    auto xNormalized = normalize(x, -M_PI, M_PI);
    auto yNormalized = normalize(
        y,
        std::log(std::tan(M_PI / 4.0 + -webMercatorPhiCutoff / 2.0)),
        std::log(std::tan(M_PI / 4.0 + webMercatorPhiCutoff / 2.0)));

    return { xNormalized, yNormalized };
}

QPair<double, double> Bach::lonLatToWorldNormCoordDegrees(double lon, double lat)
{
    auto degToRad = [](double deg) {
        return deg * M_PI / 180.0;
    };
    return lonLatToWorldNormCoord(degToRad(lon), degToRad(lat));
}

int Bach::calcMapZoomLevelForTileSizePixels(
    int vpWidth,
    int vpHeight,
    double vpZoom,
    int desiredTileWidth)
{
    // Calculate current tile size based on the largest dimension and current scale
    int currentTileSize = qMax(vpWidth, vpHeight);

    // Calculate desired scale factor
    double desiredScale = (double)desiredTileWidth / currentTileSize;

    // Figure out how the difference between the zoom levels of viewport and map
    // needed to satisfy the pixel-size requirement.
    double newMapZoomLevel = vpZoom - log2(desiredScale);

    // Round to int, and clamp output to zoom level range.
    return std::clamp((int)round(newMapZoomLevel), 0, maxZoomLevel);
}

QPair<double, double> Bach::calcViewportSizeNorm(double vpZoomLevel, double viewportAspect) {
    // Math formula can be seen in the figure in the report, with the caption
    // "Calculating viewport size as a factor of the world map"
    auto temp = 1 / pow(2, vpZoomLevel);
    return {
        temp * qMin(1.0, 1.0 / viewportAspect),
        temp * qMax(1.0, viewportAspect)
    };
}

QVector<TileCoord> Bach::calcVisibleTiles(
    double vpX,
    double vpY,
    double vpAspect,
    double vpZoomLevel,
    int mapZoomLevel)
{
    mapZoomLevel = qMax(0, mapZoomLevel);

    // We need to calculate the width and height of the viewport in terms of
    // world-normalized coordinates.
    auto [vpWidthNorm, vpHeightNorm] = calcViewportSizeNorm(vpZoomLevel, vpAspect);

    // Figure out the 4 edges in world-normalized coordinate space.
    auto vpMinNormX = vpX - (vpWidthNorm / 2.0);
    auto vpMaxNormX = vpX + (vpWidthNorm / 2.0);
    auto vpMinNormY = vpY - (vpHeightNorm / 2.0);
    auto vpMaxNormY = vpY + (vpHeightNorm / 2.0);

    // Amount of tiles in each direction for this map zoom level.
    auto tileCount = 1 << mapZoomLevel;

    auto clampToGrid = [&](int i) {
        return std::clamp(i, 0, tileCount-1);
    };

    // Convert edges into the index-based grid coordinates, and apply a clamp operation
    // in case the viewport goes outside the map.
    auto leftTileX = clampToGrid((int)floor(vpMinNormX * tileCount));
    auto rightTileX = clampToGrid((int)floor(vpMaxNormX * tileCount));
    auto topTileY = clampToGrid((int)floor(vpMinNormY * tileCount));
    auto botTileY = clampToGrid((int)floor(vpMaxNormY * tileCount));

    // Iterate over our two ranges to build our list.

    if (mapZoomLevel == 0 &&
        rightTileX - leftTileX == 0 &&
        botTileY - topTileY == 0)
    {
        return { { 0, 0, 0 } };
    } else {
        QVector<TileCoord> visibleTiles;
        for (int y = topTileY; y <= botTileY; y++) {
            for (int x = leftTileX; x <= rightTileX; x++) {
                visibleTiles += { mapZoomLevel, x, y };
            }
        }
        return visibleTiles;
    }
}

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
    QPoint pixelPos,
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

/* This function takes care of rendering a single tile.
 * This is called repeatedly from the 'paintTiles' function.
 */
static void paintSingleTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoomLevel,
    double viewportZoomLevel,
    const StyleSheet &styleSheet,
    const QTransform &transformIn)
{
    for (auto const& abstractLayerStyle : styleSheet.m_layerStyles) {

        // Background is a special case and has no associated layer.
        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::background) {
            // Fill the entire tile with a single color
            auto const& layerStyle = *static_cast<BackgroundStyle const*>(abstractLayerStyle);
            auto backgroundColor = layerStyle.getColorAtZoom(mapZoomLevel);
            painter.fillRect(transformIn.mapRect(QRect(0, 0, 1, 1)), backgroundColor);
            continue;
        }

        auto layerExists = tileData.m_layers.contains(abstractLayerStyle->m_sourceLayer);
        if (!layerExists) {
            continue;
        }
        auto const& layer = *tileData.m_layers[abstractLayerStyle->m_sourceLayer];

        if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::fill) {
            auto const& layerStyle = *static_cast<FillLayerStyle const*>(abstractLayerStyle);

            painter.setBrush(layerStyle.getFillColorAtZoom(mapZoomLevel));

            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() == AbstractLayerFeature::featureType::polygon) {
                    auto const& feature = *static_cast<PolygonFeature const*>(abstractFeature);
                    auto const& path = feature.polygon();

                    QTransform transform = transformIn;
                    transform.scale(1 / 4096.0, 1 / 4096.0);
                    auto newPath = transform.map(path);

                    painter.save();
                    painter.setPen(Qt::NoPen);
                    painter.drawPath(newPath);
                    painter.restore();
                }
            }
        }
        else if (abstractLayerStyle->type() == AbstractLayereStyle::LayerType::line) {
            auto const& layerStyle = *static_cast<LineLayerStyle const*>(abstractLayerStyle);

            painter.save();

            auto pen = painter.pen();
            auto lineColor = layerStyle.getLineColorAtZoom(mapZoomLevel);
            //lineColor.setAlphaF(layerStyle.getLineOpacityAtZoom(mapZoomLevel));
            pen.setColor(lineColor);
            pen.setWidth(layerStyle.getLineWidthAtZoom(mapZoomLevel));
            //pen.setMiterLimit(layerStyle.getLineMitterLimitAtZoom(mapZoomLevel));
            painter.setPen(pen);

            painter.setBrush(Qt::NoBrush);

            for (auto const& abstractFeature : layer.m_features) {
                if (abstractFeature->type() == AbstractLayerFeature::featureType::line) {
                    auto& feature = *static_cast<LineFeature const*>(abstractFeature);
                    auto const& path = feature.line();
                    QTransform transform = transformIn;
                    transform.scale(1 / 4096.0, 1 / 4096.0);
                    auto newPath = transform.map(path);

                    painter.save();
                    painter.drawPath(newPath);
                    painter.restore();
                }
            }

            painter.restore();
        }
    }
}

void Bach::paintTiles(
    QPainter &painter,
    double vpX,
    double vpY,
    double viewportZoomLevel,
    int mapZoomLevel,
    const QMap<TileCoord, const VectorTile*> &tileContainer,
    const StyleSheet &styleSheet)
{
    // Gather width and height of the viewport, in pixels..
    auto vpWidth = painter.window().width();
    auto vpHeight = painter.window().height();

    // Aspect ratio of the viewport.
    auto vpAspect = (double)vpWidth / (double)vpHeight;

    // The longest length between vpWidth and vpHeight
    auto vpMaxDim = qMax(vpWidth, vpHeight);

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

        // Helper functions to turn coordinates that are expressed as fraction of the viewport,
        // into pixel coordinates.
        auto toPixelSpace = [&](double in) { return (int)round(in * vpMaxDim); };

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
                geometryTransform);

            painter.restore();
        }

        // Paint debug lines around the tile.
        paintSingleTileDebug(
            painter,
            tileCoord,
            { tilePixelPosX, tilePixelPosY },
            vpMaxDim * tileSizeNorm);

        painter.restore();
    }
}
