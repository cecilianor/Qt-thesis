// Qt header files.
#include <QShowEvent>

// Other header files.
#include "MainWindow.h"
#include "MapCoordControlWidget.h"
#include "MapPanControlWidget.h"
#include "MapRenderSettingsWidget.h"
#include "MapZoomControlWidget.h"

using Bach::MainWindow;

/*!
 * \brief MainWindow::MainWindow
 * Sets up the main window of the map application.
 *
 * The window is set to be 800x800 px at the start. The function sets up
 * window controls, and allows users to input coordinates to move
 * around the window.
 *
 * \param mapWidgetIn The input map widget.
 */
MainWindow::MainWindow(MapWidget* mapWidgetIn) : mapWidget{ mapWidgetIn }
{
    resize(800, 800);
    setCentralWidget(mapWidget);

    // Establish the UI controls.
    zoomControls = new MapZoomControlWidget(mapWidget);
    panControls = new MapPanControlWidget(mapWidget);
    renderControls = new MapRenderSettingsWidget(mapWidget);

    // Set up the menu that lets the user enter manual coordinates and
    // connect them to the map widget.
    MapCoordControlWidget *coordControls = new MapCoordControlWidget(mapWidget);
    QObject::connect(
        coordControls,
        &MapCoordControlWidget::submitNewCoords,
        mapWidget,
        &MapWidget::setViewport);

    mapWidget->focusWidget();

    // Positioning of the hovering controls happens in the showEvent method.
}

/*!
 * \brief MainWindow::resizeEvent
 * Handles resizing of the application window.
 *
 * \param event The event that triggers window resizing.
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateControlsPositions();
}

/*!
 * \brief MainWindow::showEvent
 * Shows/displays window controls at the correct position in a window.
 *
 * \param event The event that triggers controls to be shown.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (event->type() == QEvent::Show) {
        updateControlsPositions();
    }
}

/*!
 * \brief MainWindow::updateControlsPositions
 * Updates the positions of the floating control widgets (zooming and panning).
 *
 * updateControlsPositions is called every time the widget changes size.
 * This updates the position of the control widgets
 * so that they're placed correctly relative to this window.
 */
void MainWindow::updateControlsPositions()
{
    // Position the panning controls.
    if (panControls != nullptr) {
        panControls->move(
            0,
            height() - panControls->height());
    }
    // Position the zooming controls.
    if (zoomControls != nullptr) {
        zoomControls->move(
            width() - zoomControls->width(),
            height() - zoomControls->height());
    }
    // Position the rendering controls.
    if (renderControls != nullptr) {
        renderControls->move(
            width() - renderControls->width(),
            0);
    }
}
