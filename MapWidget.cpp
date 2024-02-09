#include "MapWidget.h"

#include <QPainter>
#include <QKeyEvent>
#include <QtMath>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Map Test Widget");
    resize(800, 800);
}

MapWidget::~MapWidget() {

}

void MapWidget::keyPressEvent(QKeyEvent* event) {
    auto amount = 0.1 / pow(2, getViewportZoomLevel());
    if (event->key() == Qt::Key::Key_Up) {
        this->y -= amount;
        this->update();
    } else if (event->key() == Qt::Key::Key_Down) {
        this->y += amount;
        this->update();
    } else if (event->key() == Qt::Key::Key_Left) {
        this->x -= amount;
        this->update();
    } else if (event->key() == Qt::Key::Key_Right) {
        this->x += amount;
        this->update();
    } else if (event->key() == Qt::Key::Key_W) {
        this->viewportZoomLevel += 0.1;
        this->update();
    } else if (event->key() == Qt::Key::Key_S) {
        this->viewportZoomLevel -= 0.1;
        this->update();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void MapWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    Bach::paintTiles(
        painter,
        x,
        y,
        getViewportZoomLevel(),
        getMapZoomLevel(),
        tileStorage);
}
