#include "MapWidget.h"

#include "Rendering.h"

#include <QtMath>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QWheelEvent>

/*!
 * \brief The TileType enum determines what tile type to render.
 *
 * Supported types:
 *
 * * VectorTile
 * * ImageTile
 */
enum class TileType {
    VectorTile,
    ImageTile, // Can be png, jpg, ...
};


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

/*!
 * \brief wheelEvent handles mouse wheel or touch pad scrolling.
 *
 * The function only implements vertical movement, and has been
 * tested on a standard vertical mouse that scrolls "up" and "down".
 *
 * Note that QWheelEvent can also handle horizontal ("sideways") scrolling,
 * so MapWidget::wheelEvent can be expanded to support that in the future
 * if desired.
 *
 * \param event records the wheel being moved horizontally or vertically.
 */
void MapWidget::wheelEvent(QWheelEvent *event) {
    {
        // Calculations are provided by Qt's own source example, here:
        // https://doc.qt.io/qt-6/qwheelevent.html#angleDelta
        QPoint numPixels = event->pixelDelta();
        QPoint numDegrees = event->angleDelta() / 8;

        // Check if degrees or pixels were used to record/measure scrolling.
        // A positive y value means the wheel was moved vertically away from the user.
        // A negative y value means the wheel was moved vertically towards the user.
        if (!numPixels.isNull()) {
            if (numPixels.y() > 0)
                zoomIn();
            else if (numPixels.y() < 0)
                zoomOut();
        } else if (!numDegrees.isNull()) {
            if (numDegrees.y() > 0)
                zoomIn();
            else if (numDegrees.y() < 0)
                zoomOut();
        }
        // accept() is called to indicate that the receiver wants
        // the mouse wheel event. Check the following for more info:
        // https://doc.qt.io/qt-6/qwheelevent.html#QWheelEvent-2
        event->accept();
    }
}


void MapWidget::paintEvent(QPaintEvent *event)
{
    QVector<TileCoord> visibleTiles = calcVisibleTiles();
    std::set<TileCoord> tilesRequested{ visibleTiles.begin(), visibleTiles.end()};
    // We want this signal to run every time a new tile is loaded later.
    auto signalFn = [this](TileCoord newTile) {
        // Possible optimization:
        // Check if this new tile is relevant anymore, and only
        // issue redraw if it is.

        update();
    };
    // Request tiles
    QScopedPointer<Bach::RequestTilesResult> requestResult = requestTilesFn(
        tilesRequested,
        signalFn);

    QPainter painter(this);

    if (isRenderingVector())
    {
        // Set up the paint settings based on the MapWidget configuration.
        Bach::PaintVectorTileSettings paintSettings = Bach::PaintVectorTileSettings::getDefault();
        paintSettings.drawFill = isRenderingFill();
        paintSettings.drawLines = isRenderingLines();
        paintSettings.drawText = isRenderingText();

        // Then run the function to paint all vector tiles into this MapWidget.
        Bach::paintVectorTiles(
            painter,
            x,
            y,
            getViewportZoomLevel(),
            getMapZoomLevel(),
            requestResult->vectorMap(),
            requestResult->styleSheet(),
            paintSettings,
            isShowingDebug());
    }
    else
    {
        Bach::paintRasterTiles(
            painter,
            x,
            y,
            getViewportZoomLevel(),
            getMapZoomLevel(),
            requestResult->rasterImageMap(),
            requestResult->styleSheet(),
            isShowingDebug());
    }
}

double MapWidget::getViewportZoomLevel() const
{
    return viewportZoomLevel;
}

int MapWidget::getMapZoomLevel() const
{
    // Calculate the map zoom level based on the viewport,
    return Bach::calcMapZoomLevelForTileSizePixels(
        width(),
        height(),
        getViewportZoomLevel());
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
    update();
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
    y -= amount;
    update();
}

void MapWidget::panDown()
{
    auto amount = getPanStepAmount();
    y += amount;
    update();
}

void MapWidget::panLeft()
{
    auto amount = getPanStepAmount();
    x -= amount;
    update();
}

void MapWidget::panRight()
{
    auto amount = getPanStepAmount();
    x += amount;
    update();
}

void MapWidget::setViewport(double xIn, double yIn, double zoomIn)
{
    bool change = false;
    if (x != xIn || y != yIn || viewportZoomLevel != zoomIn)
        change = true;
    x = xIn;
    y = yIn;
    viewportZoomLevel = zoomIn;
    if (change) {
        update();
    }
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

void MapWidget::setShouldDrawFill(bool drawFill)
{
    renderFill = drawFill;
    update();
}

void MapWidget::setShouldDrawLines(bool drawLines)
{
    renderLines = drawLines;
    update();
}

void MapWidget::setShouldDrawText(bool drawText)
{
    renderText = drawText;
    update();
}

void MapWidget::toggleIsShowingDebug()
{
    showDebug = !showDebug;
    update();
}

void MapWidget::toggleIsRenderingVectorTile()
{
    renderVectorTile = !renderVectorTile;
    update();
}
