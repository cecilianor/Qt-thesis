#include "Rendering.h"
#include "Evaluator.h"

/* Finds the fill color of given feature at the given zoom level.
 *
 * This function also takes opacity into account.
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

/* Render a single feature of a tile, where the feature is of type polygon.
 *
 * Assumes the painter's position has been moved to the origin of the tile.
 *
 */
void Bach::paintSingleTileFeature_Fill_Polygon(
    QPainter &painter,
    const PolygonFeature &feature,
    const FillLayerStyle &layerStyle,
    const int mapZoom,
    const double vpZoom,
    const QTransform &transformIn)
{
    auto brushColor = getFillColor(layerStyle, feature, mapZoom, vpZoom);

    painter.setBrush(brushColor);
    painter.setRenderHints(QPainter::Antialiasing, layerStyle.m_antialias);
    painter.setPen(Qt::NoPen);

    auto const& path = feature.polygon();

    QTransform transform = transformIn;
    transform.scale(1 / 4096.0, 1 / 4096.0);
    auto newPath = transform.map(path);

    painter.drawPath(newPath);
}
