// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

#ifndef TILECOORD_HPP
#define TILECOORD_HPP

// Qt header files
#include <QString>

// Represents the position of a tile within the maps grid at a given zoom level.
//
// This is the C++ equivalent of the tile-position-triplet in the report.
struct TileCoord {
    /* Map zoom level of this tile's position. Range [0, 16].
     */
    int zoom = 0;

    /* X direction index-coordinate of this tile.
     *
     * Should always be in the range [0, tilecount-1]
     * where tilecount = 2^zoom
     */
    int x = 0;
    /* Y direction index-coordinate of this tile.
     *
     * Should always be in the range [0, tilecount-1]
     * where tilecount = 2^zoom
     */
    int y = 0;

    QString toString() const;

    // Define less-than operator, equality operator, and inequality
    // operator in order to allow using this type as a key in QMap.
    bool operator<(const TileCoord &other) const;
    bool operator==(const TileCoord &other) const;
    bool operator!=(const TileCoord &other) const;
};

#endif // TILECOORD_HPP
