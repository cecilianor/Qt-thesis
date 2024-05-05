// Qt header files
#include <QCoreApplication>
#include <QKeyEvent>
#include <QtMath>
#include <QPainter>
#include <QtMath>
#include <QWheelEvent>

// Other header files
#include "MapWidget.h"
#include "Rendering.h"

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
 * \brief MapWidget::mousePressEvent registers if the mouse is pressed.
 *
 * \param event is the event that registered if the mouse is pressed.
 */
void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::MouseButton::LeftButton) {
        mouseStartPosition = event->pos();
    }
}

/*!
 * \brief MapWidget::mouseReleaseEvent registers that mouse buttons are released.
 *
 * The function can reset the mouseStartPosition variable to the point {-1, -1}.
 *
 * \param event is the event that fires if mouse buttons are released.
 */
void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // mouseStartPosition = {-1,-1};
}

/*!
 * \brief MapWidget::mouseMoveEvent records mouse position while a
 * mouse button is pressed.
 *
 * Note that the left mouse button has to be pressed at the same time
 * for this function to run.
 *
 * \param event is the event where the mouse is moved around.
 */
void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Check if the left mouse button is pressed
    if (event->buttons() & Qt::LeftButton) {
        mouseCurrentPosition = event->pos();

        // Calculate the difference between the current and original mouse position.
        QPointF diff = mouseCurrentPosition - mouseStartPosition;

        // Scaling factor used when zooming.
        auto scalar = 1/(std::pow(2, getViewportZoomLevel()));

        // Calculate window aspect ratio, used to scale x coordinate
        // correctly. This was added in after talking to ChatGPT about
        // what could be the cause of the problem.
        double windowAspectRatio = static_cast<double>(width())
                                   / static_cast<double>(height());

        // Scale the difference variable based on zoom level.
        diff *= scalar;

        // Scale the difference variable based on aspect ratio.
        // This makes the mouse cursor stay hovered over the exact area
        // where it was when the left mouse button was clicked.
        if (width() < height())
            diff.rx() *= windowAspectRatio;
        else if (width() > height())
            diff.ry() /= windowAspectRatio;

        // Translate normalized coordinates to world coordinate space.
        auto world_x = x * width();
        auto world_y = y * height();

        // Find where to move the position to in the world coordinate space.
        auto new_x = world_x - diff.rx();
        auto new_y = world_y - diff.ry();

        // Normalise the new coordinates so they can be put back in the norm space.
        auto new_x_norm = Bach::normalizeValueToZeroOneRange(new_x, 0, width());
        auto new_y_norm = Bach::normalizeValueToZeroOneRange(new_y, 0, height());

        // Move to the map to the new position.
        x = new_x_norm;
        y = new_y_norm;

        // Store the current mouse position before re-rendering.
        mouseStartPosition = mouseCurrentPosition;

        // Call update to render the window.
        update();
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
