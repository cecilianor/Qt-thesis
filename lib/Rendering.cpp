#include "Rendering.h"

QVector<TileCoord> Bach::CalcVisibleTiles(
    double vpX,
    double vpY,
    double vpZoomLevel,
    int mapZoomLevel)
{
    mapZoomLevel = qMax(0, mapZoomLevel);
    QVector<TileCoord> visibleTiles;
    auto vpWidthNorm = 1 / pow(2, vpZoomLevel);
    auto vpMinNormX = vpX - (vpWidthNorm / 2.0);
    auto vpMaxNormX = vpX + (vpWidthNorm / 2.0);
    auto vpMinNormY = vpY - (vpWidthNorm / 2.0);
    auto vpMaxNormY = vpY + (vpWidthNorm / 2.0);

    auto tileCount = 1 << mapZoomLevel;

    auto leftmostTileX = (int)qMax(0.0, floor(vpMinNormX * tileCount));
    auto rightmostTileX = (int)qMin((double)tileCount-1, ceil(vpMaxNormX * tileCount));
    auto topmostTileY = (int)qMax(0.0, floor(vpMinNormY * tileCount));
    auto bottommostTileY = (int)qMin((double)tileCount-1, ceil(vpMaxNormY * tileCount));
    for (int y = topmostTileY; y <= bottommostTileY; y++) {
        for (int x = leftmostTileX; x <= rightmostTileX; x++) {
            visibleTiles += { mapZoomLevel, x, y };
        }
    }
    return visibleTiles;
}

void paintSingleTileDebug(
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
