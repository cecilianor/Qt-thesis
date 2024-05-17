// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

#include "TileCoord.h"

/*!
 * \brief TileCoord::toString generates a string representing a TileCoord.
 *
 * This string is used when rendering debug information in the application.
 *
 * \return coordiante string on the form 'zoom: zoom (x coordinate, y coordinate)'.
 *
 * Example return string: "zoom 1 (1, 0)".
 */
QString TileCoord::toString() const
{
    // Create formatted string based on the members.
    return QString("zoom %1 (%2, %3)")
        .arg(zoom)
        .arg(x)
        .arg(y);
}

/*!
 * \brief TileCoord::operator < defines the less than operator: '<'.
 *
 * The operator can now be used as a key in QMap.
 *
 * \param other is the TileCoord to compare the current TileCoord to.
 * \return true if the current TileCoord values are less than the zoom, x, y of the 'other'.
 */
bool TileCoord::operator<(const TileCoord &other) const
{
    if (zoom < other.zoom)
        return true;
    else if (zoom > other.zoom)
        return false;

    if (x < other.x)
        return true;
    else if (x > other.x)
        return false;

    if (y < other.y)
        return true;
    else
        return false;
}


/*!
 * \brief TileCoord::operator == defines the equality operator: '=='.
 *
 * The operator can now be used as a key in QMap.
 *
 * \param other is the TileCoord to compare the current TileCoord to.
 * \return true if zoom, x, y are the same for two `TileCoord`s.
 */
bool TileCoord::operator==(const TileCoord& other) const
{
    return zoom == other.zoom && x == other.x && y == other.y;
}

/*!
 * \brief TileCoord::operator != defines the inequality operator: '!='.
 *
 *  The operator can now be used as a key in QMap.
 *
 * \param other is the TileCoord to compare the current TileCoord to.
 * \return true if zoom, x, y are not the same for two `TileCoord`s.
 */
bool TileCoord::operator!=(const TileCoord& other) const
{
    return !(*this == other);
}
