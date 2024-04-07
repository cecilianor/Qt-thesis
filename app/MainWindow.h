#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

#include "MapWidget.h"

namespace Bach {
    /*! \class Main window Widget class for our application.
     *
     * Constructor sets up the smaller Widgets also.
     */
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(MapWidget*);

        void resizeEvent(QResizeEvent*) override;
        void showEvent(QShowEvent*) override;

    private:
        /* Updates the positions of the floating control widgets (zoom, panning...)
         *
         * Should be called every time the widget changes size.
         * This updates the position of our control widgets,
         * so that they're placed correctly relative to this window.
         */
        void updateControlsPositions();

        // References to the various widgets whose position we need to keep
        // updated.
        MapWidget* mapWidget = nullptr;
        QWidget* zoomControls = nullptr;
        QWidget* panControls = nullptr;
        QWidget* renderControls = nullptr;
    };
}
#endif // MAINWINDOW_H
