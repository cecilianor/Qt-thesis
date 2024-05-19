// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Qt header files.
#include <QScopedPointer>
#include <QWidget>

// STL header files.
#include <functional>
#include <set>

// Other header files.
#include "RequestTilesResult.h"
#include "TileCoord.h"

/*!
 * \class MapWidget
 * \brief The MapWidget class is responsible for displaying a map.
 *
 * It should be used as a smaller widget within a larger Widget hierarchy.
 * The MapWidget has a built-in viewport configuration (zoom level and center coordinates).
 */
class MapWidget : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \class KeyPressFilter
     * \brief The KeyPressFilter class filters arrow key events in a QApplication.
     *
     * The class feeds them directly to the MapWidget whenever it's alive.
     * This could be problematic if additional support for UI keyboard navigation is needed
     * in the future, and should be adjusted to allow keyboard navigation with tab,
     * and only intercept these particular events when MapWidget is focused.
     */
    class KeyPressFilter : public QObject {
    public:
        KeyPressFilter(MapWidget *mapWidget) : mapWidget{ mapWidget } {}
    protected:
        /* This is the function that intercepts the events that go to the QApplication.
         */
        bool eventFilter(QObject*, QEvent*) override;
    private:
        MapWidget *mapWidget = nullptr;
    };

private:
    // Handle to the installed event-filter.
    std::unique_ptr<KeyPressFilter> keyPressFilter = nullptr;

    // Zoom level of viewport.
    // Commonly used range is [-2, 22] but values outside works also.
    double viewportZoomLevel = 0;

    // Center of viewport X (or width).
    // Range [0, 1].
    double x = 0.5;
    // Center of viewport Y (or height).
    // Range [0, 1].
    double y = 0.5;

    // Pass in true if you want to zoom.
    // Applies a single zoom step to the viewport.
    // Used by other public methods.
    void genericZoom(bool magnify);

    // Controls whether debug lines should be shown.
    bool showDebug = false;

    // If set to true, we should be rendering vector graphics.
    // If set to false, we should be rendering raster graphics.
    bool renderVectorTile = true;

    // If true, render fill-elements.
    bool renderFill = true;

    // If true, render line-elements.
    bool renderLines = true;

    // If true, render line-elements.
    bool renderText = true;

public:
    MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    // Application events.
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent *) override;

    /*!
     * \brief mouseStartPosition stores the location of the cursor.
     *
     * {-1, -1} is set as the initial value. The mouse can never reach
     * this location in the window since each axis starts at 0 and then
     * increases in the positive direction.
     *
     * The variable stores the cursor location immediately after it's
     * been pressed.
     */
    QPoint mouseStartPosition = {-1, -1};
    /*!
     * \brief mouseCurrentPosition stores the location of the cursor.
     *
     * {-1, -1} is set as the initial value. The mouse can never reach
     * this location in the window since each axis starts at 0 and then
     * increases in the positive direction.
     *
     * The variable stores the current cursor location while rendering.
     */
    QPoint mouseCurrentPosition = {-1, -1};

    /* Zoom level of viewport. This is a floating number and can be partially zoomed between
     * discrete steps.
     *
     * Range [0, 16]
     */
    double getViewportZoomLevel() const;

    /* Returns the zoom level of the map based on the widget's
     * current viewport configuration.
     *
     * Always an integer in the range [0, 16]
     */
    int getMapZoomLevel() const;

    /* Calculates the set of visible tiles
     * based on the widget's current viewport configuration.
     */
    QVector<TileCoord> calcVisibleTiles() const;

    /* Returns how long the viewport counts as one step
     * during panning.
     *
     * The returned value is a distance in world-normalized coordinates, range [0, 1].
     */
    double getPanStepAmount() const;

    /*! Function signature of the tile request callback.
     *
     * Returns a ScopedPointer containing a RequestTilesResult object.
     * RequestTilesResult object contains map of returned tiles.
     * May have custom logic in destructor.
     * MapWidget should release this object as early as possible.
     *
     * \param First parameter is a set of TileCoordinates to request.
     *
     * \param Second parameter is a callback to signal when a tile is loaded later.
     *        For this widget, it will signal the widget to redraw itself with the new result.
     */
    using RequestTilesFnT =
        QScopedPointer<Bach::RequestTilesResult>(
            const std::set<TileCoord>&,
            std::function<void(TileCoord)>);
    std::function<RequestTilesFnT> requestTilesFn;

    // Handle what should be rendered or not to the viewport.
    bool isShowingDebug() const { return showDebug; }
    bool isRenderingVector() const { return renderVectorTile; }
    bool isRenderingFill() const { return renderFill; }
    void setShouldDrawFill(bool);
    bool isRenderingLines() const { return renderLines; }
    void setShouldDrawLines(bool);
    bool isRenderingText() const { return renderText; }
    void setShouldDrawText(bool);

public slots:
    // Swap between debug and regular mode in the GUI.
    void toggleIsShowingDebug();

    // Swap between the vector and raster tile loading mode.
    void toggleIsRenderingVectorTile();

    // Sets viewport.
    void setViewport(double x, double y, double zoom);

    // Zooming functionality.
    void zoomIn();
    void zoomOut();

    // Map panning functionality.
    void panUp();
    void panDown();
    void panLeft();
    void panRight();
};

#endif // MAPWIDGET_H
