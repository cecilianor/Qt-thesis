#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt header files.
#include <QKeyEvent>
#include <QMainWindow>

// Other header files.
#include "MapWidget.h"

namespace Bach {
/*! \class Main window Widget class for the application.
     *
     * Constructor sets up the smaller Widgets as well.
     */
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // References to the various widgets whose position must remain updated.
    MapWidget* mapWidget = nullptr;
    QWidget* zoomControls = nullptr;
    QWidget* panControls = nullptr;
    QWidget* renderControls = nullptr;

    void updateControlsPositions();

public:
    MainWindow(MapWidget*);
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
};
}
#endif // MAINWINDOW_H
