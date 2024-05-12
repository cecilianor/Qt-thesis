// Qt header files.
#include <QCoreApplication>
#include <QKeyEvent>
#include <QtMath>
#include <QPainter>
#include <QtMath>
#include <QWheelEvent>

// Other header files.
#include "MapWidget.h"
#include "Rendering.h"

/*!
 * \brief The TileType enum determines what tile type to render.
 *
 *  Supported types: VectorTile, ImageTile
 */
enum class TileType {
    VectorTile,
    ImageTile, // Can be png, jpg, or other types.
};


/*!
 * \brief MapWidget::MapWidget
 * Constructs a MapWidget and attaches a filter for key presses.
 *
 * \param parent The QWidget to attach the MapWidget to.
 */
MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    // Establish and install the keypress filter.
    this->keyPressFilter = std::make_unique<KeyPressFilter>(this);
    QCoreApplication::instance()->installEventFilter(this->keyPressFilter.get());
}

/*!
 * \brief MapWidget::~MapWidget
 * Destructor for the MapWidget class.
 *
 * The destructor removes the keypress filter that's used to handle
 * application key presses.
 */
MapWidget::~MapWidget()
{
    // Remember to remove the keypress filter.
    QCoreApplication::instance()->removeEventFilter(this->keyPressFilter.get());
}

/*!
 * \brief MapWidget::keyPressEvent
 * Handles key presses in the map application.
 *
 * Supported keys:
 * Arrow keys to move around (up, down, left, right).
 * W zooms in.
 * S zooms out.
 *
 * \param event The event where keyboard keys were pressed.
 */
void MapWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key::Key_Up)
        panUp();
    else if (event->key() == Qt::Key::Key_Down)
        panDown();
    else if (event->key() == Qt::Key::Key_Left)
        panLeft();
    else if (event->key() == Qt::Key::Key_Right)
        panRight();
    else if (event->key() == Qt::Key::Key_W)
        zoomIn();
    else if (event->key() == Qt::Key::Key_S)
        zoomOut();
    else
        QWidget::keyPressEvent(event);
}

/*!
 * \brief MapWidget::mousePressEvent registers if the mouse is pressed.
 *
 * \param event is the event that registered if the mouse is pressed.
 */
void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::MouseButton::LeftButton)
        mouseStartPosition = event->pos();

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
    // This signal should run every time a new tile is loaded later.
    auto signalFn = [this](TileCoord newTile) {
        // Possible optimization:
        // Check if this new tile is relevant anymore, and only
        // issue redraw if it is.
        update();
    };
    // Request tiles.
    QScopedPointer<Bach::RequestTilesResult> requestResult = requestTilesFn(
        tilesRequested,
        signalFn);

    QPainter painter(this);

    if (isRenderingVector()) {
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
    } else {
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

/*!
 * \brief MapWidget::getViewportZoomLevel
 * Gets the zoom level of the viewport.
 *
 * \return The zoom level of the viewport.
 */
double MapWidget::getViewportZoomLevel() const
{
    return viewportZoomLevel;
}

/*!
 * \brief MapWidget::getMapZoomLevel
 * Gets the zoom level of the map.
 *
 * \return The zoom level of the map.
 */
int MapWidget::getMapZoomLevel() const
{
    // Calculate the map zoom level based on the viewport,
    return Bach::calcMapZoomLevelForTileSizePixels(
        width(),
        height(),
        getViewportZoomLevel());
}

/*!
 * \brief MapWidget::calcVisibleTiles
 * Calculates which tiles to be displayed in the viewport.
 *s
 * \return A QVector containing the coordinates if tiles that should be visible.
 */
QVector<TileCoord> MapWidget::calcVisibleTiles() const
{
    return Bach::calcVisibleTiles(
        x,
        y,
        (double)width() / height(),
        getViewportZoomLevel(),
        getMapZoomLevel());
}

/*!
 * \brief MapWidget::getPanStepAmount
 * Gets how much to pan when a panning key is pressed on the keyboard.
 * The amount to pan varies based on how zoomed in the viewport is.
 *
 * \return A number (double) representing how much to pan when panning with arrow keys.
 */
double MapWidget::getPanStepAmount() const
{
    return 0.1 / pow(2, getViewportZoomLevel());
}

/*!
 * \brief MapWidget::genericZoom
 * Zooms in or out the viewport in the application.
 *
 * \param magnify If the viewport should be zoomed in (true) or not (false).
 */
void MapWidget::genericZoom(bool magnify)
{
    if (magnify)
        viewportZoomLevel += 0.1;
    else
        viewportZoomLevel -= 0.1;
    update();
}

/*!
 * \brief MapWidget::zoomIn
 * Zooms in a single step.
 */
void MapWidget::zoomIn()
{
    genericZoom(true);
}

/*!
 * \brief MapWidget::zoomOut
 * Zooms out a single step.
 */
void MapWidget::zoomOut()
{
    genericZoom(false);
}

/*!
 * \brief MapWidget::panUp
 * Pans a single step up.
 */
void MapWidget::panUp()
{
    auto amount = getPanStepAmount();
    y -= amount;
    update();
}

/*!
 * \brief MapWidget::panDown
 * Pans a single step down.
 */
void MapWidget::panDown()
{
    auto amount = getPanStepAmount();
    y += amount;
    update();
}

/*!
 * \brief MapWidget::panLeft
 * Pans a single step to the left.
 */
void MapWidget::panLeft()
{
    auto amount = getPanStepAmount();
    x -= amount;
    update();
}

/*!
 * \brief MapWidget::panRight
 * Pans a single step to the right.
 */
void MapWidget::panRight()
{
    auto amount = getPanStepAmount();
    x += amount;
    update();
}

/*!
 * \brief MapWidget::setViewport
 * Updates the center coordinates and the zoom-level of the viewport.
 *
 * \param xIn The width of the viewport.
 * \param yIn The height of the viewport.
 * \param zoomIn The zoom level to set.
 */
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
/*!
 * \brief MapWidget::KeyPressFilter::eventFilter
 * Filters the arrow key events of the QApplication
 * and feeds them directly to the MapWidget whenever it's alive.
 *
 * \param obj Used to pass the key event to its parent classs.
 * \param event The arrow key event to be filtered.
 * \return True if the arrow keys have been pressed.
 */
bool MapWidget::KeyPressFilter::eventFilter(QObject *obj, QEvent *event)
{  
    // Only intercept keypress events.
    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = static_cast<QKeyEvent*>(event);

        if ( keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right
            || keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            mapWidget->keyPressEvent(keyEvent);
            return true;
        }
    }
    // Pass the event on to the parent class for default processing.
    return QObject::eventFilter(obj, event);
}

/*!
 * \brief MapWidget::setShouldDrawFill
 * Controls if filled in colors should be drawn on the map or not.
 *
 * \param drawFill indicates if color should be filled in (true) or not (false).
 */
void MapWidget::setShouldDrawFill(bool drawFill)
{
    renderFill = drawFill;
    update();
}

/*!
 * \brief MapWidget::setShouldDrawLines
 * Controls if lines should be drawn on the map or not.
 *
 * \param indicates if lines should be drawn (true) or not (false).
 */
void MapWidget::setShouldDrawLines(bool drawLines)
{
    renderLines = drawLines;
    update();
}

/*!
 * \brief MapWidget::setShouldDrawText
 * Controls if text should be drawn on the map or not.
 *
 * \param drawText indicates if text should be drawn (true) or not (false).
 */
void MapWidget::setShouldDrawText(bool drawText)
{
    renderText = drawText;
    update();
}

/*!
 * \brief MapWidget::toggleIsShowingDebug
 * Toggles if the debug menu and lines should be shown or not.
 */
void MapWidget::toggleIsShowingDebug()
{
    showDebug = !showDebug;
    update();
}

/*!
 * \brief MapWidget::toggleIsRenderingVectorTile
 * Toggles if the vector map type should be rendered or not.
 */
void MapWidget::toggleIsRenderingVectorTile()
{
    renderVectorTile = !renderVectorTile;
    update();
}
