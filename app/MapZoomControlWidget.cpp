// Qt header files.
#include <QPushButton>
#include <QVBoxLayout>

// Other header files.
#include "MapZoomControlWidget.h"

using Bach::MapZoomControlWidget;

/*!
 * \brief MapZoomControlWidget::MapZoomControlWidget
 * Controls zooming in the map application.
 *
 * \param parent The MapWidget application that map zoom controls are attached to.
 */
MapZoomControlWidget::MapZoomControlWidget(MapWidget* parent) : QWidget(parent)
{
    // Set up application buttons in a simple vertical box layout.
    auto vLayout = new QVBoxLayout(this);
    setLayout(vLayout);

    // Then, set up each individual button and connect them to the MapWidget events.
    auto zoomInBtn = new QPushButton("+", this);
    QObject::connect(zoomInBtn, &QPushButton::clicked, parent, &MapWidget::zoomIn);
    vLayout->addWidget(zoomInBtn);

    auto zoomOutBtn = new QPushButton("-", this);
    QObject::connect(zoomOutBtn, &QPushButton::clicked, parent, &MapWidget::zoomOut);
    vLayout->addWidget(zoomOutBtn);
}
