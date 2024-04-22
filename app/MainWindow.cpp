// Qt header files
#include <QShowEvent>

// Other header files
#include "MainWindow.h"
#include "MapCoordControlWidget.h"
#include "MapPanControlWidget.h"
#include "MapRenderSettingsWidget.h"
#include "MapZoomControlWidget.h"

using Bach::MainWindow;

MainWindow::MainWindow(MapWidget* mapWidgetIn) : mapWidget{ mapWidgetIn }
{
    resize(800, 800);
    setCentralWidget(mapWidget);

    // Establish the UI controls.
    zoomControls = new MapZoomControlWidget(mapWidget);
    panControls = new MapPanControlWidget(mapWidget);
    renderControls = new MapRenderSettingsWidget(mapWidget);

    // Setup the menu that lets us enter manual coordinates and hook it up to the
    // map-widget.
    MapCoordControlWidget *coordControls = new MapCoordControlWidget(mapWidget);
    QObject::connect(
        coordControls,
        &MapCoordControlWidget::submitNewCoords,
        mapWidget,
        &MapWidget::setViewport);

    mapWidget->focusWidget();

    // Positioning of the hovering controls happens in the showEvent method.
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateControlsPositions();
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
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

    if (renderControls != nullptr) {
        renderControls->move(
            width() - renderControls->width(),
            0);
    }
}
