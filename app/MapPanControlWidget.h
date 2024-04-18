#ifndef MAPPANCONTROLWIDGET_H
#define MAPPANCONTROLWIDGET_H

// Qt header files
#include <QWidget>

// Other header files
#include "MapWidget.h"

namespace Bach {
    /*!
     * \class
     * Contains the group of controls for panning,
     * made to be used in tandem with MapWidget.
     */
    class MapPanControlWidget : public QWidget {
    public:
        MapPanControlWidget(MapWidget* parent);
    };
}

#endif // MAPPANCONTROLWIDGET_H
