#include "Rendering.h"

int Bach::CalcMapZoomLevelForTileSizePixels(
    int vpWidth,
    int vpHeight,
    double vpZoom,
    int desiredTileWidth)
{
    // Calculate current tile size based on the largest dimension and current scale
    int currentTileSize = qMax(vpWidth, vpHeight);

    // Calculate desired scale factor
    double desiredScale = (double)desiredTileWidth / currentTileSize;

    double newMapZoomLevel = vpZoom - log2(desiredScale);

    // Clamp output to zoom level range.
    return std::clamp((int)round(newMapZoomLevel), 0, maxZoomLevel);
}

/*
QPair<double, double> calcViewportSizeNorm(double vpZoomLevel, double viewportAspect) {
    auto temjp = 1 / pow(2, vpZoomLevel);
}
*/

QVector<TileCoord> Bach::CalcVisibleTiles(
    double vpX,
    double vpY,
    double vpAspect,
    double vpZoomLevel,
    int mapZoomLevel)
{
    mapZoomLevel = qMax(0, mapZoomLevel);

    auto vpWidthNorm = 1 / pow(2, vpZoomLevel);
    auto vpHeightNorm = vpWidthNorm; // Initial assumption: square aspect ratio
    // Adjust for aspect ratio
    if (vpAspect > 1.0) {
        // Viewport is wider than it is tall
        vpHeightNorm /= vpAspect;
    } else {
        // Viewport is taller than it is wide or square
        vpWidthNorm *= vpAspect;
    }

    auto vpMinNormX = vpX - (vpWidthNorm / 2.0);
    auto vpMaxNormX = vpX + (vpWidthNorm / 2.0);
    auto vpMinNormY = vpY - (vpHeightNorm / 2.0);
    auto vpMaxNormY = vpY + (vpHeightNorm / 2.0);

    auto tileCount = 1 << mapZoomLevel;

    auto clampToGrid = [&](int i) {
        return std::clamp(i, 0, tileCount-1);
    };
    auto leftmostTileX = clampToGrid(floor(vpMinNormX * tileCount));
    auto rightmostTileX = clampToGrid(ceil(vpMaxNormX * tileCount));
    auto topmostTileY = clampToGrid(floor(vpMinNormY * tileCount));
    auto bottommostTileY = clampToGrid(ceil(vpMaxNormY * tileCount));

    QVector<TileCoord> visibleTiles;
    for (int y = topmostTileY; y <= bottommostTileY; y++) {
        for (int x = leftmostTileX; x <= rightmostTileX; x++) {
            visibleTiles += { mapZoomLevel, x, y };
        }
    }
    return visibleTiles;
}

void Bach::paintSingleTileDebug(
    QPainter& painter,
    TileCoord const& tileCoord,
    QPoint pixelPos,
    QTransform const& transform)
{
    painter.setPen(Qt::green);
    painter.drawLine(transform.map(QLineF{ QPointF(0.45, 0.45), QPointF(0.55, 0.55) }));
    painter.drawLine(transform.map(QLineF{ QPointF(0.55, 0.45), QPointF(0.45, 0.55) }));
    painter.drawRect(transform.mapRect(QRectF(0, 0, 1, 1)));

    {
        // Text rendering has issues if our coordinate system is [0, 1].
        // So we get it back to unscaled and just offset where we need it.
        painter.save();
        QTransform transform;
        transform.translate(pixelPos.x(), pixelPos.y());
        painter.setTransform(transform);
        painter.drawText(10, 30, tileCoord.toString());

        painter.restore();
    }
}
