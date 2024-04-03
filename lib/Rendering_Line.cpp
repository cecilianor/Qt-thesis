#include "Rendering.h"
#include "Evaluator.h"

/* Finds the line color of given feature at the given zoom level.
 */
static QColor getLineColor(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant color = layerStyle.getLineColorAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(color.typeId() == QMetaType::Type::QJsonArray){
        color = Evaluator::resolveExpression(
            color.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return color.value<QColor>();
}

/* Finds the line opacity of given feature at the given zoom level.
 */
static float getLineOpacity(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant lineOpacity = layerStyle.getLineOpacityAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(lineOpacity.typeId() == QMetaType::Type::QJsonArray){
        lineOpacity = Evaluator::resolveExpression(
            lineOpacity.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return lineOpacity.value<float>();
}

/* Finds the line width of this feature at given zoom level.
 */
static int getLineWidth(
    const LineLayerStyle &layerStyle,
    const LineFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant lineWidth = layerStyle.getLineWidthAtZoom(mapZoom);
    // The layer style might return an expression, we need to resolve it.
    if(lineWidth.typeId() == QMetaType::Type::QJsonArray){
        lineWidth = Evaluator::resolveExpression(
            lineWidth.toJsonArray(),
            &feature,
            mapZoom,
            vpZoom);
    }
    return lineWidth.value<int>();
}

/* Paints a single Line feature within a tile.
 *
 * Assumes the painters origin has moved to the tiles origin.
 */
void Bach::paintSingleTileFeature_Line(Bach::PaintingDetailsLine details)
{
    QPainter &painter = *details.painter;
    const LineFeature &feature = *details.feature;
    const LineLayerStyle &layerStyle = *details.layerStyle;
    QPen pen = painter.pen();

    pen.setColor(getLineColor(layerStyle, feature, details.mapZoom, details.vpZoom));
    // Not sure how to take opacity into account yet.
    // There's some interaction happening with the color.
    //painter.setOpacity(getLineOpacity(layerStyle, feature, mapZoom, vpZoom));

    pen.setWidth(getLineWidth(layerStyle, feature, details.mapZoom, details.vpZoom));
    pen.setCapStyle(layerStyle.getCapStyle());
    pen.setJoinStyle(layerStyle.getJoinStyle());

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // Not sure yet how to determine AA for lines.
    painter.setRenderHints(QPainter::Antialiasing, false);

    const QPainterPath &path = feature.line();
    QTransform transform = details.transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    const QPainterPath newPath = transform.map(path);

    painter.drawPath(newPath);
}

