#include "Rendering.h"

/*!
 * \brief Bach::lonLatToWorldNormCoord converts longitude and latitude to world-normalized coordinates.
 *
 * Takes radians, not degrees.
 * The math formula employed is described by the figure in the report with the caption
 * "Converting longitude and latitude to world-normalized coordinates".
 *
 * \param lon is the longitude measured in radians.
 * \param lat is the latitude measured in radians.
 * \return a pair containing the normalized longitude and latitude coordinates (radians).
 */
Bach::MapCoordinate Bach::lonLatToWorldNormCoord(double lon, double lat)
{
    constexpr double webMercatorPhiCutoff = 1.4844222297;

    // Convert longitude and latitude to radians
    auto lambda = lon;
    auto phi = lat;

    // Convert to Web Mercator
    auto x = lambda;
    auto y = std::log(std::tan(M_PI / 4.0 + phi / 2.0));

    // Normalize x and y to [0, 1]
    // Assuming the Web Mercator x range is [-π, π] and y range is calculated from latitude range
    auto xNormalized = normalizeValueToZeroOneRange(x, -M_PI, M_PI);
    // We have to flip the sign of Y, because Mercator has positive Y moving up,
    // while the world-normalized coordinate space has Y moving down.
    auto yNormalized = normalizeValueToZeroOneRange(
        -y,
        std::log(std::tan(M_PI / 4.0 + -webMercatorPhiCutoff / 2.0)),
        std::log(std::tan(M_PI / 4.0 + webMercatorPhiCutoff / 2.0)));


    return { xNormalized, yNormalized };
}

/* Converts longitude and latitude to world-normalized coordinates.
     * Takes degrees.
     */

/*!
 * \brief Bach::lonLatToWorldNormCoordDegrees converts longitude and latitude to world-normalized coordinates.
     * Takes degrees, not radians.
 * \param lon is the longitude measured in degrees.
 * \param lat is the latitude measured in degrees.
 * \return a pair containing the normalized longitude and latitude coordinates (degrees).
 */
Bach::MapCoordinate Bach::lonLatToWorldNormCoordDegrees(double lon, double lat)
{
    auto degToRad = [](double deg) {
        return deg * M_PI / 180.0;
    };
    return lonLatToWorldNormCoord(degToRad(lon), degToRad(lat));
}

/*!
 * \brief Bach::calcMapZoomLevelForTileSizePixels calculates zoom level to get displayed tile size as close as possible to desiredTileWidth.
 *
 * \param vpWidth is the width of the viewport in pixels.
 * \param vpHeight is the height of the viewport in pixels.
 * \param vpZoom is the zoom level of the viewport.
 * \param desiredTileWidth is the desired size of tiles in pixels.
 * \return an integer for the zoom level to use for map's zoom-level, in the range [0, 16].
 */
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

/*!
 * \brief Bach::calcViewportSizeNorm calculates width and height of the viewport in world-normalized coordinates.
 *
 * This means the size expressed as a fraction of the world map. For example,
 * a viewportZoom set to 0 will return size as 1, while a zoom level of
 * 1 will return 0.5. This takes the aspect ratio of the viewport into account,
 * with the largest side being mapped to relation mentioned.
 *
 * The math formula used is described by the figure in the report with the caption
 * "Calculating viewport size as a factor of the world map".
 *
 * \param vpZoomLevel is the zoom level of the viewport.
 * \param viewportAspect is the aspect ratio of the viewport.
 * \return width and height as fractions, in the range [0, 1].
 */
Bach::MapCoordinate Bach::calcViewportSizeNorm(double vpZoomLevel, double viewportAspect) {
    // Math formula can be seen in the figure in the report, with the caption
    // "Calculating viewport size as a factor of the world map"
    auto temp = 1 / pow(2, vpZoomLevel);
    return {
        temp * qMin(1.0, viewportAspect),
        temp * qMin(1.0, 1 / viewportAspect)
    };
}

/*!
 * \brief Bach::calcVisibleTiles calculates the set of visible tiles in a viewport.
 *
 * The method is described in the report in the figure with caption
 * "Calculating set of tiles within viewport"
 *
 * vpAspect expects the aspect ratio of the viewport, expressed as a fraction width / height.
 *
 * \param vpX ?
 * \param vpY ?
 * \param vpAspect ?
 * \param vpZoomLevel is the zoom level of the viewport.
 * \param mapZoomLevel is the zoom level of the map (between what values?).
 * \return a list of tile-coordinates.
 */
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

/*!
 * \brief normalizeValueToZeroOneRange normalizes a value from its original range to [0, 1]
 * \param value is the value to normalize.
 * \param min is the lowest possible value in the original range.
 * \param max is the highest possible value in the original range.
 * \return the normalized, new value in the [0, 1] range.
 */
double Bach::normalizeValueToZeroOneRange(double value, double min, double max)
{
    const double epsilon = 0.0001;
    // Return 0 if the divisor is approaching 0 (illegal mathematically).
    //
    // Note that the result will approach infinity if the divisor is
    // approaching 0. We return 0.0 here to keep the returned
    // value within the required [0, 1] space.
    if(max-min < epsilon)
        return 0.0;
    else
        return (value - min) / (max - min);
}
