#include "MapWidget.h"

#include "Rendering.h"

#include <QtMath>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QPainter>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    // Establish and install our keypress filter.
    this->keyPressFilter = std::make_unique<KeyPressFilter>(this);
    QCoreApplication::instance()->installEventFilter(this->keyPressFilter.get());
}

MapWidget::~MapWidget()
{
    // Remember to remove our keypress filter.
    QCoreApplication::instance()->removeEventFilter(this->keyPressFilter.get());
}

void MapWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key::Key_Up) {
        panUp();
    } else if (event->key() == Qt::Key::Key_Down) {
        panDown();
    } else if (event->key() == Qt::Key::Key_Left) {
        panLeft();
    } else if (event->key() == Qt::Key::Key_Right) {
        panRight();
    } else if (event->key() == Qt::Key::Key_W) {
        zoomIn();
    } else if (event->key() == Qt::Key::Key_S) {
        zoomOut();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void MapWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    Bach::paintTiles(
        painter,
        x,
        y,
        getViewportZoomLevel(),
        getMapZoomLevel(),
        tileStorage,
        styleSheet);
}

double MapWidget::getViewportZoomLevel() const
{
    return viewportZoomLevel;
}

int MapWidget::getMapZoomLevel() const
{
    // Calculate the map zoom level based on the viewport,
    // or just return the overriden value.
    if (overrideMapZoom) {
        return overrideMapZoomLevel;
    } else {
        return Bach::calcMapZoomLevelForTileSizePixels(
            width(),
            height(),
            getViewportZoomLevel());
    }
}

QVector<TileCoord> MapWidget::calcVisibleTiles() const
{
    return Bach::calcVisibleTiles(
        x,
        y,
        (double)width() / height(),
        getViewportZoomLevel(),
        getMapZoomLevel());
}

double MapWidget::getPanStepAmount() const
{
    return 0.1 / pow(2, getViewportZoomLevel());
}

void MapWidget::genericZoom(bool magnify)
{
    if (magnify)
        viewportZoomLevel += 0.1;
    else
        viewportZoomLevel -= 0.1;
    this->update();
}

void MapWidget::zoomIn()
{
    genericZoom(true);
}

void MapWidget::zoomOut()
{
    genericZoom(false);
}

void MapWidget::panUp()
{
    auto amount = getPanStepAmount();
    this->y -= amount;
    this->update();
}

void MapWidget::panDown()
{
    auto amount = getPanStepAmount();
    this->y += amount;
    this->update();
}

void MapWidget::panLeft()
{
    auto amount = getPanStepAmount();
    this->x -= amount;
    this->update();
}

void MapWidget::panRight()
{
    auto amount = getPanStepAmount();
    this->x += amount;
    this->update();
}

bool MapWidget::KeyPressFilter::eventFilter(QObject *obj, QEvent *event)
{
    //if (!mapWidget->hasFocus())
        //return QObject::eventFilter(obj, event);

    // Only intercept keypress events.
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            mapWidget->keyPressEvent(keyEvent);
            // Mark event as handled
            return true;
        default:
             // Do not intercept other key presses
            break;
        }
    }
    // Pass the event on to the parent class for default processing
    return QObject::eventFilter(obj, event);
}
