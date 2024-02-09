#include "VectorTiles.h"
#include "vector_tile.qpb.h"
#include <QProtobufSerializer>

AbstractLayerFeature::featureType PolygonFeature::type() const
{
    return AbstractLayerFeature::featureType::polygon;
}

QPainterPath const& PolygonFeature::polygon() const
{
    return m_polygon;
}

QPainterPath& PolygonFeature::polygon() {
    return m_polygon;
}

/*
 * ----------------------------------------------------------------------------
 */
AbstractLayerFeature::featureType LineFeature::type() const
{
    return AbstractLayerFeature::featureType::line;
}

QPainterPath LineFeature::line() const
{
    return m_line;
}
/*
 * ----------------------------------------------------------------------------
 */
AbstractLayerFeature::featureType PointFeature::type() const
{
    return AbstractLayerFeature::featureType::point;
}

void PointFeature::addPoint(QPoint point)
{
    m_points.append(point);
}

QList<QPoint> PointFeature::points() const
{
    return m_points;
}
/*
 * ----------------------------------------------------------------------------
 */
TileLayer::TileLayer(int version, QString name, int extent)
    : m_version(version),
    m_name(name),
    m_extent(extent) {

}

TileLayer::~TileLayer() {
    for (auto feature : m_features) {
        delete feature;
    }
}

int TileLayer::version() const
{
    return m_version;
}

QString TileLayer::name() const
{
    return m_name;
}

int TileLayer::extent() const
{
    return m_extent;
}

/*
 * ----------------------------------------------------------------------------
 */


PolygonFeature* polygonFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    PolygonFeature *newFeature = new PolygonFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 commandId = point & 0x7;
        quint32 count = point >> 3;
        i++;
        if (commandId == 7) {
            newFeature->polygon().closeSubpath();
            continue;
        }
        while(count > 0 && i < feature.geometry().size() - 1) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            if (commandId == 1) {
                newFeature->polygon().moveTo(x, y);
            } else if (commandId == 2) {
                newFeature->polygon().lineTo(x, y);
            }
            count--;
        }
    }

    return newFeature;
}


LineFeature* lineFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    LineFeature *newFeature = new LineFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 commandId = point & 0x7;
        quint32 count = point >> 3;
        i++;

        while(count > 0 && i < feature.geometry().size() - 1) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            if (commandId == 1) {
                newFeature->line().moveTo(x, y);
            } else if (commandId == 2) {
                newFeature->line().lineTo(x, y);
            }
            count--;
        }
    }

    return newFeature;
}


PointFeature* pointFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    PointFeature *newFeature = new PointFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 count = point >> 3;
        i++;
        while(count > 0) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            newFeature->addPoint(QPoint(x, y));
            count--;
        }
    }
    return newFeature;
}

/*
 * ----------------------------------------------------------------------------
 */

VectorTile::VectorTile() {

}

VectorTile::~VectorTile() {
    for (QString key : m_layers.keys()) {
        delete m_layers[key];
    }
}

void VectorTile::DeserializeMessage(QByteArray data)
{
    QProtobufSerializer serializer;

    vector_tile::Tile tile;
    tile.deserialize(&serializer, data);

    if (serializer.deserializationError()!= QAbstractProtobufSerializer::NoError) {
        qWarning().nospace() << "Unable to deserialize tile: "
                             << serializer.deserializationErrorString();
        return;
    }

    for (auto layer : tile.layers()) {
        qDebug() << "Parsing layer" << layer.name();
        qDebug() << " layer version: " << layer.version();
        qDebug() << " layer extent: " << layer.extent();
        TileLayer *newLayer = new TileLayer(layer.version(), layer.name(), layer.extent());
        m_layers.insert(QString::fromStdString(layer.name().toStdString()), newLayer);

        for(const auto &feature : layer.features()) {
            switch (feature.type()) {
            case vector_tile::Tile::GeomType::POLYGON:
                newLayer->m_features.append(polygonFeatureFromProto(feature));
                break;
            case vector_tile::Tile::GeomType::LINESTRING:
                newLayer->m_features.append(lineFeatureFromProto(feature));
                break;
            case vector_tile::Tile::GeomType::POINT:
                newLayer->m_features.append(pointFeatureFromProto(feature));
                break;
            case vector_tile::Tile::GeomType::UNKNOWN:
                break;
            }
        }
    }
}

























