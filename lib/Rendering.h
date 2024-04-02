#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <QPainter>
#include <QMap>
#include <QPair>

#include "TileCoord.h"
#include "VectorTiles.h"
#include "Layerstyle.h"

namespace Bach {
    /*!
     * \brief maxZoomLevel is the maximum allowed zoom level of the map.
     */
    constexpr int maxZoomLevel = 15;

    /* Controls the default value for how big tiles should appear on-screen.
     * Smaller values mean each individual tile will appear smaller on screen and provide more detail,
     * larger means each it will appear larger.
     *
     * Expressed in pixels.
     */
    const int defaultDesiredTileSizePixels = 512;

    QPair<double, double> lonLatToWorldNormCoord(double lon, double lat);
    QPair<double, double> lonLatToWorldNormCoordDegrees(double lon, double lat);
    QPair<double, double> calcViewportSizeNorm(double viewportZoom, double viewportAspect);
    double normalizeValueToZeroOneRange(double value, double min, double max);
    void paintSingleTileFeature_Fill_Polygon(
        QPainter &painter,
        const PolygonFeature &feature,
        const FillLayerStyle &layerStyle,
        const int mapZoom,
        const double vpZoom,
        const QTransform &transformIn);

    int calcMapZoomLevelForTileSizePixels(
        int vpWidth,
        int vpHeight,
        double vpZoom,
        int desiredTileSize = defaultDesiredTileSizePixels);

    QVector<TileCoord> calcVisibleTiles(
        double vpX,
        double vpY,
        double vpAspect,
        double vpZoomLevel,
        int mapZoomLevel);

    void paintVectorTiles(
        QPainter &painter,
        double vpX,
        double vpY,
        double viewportZoomLevel,
        int mapZoomLevel,
        const QMap<TileCoord, const VectorTile*> &tileContainer,
        const StyleSheet &styleSheet,
        bool drawDebug);


    void paintPngTiles(
        QPainter &painter,
        double vpX,
        double vpY,
        double viewportZoomLevel,
        int mapZoomLevel,
        const QMap<TileCoord, const QImage*> &tileContainer,
        const StyleSheet &styleSheet,
        bool drawDebug);
}

#endif // RENDERING_HPP
