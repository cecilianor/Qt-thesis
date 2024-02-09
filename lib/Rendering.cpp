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
