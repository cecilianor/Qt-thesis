#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPainter>

#include "VectorTiles.h"
#include "TileCoord.h"
#include "Layerstyle.h"

/* Widget class responsible for displaying the actual map.
 * Should be used as a smaller widget within a larger Widget hierarchy.
 *
 * This MapWidget has a built-in viewport configuration (zoom level and center coordinates).
 */
class MapWidget : public QWidget
{
    Q_OBJECT

public:
    MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

    /* Zoom level of viewport. This is a floating number and can be partially zoomed between
     * discrete steps.
     *
     * Range [0, 16]
     */
    double getViewportZoomLevel() const;

    // Guaranteed to be an exact integer.
    int getMapZoomLevel() const;

    QVector<TileCoord> CalcVisibleTiles() const;

    // Temporary. This probably does not belong here.
    QMap<TileCoord, const VectorTile*> tileStorage;
    // Temporary. This probably does not belong here.
    StyleSheet styleSheet;

private:
    // Zoom level of viewport
    // Range [0, 16]
    // This should never go below 0 in final release, but
    // is useful for testing.
    double viewportZoomLevel = 0;

    // Default should be 0 for final release.
    // This offsets the map zoom compared to viewport zoom.
    double mapZoomLevelOffset = 0;

    // Mostly useful for debugging.
    // Allows you to lock the zoom level of the tiles
    // that are displayed.
    bool overrideMapZoom = false;
    int overrideMapZoomLevel = 0;

    // Center of viewport X
    // Range [0, 1]
    double x = 0.5;
    // Center of viewport Y
    // Range [0, 1]
    double y = 0.5;
};

#endif // MAPWIDGET_H
