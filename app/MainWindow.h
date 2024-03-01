#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>

#include <functional>

#include "MapWidget.h"
#include "VectorTiles.h"
#include "TileCoord.h"

namespace Bach {
    /* Main window class for our application.
     *
     * Constructor sets up the smaller Widgets when necessary.
     */
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        /* We're calling many of the parent type methods from inside
         * override methods in this class.
         *
         * This lets us easily change parent type without much rewriting.
         */
        using ParentType = QMainWindow;

        MainWindow(MapWidget*, std::function<VectorTile(TileCoord)>&&);

        void resizeEvent(QResizeEvent*) override;
        void showEvent(QShowEvent*) override;
        void keyPressEvent(QKeyEvent *event) override {
            if (event->key() == Qt::Key::Key_R) {
                mapWidget->loadNewTiles(loadTileFn);
                return;
            }
            ParentType::keyPressEvent(event);
        }

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
        std::function<VectorTile(TileCoord)> loadTileFn = nullptr;
    };
}
#endif // MAINWINDOW_H
