#ifndef MAPZOOMCONTROLWIDGET_H
#define MAPZOOMCONTROLWIDGET_H

// Qt header files
#include <QWidget>

// Other header files
#include "MapWidget.h"

namespace Bach {
    /*!
     * \class
     * Contains the group of controls for zooming,
     * made to be used in tandem with MapWidget.
     */
    class MapZoomControlWidget : public QWidget {
    public:
        MapZoomControlWidget(MapWidget* parent);
    };
}


#endif // MAPZOOMCONTROLWIDGET_H
