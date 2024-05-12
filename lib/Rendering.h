#ifndef RENDERING_HPP
#define RENDERING_HPP

// Qt header files
#include <QMap>
#include <QPainter>
#include <QPair>

// Other header files
#include "LayerStyle.h"
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

    /*!
     * \internal
     * \brief The PaintingDetailsPolygon class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct PaintingDetailsPolygon {
        QPainter *painter = nullptr;
        const FillLayerStyle *layerStyle = nullptr;
        const PolygonFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    /*!
     * \internal
     * \brief The PaintingDetailsLine class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct PaintingDetailsLine {
        QPainter *painter = nullptr;
        const LineLayerStyle *layerStyle = nullptr;
        const LineFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    /*!
     * \internal
     * \brief The PaintingDetailsPoint class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct PaintingDetailsPoint {
        QPainter *painter = nullptr;
        const SymbolLayerStyle *layerStyle = nullptr;
        const PointFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    /*!
     * \internal
     * \brief The PaintingDetailsPointCurved class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct PaintingDetailsPointCurved {
        QPainter *painter = nullptr;
        const SymbolLayerStyle *layerStyle = nullptr;
        const LineFeature *feature = nullptr;
        int mapZoom{};
        double vpZoom{};
        QTransform transformIn;
    };

    /*!
     * \internal
     * \brief The vpGlobalText class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct vpGlobalText{
        QPoint tileOrigin;
        QList<QPainterPath> path;
        QList<QString> text;
        QList<QPoint> position;
        QFont font;
        QColor textColor;
        int outlineSize;
        QColor outlineColor;
        QRect boundingRect;
    };


    /*!
     * \internal
     * \brief The singleCurvedTextCharacter class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct singleCurvedTextCharacter {
        QChar character;
        QPointF position;
        qreal angle;
    };

    /*!
     * \internal
     * \brief The vpGlobalCurvedText class
     * Helper class for passing values around internally in painter functions.
     *
     * Only for internal use.
     */
    struct vpGlobalCurvedText {
        QVector<singleCurvedTextCharacter> textList;
        QFont font;
        QColor textColor;
        float opacity;
        QPoint tileOrigin;
        QColor outlineColor;
        int outlineSize;
    };

    /*!
     * \brief The MapCoordinate struct stores a map coordinate with a x and y.
     *
     * `x` and y are both doubles.
     */
    struct MapCoordinate {
        double x;
        double y;
    };

    MapCoordinate lonLatToWorldNormCoord(double lon, double lat);
    MapCoordinate lonLatToWorldNormCoordDegrees(double lon, double lat);
    MapCoordinate calcViewportSizeNorm(double viewportZoom, double viewportAspect);
    double normalizeValueToZeroOneRange(double value, double min, double max);

    void paintSingleTileFeature_Polygon(PaintingDetailsPolygon details);

    void paintSingleTileFeature_Line(PaintingDetailsLine details);


    void processSingleTileFeature_Point(
        PaintingDetailsPoint details,
        const int tileSize,
        const int tileOriginX,
        const int tileOriginY,
        const bool forceNoChangeFontType,
        QVector<QRect> &rects,
        QVector<vpGlobalText> &vpTextList);

    void paintSingleTileFeature_Point_Curved(PaintingDetailsPointCurved details);

    void processSingleTileFeature_Point_Curved(
        PaintingDetailsPointCurved details,
        const int tileSize,
        int tileOriginX,
        int tileOriginY,
        QVector<QRect> &rects,
        QVector<vpGlobalCurvedText> &vpCurvedTextList);


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

        /*!
         * \brief
         * used to switch the rendering function between the QPainter drawPath function
         * and the QTextLayout draw function for text rendering.
         */
        bool useQTextLayout = {};

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
