#ifndef MAPZOOMCONTROLWIDGET_H
#define MAPZOOMCONTROLWIDGET_H

// Qt header files.
#include <QWidget>

// Other header files.
#include "MapWidget.h"

namespace Bach {
/*!
 * \class MapZoomControlWidget
 * \brief The MapZoomControlWidget class contains the group
 * of controls for zooming, made to be used in tandem with a MapWidget.
 */
class MapZoomControlWidget : public QWidget {
public:
    MapZoomControlWidget(MapWidget* parent);
};
}

#endif // MAPZOOMCONTROLWIDGET_H
