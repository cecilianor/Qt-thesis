#include "MainWindow.h"

#include <QShowEvent>
#include <QCoreApplication>

#include "MapPanControlWidget.h"
#include "MapZoomControlWidget.h"

using Bach::MainWindow;

MainWindow::MainWindow(MapWidget* mapWidgetIn) : mapWidget{ mapWidgetIn }  {
    resize(800, 800);
    setCentralWidget(mapWidget);

    this->zoomControls = new MapZoomControlWidget(mapWidget);
    this->panControls = new MapPanControlWidget(mapWidget);

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
