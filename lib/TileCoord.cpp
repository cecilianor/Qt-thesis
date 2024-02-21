#include "TileCoord.h"

QString TileCoord::toString() const {
    // Create formatted string based on the members.
    return QString("zoom %1 (%2, %3)")
        .arg(zoom)
        .arg(x)
        .arg(y);
}

bool TileCoord::operator<(TileCoord const& other) const {
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

bool TileCoord::operator==(TileCoord const& other) const {
    return zoom == other.zoom && x == other.x && y == other.y;
}
