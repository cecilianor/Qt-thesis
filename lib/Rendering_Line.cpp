#include "Rendering.h"
#include "Evaluator.h"


/*!
 * \brief getLineColor
 * Get the QVariant of the color from the layerStyle and resolve and return it if its an expression,
 * or return it as a QColor otherwise
 * \param layerStyle the layerStyle containing the color variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return the color to be used to render the line
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

/*!
 * \brief getLineOpacity
 * Get the QVariant of the opcity from the layerStyle and resolve and return it if its an expression,
 * or return it as a float otherwise
 * \param layerStyle the layerStyle containing the opacity variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return the opacity to be used to render the line
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

/*!
 * \brief getLineWidth
 * Get the QVariant of the line width from the layerStyle and resolve and return it if its an expression,
 * or return it as a float otherwise
 * \param layerStyle the layerStyle containing the line width variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return the line width value to be used to render the line
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
/*!
 * \brief Bach::paintSingleTileFeature_Line
 * This function reders a single line feature.
 * \param details the struct containig all the elemets needed to paint the feature includeing the layerStyle and the feature itself.
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

