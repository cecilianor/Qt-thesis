#ifndef TILERENDERER_H
#define TILERENDERER_H

#include <QPainter>

#include "tilerenderrules.h"
#include "vectortile.h"

class TileRenderer
{
public:
    TileRenderer();

    void render(QPainter *p, const VectorTile &tile);

    int m_zoomLevel;
    TileRenderRules m_rules;
};

#endif // TILERENDERER_H
