#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

#include "MapWidget.h"

namespace Bach {
    /* Main window class for our application.
     *
     * Constructor sets up the smaller Widgets when necessary.
     */
    class MainWindow : public QMainWindow {
    public:
        /* We're calling many of the parent type methods from inside
         * override methods in this class.
         *
         * This lets us easily change parent type without much rewriting.
         */
        using ParentType = QMainWindow;

        MainWindow(MapWidget*);

        void resizeEvent(QResizeEvent*) override;
        void showEvent(QShowEvent*) override;

        /* Updates the positions of the floating control widgets (zoom, panning...)
         *
         * Should be called every time the widget changes size.
         * This updates the position of our control widgets,
         * so that they're placed correctly relative to this window.
         */
        void updateControlsPositions();

    private:
        MapWidget* mapWidget = nullptr;
        QWidget* zoomControls = nullptr;
        QWidget* panControls = nullptr;
    };
}
#endif // MAINWINDOW_H
