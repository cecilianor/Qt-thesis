#include "Rendering.h"
#include "Evaluator.h"

/*!
 * \brief getFillColor
 * Get the QVariant of the color from the layerStyle and resolve and return it if its an expression,
 * or return it as a QColor otherwise. This function also gets the opacity of the polygon.
 * \param layerStyle the layerStyle containing the color variable
 * \param feature the feature to be used in case the QVariant is an expression
 * \param mapZoom the map zoom level to be used in case the QVariant is an expression
 * \param vpZoom the viewport zoom level to be used in case the QVariant is an expression
 * \return the QColor to be used to render the polygon
 */
static QColor getFillColor(
    const FillLayerStyle &layerStyle,
    const AbstractLayerFeature &feature,
    int mapZoom,
    double vpZoom)
{
    QVariant colorVariant = layerStyle.getFillColorAtZoom(mapZoom);
    QColor color;
    // The layer style might return an expression, we need to resolve it.
    if(colorVariant.typeId() == QMetaType::Type::QJsonArray){
        color = Evaluator::resolveExpression(
                    colorVariant.toJsonArray(),
                    &feature,
                    mapZoom,
                    vpZoom).value<QColor>();
    } else {
        color = colorVariant.value<QColor>();
    }

    QVariant fillOpacityVariant = layerStyle.getFillOpacityAtZoom(mapZoom);
    float fillOpacity;
    // The layer style might return an expression, we need to resolve it.
    if (fillOpacityVariant.typeId() == QMetaType::Type::QJsonArray){
        fillOpacity = Evaluator::resolveExpression(
                          fillOpacityVariant.toJsonArray(),
                          &feature,
                          mapZoom,
                          vpZoom).value<float>();
    } else {
        fillOpacity = fillOpacityVariant.value<float>();
    }

    color.setAlphaF(fillOpacity * color.alphaF());
    return color;
}


/*!
 * \brief Bach::paintSingleTileFeature_Polygon
 * This function reders a single polygon feature.
 * \param details the struct containig all the elemets needed to paint the feature includeing the layerStyle and the feature itself.
 */
void Bach::paintSingleTileFeature_Polygon(Bach::PaintingDetailsPolygon details)
{
    const FillLayerStyle &layerStyle = *details.layerStyle;
    const PolygonFeature &feature = *details.feature;
    QColor brushColor = getFillColor(layerStyle, feature, details.mapZoom, details.vpZoom);

    QPainter &painter = *details.painter;
    painter.setBrush(brushColor);
    painter.setRenderHints(QPainter::Antialiasing, layerStyle.m_antialias);
    painter.setPen(Qt::NoPen);

    const QPainterPath &path = feature.polygon();

    QTransform transform = details.transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    const QPainterPath newPath = transform.map(path);

    painter.drawPath(newPath);
}

