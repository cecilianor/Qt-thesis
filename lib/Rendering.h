#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <QPainter>
#include <QMap>
#include <QPair>

#include "TileCoord.h"
#include "VectorTiles.h"
#include "Layerstyle.h"

namespace Bach {
    constexpr int maxZoomLevel = 15;

    /* Controls the default value for how big tiles should appear on-screen.
     * Smaller values mean each individual tile will appear smaller on screen and provide more detail,
     * larger means each it will appear larger.
     *
     * Expressed in pixels.
     */
    const int defaultDesiredTileSizePixels = 512;

    /* Converts longitude and latitude to world-normalized coordinates.
     * Takes radians.
     *
     * The math formula employed is described by the figure in the report with the caption
     * "Converting longitude and latitude to world-normalized coordinates"
     */
    QPair<double, double> lonLatToWorldNormCoord(double lon, double lat);
    /* Converts longitude and latitude to world-normalized coordinates.
     * Takes degrees.
     */
    QPair<double, double> lonLatToWorldNormCoordDegrees(double lon, double lat);

    /* Calculates the width and height of the viewport in world-normalized coordinates.
     * This means the size expressed as a fraction of the world map. For example,
     * a viewportZoom set to 0 will return size as 1, while a zoom level of
     * 1 will return 0.5. This takes the aspect ratio of the viewport into account,
     * with the largest side being mapped to relation mentioned.
     *
     * The math formula used is described by the figure in the report with the caption
     * "Calculating viewport size as a factor of the world map".
     *
     * Returns width and height as fractions, in the range [0, 1]
     */
    QPair<double, double> calcViewportSizeNorm(double viewportZoom, double viewportAspect);

    /* Calculates the closest map zoom level as such that a tile's on-screen size
     * is closest to 'desiredTileSize'.
     *
     * Parameters:
     *      vpWidth expects the width of the viewport in pixels.
     *      vpHeight expects the height of the viewport in pixels.
     *      vpZoom expects the zoom level of the viewport.
     *      desiredTileWidth expects the desired size of tiles in pixels.
     *
     * Returns an integer for the zoom level to use for maps zoom-level. In the range [0, 16].
     */
    int calcMapZoomLevelForTileSizePixels(
        int vpWidth,
        int vpHeight,
        double vpZoom,
        int desiredTileSize = defaultDesiredTileSizePixels);

    /* Calculates the set of visible tiles in a viewport.
     *
     * The method is described in the report in the figure with caption
     * "Calculating set of tiles within viewport"
     *
     * vpAspect expects the aspect ratio of the viewport, expressed as a fraction width / height.
     *
     * Returns a list of tile-coordinates.
     */
    QVector<TileCoord> calcVisibleTiles(
        double vpX,
        double vpY,
        double vpAspect,
        double vpZoomLevel,
        int mapZoomLevel);

    /*  Main rendering function for painting the map. This function will iterate
     *  over multiple tiles and place them correctly on screen.
     *
     *  The output is rendered into the apainter-object.
     *
     *  The tiles rendered are based on the stylesheet and the tilecontainer passed in.
     *
     *  Parameters:
     *      tileContainer: Contains all the tile-data available at this point in time.
     *      styleSheet: Function will read the layer styling data in this object to determine how to render
     *                  individual tiles.
     */
    void paintVectorTiles(
        QPainter &painter,
        double vpX,
        double vpY,
        double viewportZoomLevel,
        int mapZoomLevel,
        const QMap<TileCoord, const VectorTile*> &tileContainer,
        const StyleSheet &styleSheet,
        bool drawDebug);

    /*!
     * \brief paintPngTiles
     * \param painter
     * \param vpX
     * \param vpY
     * \param viewportZoomLevel
     * \param mapZoomLevel
     * \param tileContainer
     * \param styleSheet
     * \param drawDebug
     */
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
