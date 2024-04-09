#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <QMap>
#include <QPainter>
#include <QPair>

#include "Layerstyle.h"
#include "TileCoord.h"
#include "VectorTiles.h"

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

    struct PaintingDetailsPolygon{
        QPainter *painter;
        const FillLayerStyle *layerStyle = nullptr;
        const PolygonFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    struct PaintingDetailsLine{
        QPainter *painter;
        const LineLayerStyle *layerStyle = nullptr;
        const LineFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    struct PaintingDetailsPoint{
        QPainter *painter;
        const SymbolLayerStyle *layerStyle = nullptr;
        const PointFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    struct PaintingDetailsPointCurved{
        QPainter *painter;
        const SymbolLayerStyle *layerStyle = nullptr;
        const LineFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };


    QPair<double, double> lonLatToWorldNormCoord(double lon, double lat);
    QPair<double, double> lonLatToWorldNormCoordDegrees(double lon, double lat);
    QPair<double, double> calcViewportSizeNorm(double viewportZoom, double viewportAspect);
    double normalizeValueToZeroOneRange(double value, double min, double max);

    void paintSingleTileFeature_Polygon(PaintingDetailsPolygon details);

    void paintSingleTileFeature_Line(PaintingDetailsLine details);


    void paintSingleTileFeature_Point(
        PaintingDetailsPoint details,
        const int tileSize,
        bool forceNoChangeFontType,
        QVector<QRect> &rects);

    void paintSingleTileFeature_Point_Curved(PaintingDetailsPointCurved details);


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


    /*!
     * \class Collection of settings that modify how vector tiles are rendered.
     */
    struct PaintVectorTileSettings {
        /*!
         *  \brief
         *  Controls whether fill elements should be rendered.
         */
        bool drawFill = {};

        /*!
         *  \brief
         *  Controls whether line elements should be rendered.
         */
        bool drawLines = {};

        /*!
         *  \brief
         *  Controls whether text elements should be rendered.
         */
        bool drawText = {};

        /*!
         *  \brief
         *  Forces text rendering to never change the font beyond
         *  the one that is already set by the QPainter beforehand.
         */
        bool forceNoChangeFontType = {};

        static PaintVectorTileSettings getDefault();
    };

    void paintVectorTiles(
        QPainter &painter,
        double vpX,
        double vpY,
        double viewportZoom,
        int mapZoom,
        const QMap<TileCoord, const VectorTile*> &tileContainer,
        const StyleSheet &styleSheet,
        const PaintVectorTileSettings &settings,
        bool drawDebug);

    void paintRasterTiles(
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
