#ifndef MAPZOOMCONTROLWIDGET_H
#define MAPZOOMCONTROLWIDGET_H

#include <QWidget>
#include "MapWidget.h"

namespace Bach {
    /* Contains the group of controls for zooming,
     * made to be used in tandem with MapWidget.
     */
    class MapZoomControlWidget : public QWidget {
    public:
        MapZoomControlWidget(MapWidget* parent);
    };
}


#endif // MAPZOOMCONTROLWIDGET_H
