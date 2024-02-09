#ifndef TILECOORD_HPP
#define TILECOORD_HPP

#include <QString>

struct TileCoord {
    int zoom;
    int x;
    int y;

    [[nodiscard]] QString toString() const {
        return QString("zoom %1 (%2, %3)")
            .arg(zoom)
            .arg(x)
            .arg(y);
    }

    // Define less-than operator and the equality operator
    // in order to allow using this type as a key in QMap<>
    [[nodiscard]] bool operator<(TileCoord const& other) const noexcept {
        if (zoom < other.zoom) return true;
        if (zoom > other.zoom) return false;
        if (x < other.x) return true;
        if (x > other.x) return false;
        if (y < other.y) return true;
        return false;
    }
    [[nodiscard]] bool operator==(TileCoord const& other) const {
        return zoom == other.zoom && x == other.x && y == other.y;
    }
};

#endif
