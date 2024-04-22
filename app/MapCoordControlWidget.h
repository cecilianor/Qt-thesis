#ifndef MAPCOORDCONTROLWIDGET_H
#define MAPCOORDCONTROLWIDGET_H

// Qt header files
#include <QBoxLayout>
#include <QLineEdit>
#include <QWidget>

// Other header files
#include "MapWidget.h"

namespace Bach {
    /*!
     * \brief The MapCoordControlWidget class is a collection
     * of controls meant to modify the location and zoom of the
     * MapWidgets viewport.
     */
    class MapCoordControlWidget : public QWidget {
        Q_OBJECT

    public:
        MapCoordControlWidget(MapWidget* parent);

    private:
        // References to our input fields so
        // we can grab their contents at runtime.
        QLineEdit* longitudeField = nullptr;
        QLineEdit* latitudeField = nullptr;
        QLineEdit* zoomField = nullptr;

        void setupInputFields(QBoxLayout* outerLayout);
        void setupButtons(QBoxLayout* outerLayout, MapWidget* mapWidget);

    private:
        void goButtonPressed();

    signals:
        /*!
         * This signal is emitted whenever the user presses the "Go" button
         */
        void submitNewCoords(double x, double y, double zoom);
    };
}

#endif // MAPCOORDCONTROLWIDGET_H
