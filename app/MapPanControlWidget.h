#ifndef MAPPANCONTROLWIDGET_H
#define MAPPANCONTROLWIDGET_H

#include <QWidget>
#include "MapWidget.h"

namespace Bach {
    /* Contains the group of controls for panning,
     * made to be used in tandem with MapWidget.
     */
    class MapPanControlWidget : public QWidget {
    public:
        MapPanControlWidget(MapWidget* parent);
    };
}

#endif // MAPPANCONTROLWIDGET_H
