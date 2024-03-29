#ifndef MAPCOORDCONTROLWIDGET_H
#define MAPCOORDCONTROLWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QBoxLayout>

#include "MapWidget.h"

namespace Bach {
    /* Contains the group of controls for manually entering coordinates.
     */
    class MapCoordControlWidget : public QWidget {
        Q_OBJECT

    public:
        MapCoordControlWidget(MapWidget* parent);

    private:
        QLineEdit* longitudeField = nullptr;
        QLineEdit* latitudeField = nullptr;
        QLineEdit* zoomField = nullptr;
        void setupInputFields(QBoxLayout* outerLayout);
        void setupButtons(QBoxLayout* outerLayout, MapWidget* mapWidget);

    private slots:
        void submitButtonPressed();

    signals:
        /* Signal gets called when the coordinates wants to submit
         * a new valid viewport configuration.
         */
        void submitNewCoords(double x, double y, double zoom);
        void loadNewTiles();
    };
}

#endif // MAPCOORDCONTROLWIDGET_H
