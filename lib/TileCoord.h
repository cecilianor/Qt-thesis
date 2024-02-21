#ifndef TILECOORD_HPP
#define TILECOORD_HPP

#include <QString>

// Represents the position of a tile within the maps grid at a given zoom level.
//
// This is the C++ equivalent of the tile-position-triplet in the report.
struct TileCoord {
    int zoom = 0;
    int x = 0;
    int y = 0;

    QString toString() const;

    // Define less-than operator and the equality operator
    // in order to allow using this type as a key in QMap
    bool operator<(TileCoord const& other) const;
    bool operator==(TileCoord const& other) const;
};

#endif
