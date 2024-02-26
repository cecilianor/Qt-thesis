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
    auto leftmostTileX = clampToGrid(floor(vpMinNormX * tileCount));
    auto rightmostTileX = clampToGrid(ceil(vpMaxNormX * tileCount));
    auto topmostTileY = clampToGrid(floor(vpMinNormY * tileCount));
    auto bottommostTileY = clampToGrid(ceil(vpMaxNormY * tileCount));

    // Iterate over our two ranges to build our list.
    QVector<TileCoord> visibleTiles;
    for (int y = topmostTileY; y <= bottommostTileY; y++) {
        for (int x = leftmostTileX; x <= rightmostTileX; x++) {
            visibleTiles += { mapZoomLevel, x, y };
        }
    }
    return visibleTiles;
}

/* This is a helper function for visualizing the boundaries of each tile.
 *
 * It is responsible for drawing green borders around each tile.
 *
 * This function assumes the painter's state has been set up such that
 * it's not modified from the default pixel-based coordinate space.
 *
 * The pen width needs to be set appropriately before using this function.
 */
static void paintSingleTileDebug(
    QPainter &painter,
    const TileCoord &tileCoord,
    QPoint pixelPos,
    const QTransform &transform)
{
    painter.setPen(Qt::green);

    // Draw the X in the middle of the tile first, by using tile-normalized coordinates
    // from 0.45 to 0.55.
    painter.drawLine(transform.map(QLineF{ QPointF(0.45, 0.45), QPointF(0.55, 0.55) }));
    painter.drawLine(transform.map(QLineF{ QPointF(0.55, 0.45), QPointF(0.45, 0.55) }));

    // Then draw the boundary of the tile.
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
        painter.save();

        QTransform transform;
        transform.translate(pixelPos.x(), pixelPos.y());
        painter.setTransform(transform);
        painter.drawText(10, 30, tileCoord.toString());

        painter.restore();
    }
}

/* This function takes care of rendering a single tile.
 * This is called repeatedly from the 'paintTiles' function.
 */
static void paintSingleTile(
    const VectorTile &tileData,
    QPainter &painter,
    int mapZoomLevel,
    float viewportZoomLevel,
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
    auto viewportWidth = painter.window().width();
    auto viewportHeight = painter.window().height();
    double vpAspectRatio = (double)viewportWidth / (double)viewportHeight;
    auto visibleTiles = calcVisibleTiles(
        vpX,
        vpY,
        vpAspectRatio,
        viewportZoomLevel,
        mapZoomLevel);

    auto largestDimension = qMax(viewportWidth, viewportHeight);

    double scale = pow(2, viewportZoomLevel - mapZoomLevel);
    double tileWidthNorm = scale;
    double tileHeightNorm = scale;

    auto font = painter.font();
    font.setPointSize(18);
    painter.setFont(font);

    // Calculate total number of tiles at the current zoom level
    int totalTilesAtZoom = 1 << mapZoomLevel;

    // Calculate the offset of the viewport center in pixel coordinates
    double centerNormX = vpX * totalTilesAtZoom * tileWidthNorm - 1.0 / 2;
    double centerNormY = vpY * totalTilesAtZoom * tileHeightNorm - 1.0 / 2;

    if (viewportHeight >= viewportWidth) {
        centerNormX += -0.5 * vpAspectRatio + 0.5;
    } else if (viewportWidth >= viewportHeight) {
        centerNormY += -0.5 * (1 / vpAspectRatio) + 0.5;
    }

    for (const auto& tileCoord : visibleTiles) {
        double posNormX = (tileCoord.x * tileWidthNorm) - centerNormX;
        double posNormY = (tileCoord.y * tileHeightNorm) - centerNormY;

        auto tilePixelPos = QPoint(
            round(posNormX * largestDimension),
            round(posNormY * largestDimension));
        int tileWidthPixels = round(tileWidthNorm * largestDimension);
        int tileHeightPixels = round(tileHeightNorm * largestDimension);

        painter.save();

        QTransform transform;
        transform.translate(tilePixelPos.x(), tilePixelPos.y());
        //transform.scale(largestDimension, largestDimension);
        painter.setTransform(transform);

        auto pen = painter.pen();
        pen.setColor(Qt::white);
        pen.setWidth(1);
        painter.setPen(pen);

        QTransform test;
        test.scale(largestDimension * scale, largestDimension * scale);

        auto tileIt = tileContainer.find(tileCoord);
        if (tileIt != tileContainer.end()) {
            auto const& tileData = **tileIt;

            painter.save();
            painter.setClipRect(
                0,
                0,
                tileWidthPixels,
                tileHeightPixels);

            paintSingleTile(
                tileData,
                painter,
                mapZoomLevel,
                viewportZoomLevel,
                styleSheet,
                test);

            painter.restore();
        }

        paintSingleTileDebug(painter, tileCoord, tilePixelPos, test);

        painter.restore();
    }
}
