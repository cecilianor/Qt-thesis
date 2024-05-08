#ifndef MAPCOORDCONTROLWIDGET_H
#define MAPCOORDCONTROLWIDGET_H

// Qt header files.
#include <QBoxLayout>
#include <QLineEdit>
#include <QWidget>

// Other header files
#include "MapWidget.h"

namespace Bach
{
/*!
 * \class MapCoordControlWidget
 * \brief The MapCoordControlWidget class is a collection
 * of controls that modify the location and zoom of the
 * MapWidget's viewport.
 */
class MapCoordControlWidget : public QWidget
{
    Q_OBJECT

private:
    // References to input fields where their contents can be
    // grabbed at runtime.
    QLineEdit* longitudeField = nullptr;
    QLineEdit* latitudeField = nullptr;
    QLineEdit* zoomField = nullptr;

    // Setup functionality for map coordinate controls.
    void setupInputFields(QBoxLayout* outerLayout);
    void setupButtons(QBoxLayout* outerLayout, MapWidget* mapWidget);
    void goButtonPressed();

signals:
    /*!
     * \brief submitNewCoords
     * This signal is emitted whenever the user presses the "Go" button.
     *
     * The function updates map coordinates to the new zoom, x, y position.
     *
     * \param x The x coordinate.
     * \param y The y coordinate.
     * \param zoom The zoom level.
     */
    void submitNewCoords(double x, double y, double zoom);

public:
    MapCoordControlWidget(MapWidget* parent);
};
}

#endif // MAPCOORDCONTROLWIDGET_H
