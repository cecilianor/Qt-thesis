#include "MainWindow.h"

#include <QShowEvent>
#include <QCoreApplication>

#include "MapPanControlWidget.h"
#include "MapZoomControlWidget.h"
#include "MapCoordControlWidget.h"

using Bach::MainWindow;

MainWindow::MainWindow(
    MapWidget* mapWidgetIn,
    std::function<VectorTile(TileCoord)>&& fn) :
    mapWidget{ mapWidgetIn },
    loadTileFn{ std::move(fn) }
{
    resize(800, 800);
    setCentralWidget(mapWidget);

    // Establish the UI controls.
    zoomControls = new MapZoomControlWidget(mapWidget);
    panControls = new MapPanControlWidget(mapWidget);

    // Setup the menu that lets us enter manual coordinates and hook it up to the
    // map-widget.
    auto coordControls = new MapCoordControlWidget(mapWidget);
    QObject::connect(coordControls, &MapCoordControlWidget::submitNewCoords, mapWidget, &MapWidget::setViewport);

    mapWidget->focusWidget();

    // Positioning of the hovering controls happens in the showEvent method.
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    ParentType::resizeEvent(event);
    updateControlsPositions();
}

void MainWindow::showEvent(QShowEvent *event) {
    ParentType::showEvent(event);
    if (event->type() == QEvent::Show) {
        updateControlsPositions();
    }
}

void MainWindow::updateControlsPositions() {
    // Position the panning controls
    if (panControls != nullptr) {
        panControls->move(
            0,
            height() - panControls->height());
    }

    // Position the zoom controls
    if (zoomControls != nullptr) {
        zoomControls->move(
            width() - zoomControls->width(),
            height() - zoomControls->height());
    }
}
