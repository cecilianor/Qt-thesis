#include "tilerenderer.h"

TileRenderer::TileRenderer()
{

}

void TileRenderer::render(QPainter *p, const VectorTile &tile)
{
    for (const auto layer : m_rules.layer()) {

        if (layer->type() == TileLayerStyle::layerType::background) {
            BackGroundLayerStyle *style = reinterpret_cast<BackGroundLayerStyle*>(layer);
            p->fillRect(QRect(0, 0, 4096, 4096), style->backgroundColor(m_zoomLevel));
        }

        if (layer->type() == TileLayerStyle::layerType::fill) {
            FillLayerStyle *style = reinterpret_cast<FillLayerStyle*>(layer);
            p->setBrush(style->fillColor(m_zoomLevel));
            p->setPen(Qt::NoPen);
            const QString &sourceLayer = layer->sourceLayer();
            if (tile.m_layer.contains(layer->sourceLayer())) {
                VectorLayer *data = tile.m_layer[layer->sourceLayer()];
                for (auto feature : data->m_features) {
                    if (feature->type() == VectorFeature::featureType::polygon) {
                        PolygonFeature *polyFeature = reinterpret_cast<PolygonFeature*>(feature);
                        p->drawPath(polyFeature->m_polygon);
                    } else if (feature->type() == VectorFeature::featureType::line) {
                        LineFeature *lineFeature = reinterpret_cast<LineFeature*>(feature);
                        p->drawPath(lineFeature->m_line);
                    } else {
                        qDebug() << "missed feature in fillLayer";
                    }
                }
            }
        }

        if (layer->type() == TileLayerStyle::layerType::line) {
            LineLayerStyle *style = reinterpret_cast<LineLayerStyle*>(layer);
            p->setPen(QPen(style->lineColor(m_zoomLevel), style->lineWidth(m_zoomLevel) * 4096/512));
            p->setBrush(Qt::NoBrush);
            const QString &sourceLayer = layer->sourceLayer();
            if (tile.m_layer.contains(layer->sourceLayer())) {
                VectorLayer *data = tile.m_layer[layer->sourceLayer()];
                for (auto feature : data->m_features) {
                    if (feature->type() == VectorFeature::featureType::polygon) {
                        PolygonFeature *polyFeature = reinterpret_cast<PolygonFeature*>(feature);
                        p->drawPath(polyFeature->m_polygon);
                    } else if (feature->type() == VectorFeature::featureType::line) {
                        LineFeature *lineFeature = reinterpret_cast<LineFeature*>(feature);
                        p->drawPath(lineFeature->m_line);
                    } else {
                        qDebug() << "missed feature in lineLayer";
                    }
                }
            }
        }

    }
}
