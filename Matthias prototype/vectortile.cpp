#include "vectortile.h"

#include <QProtobufSerializer>

#include "vector_tile.qpb.h"

/*
 *
 *
 *
 * All of this follows
 *
 * https://github.com/mapbox/vector-tile-spec/tree/master/2.1
 *
 *
 *
 *
 */

VectorLayer::VectorLayer()
{

}

VectorLayer::~VectorLayer()
{
    for (auto feature : m_features) {
        delete feature;
    }
}

QRect VectorLayer::boundingRect() const {
    QRect rect;
    for (const auto &feature : m_features) {
        //qDebug() << "    " << rect;
        if (feature->type() == VectorFeature::featureType::polygon ||
            feature->type() == VectorFeature::featureType::line )
            rect = rect.united(feature->boundingRect());
    }
    return rect;
}


VectorTile::VectorTile()
{

}

VectorTile::~VectorTile()
{
    for (auto key : m_layer.keys()) {
        delete m_layer[key];
    }
}

PolygonFeature* polygonFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    qDebug() << "    create PolygonFeature";
    PolygonFeature *newFeature = new PolygonFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 commandId = point & 0x7;
        quint32 count = point >> 3;
        i++;

        if (commandId == 7) {
            newFeature->m_polygon.closeSubpath();
            continue;
        }
        //qDebug() << "        command:" << commandId << count;

        while(count > 0 && i < feature.geometry().size() - 1) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            if (commandId == 1) {
                newFeature->m_polygon.moveTo(x, y);
            } else if (commandId == 2) {
                newFeature->m_polygon.lineTo(x, y);
            }
            //qDebug() << "            add point" << x << y << "count =" << count;
            count--;
        }
    }

    return newFeature;
}

LineFeature* lineFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    qDebug() << "    create LineFeature";
    LineFeature *newFeature = new LineFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 commandId = point & 0x7;
        quint32 count = point >> 3;
        i++;

        //qDebug() << "        command:" << commandId << count;

        while(count > 0 && i < feature.geometry().size() - 1) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            if (commandId == 1) {
                newFeature->m_line.moveTo(x, y);
            } else if (commandId == 2) {
                newFeature->m_line.lineTo(x, y);
            }
            //qDebug() << "            add point" << x << y << "count =" << count;
            count--;
        }
    }

    return newFeature;
}

PointFeature* pointFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    qDebug() << "    create PointFeature";
    PointFeature *newFeature = new PointFeature();

    qint32 x = 0;
    qint32 y = 0;

    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        quint32 commandId = point & 0x7;
        quint32 count = point >> 3;
        i++;

        //qDebug() << "        command:" << commandId << count;

        while(count > 0) {
            point = feature.geometry().at(i);
            x += ((point >> 1) ^ (-(point & 1)));
            i++;
            point = feature.geometry().at(i);
            y += ((point >> 1) ^ (-(point & 1)));
            i++;
            newFeature->m_points.append(QPoint(x, y));
            //qDebug() << "            add point" << x << y;
            count--;
        }
    }
    return newFeature;
}

UnknownFeature* unknownFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    qDebug() << "    create unknown feature";
    UnknownFeature *newFeature = new UnknownFeature();

    return newFeature;
}

void VectorTile::load(QByteArray data)
{
    QProtobufSerializer serializer;

    vector_tile::Tile tile;
    tile.deserialize(&serializer, data);

    if (serializer.deserializationError()!= QAbstractProtobufSerializer::NoError) {
        qWarning().nospace() << "Unable to deserialize tile ("
                   << serializer.deserializationError() << ") "
                   << serializer.deserializationErrorString();
        return;
    }

    for (auto layer : tile.layers()) {
        if (layer.hasName())
            qDebug() << "Parsing layer" << layer.name();
        //if (layer.hasExtent())
        //    qDebug() << "    extent:" << layer.extent();
        //if (layer.hasVersion())
        //    qDebug() << "    version:" << layer.version();
        //qDebug() << "    number of features:" << layer.features().size();

        if (!layer.hasName())
            continue;

        VectorLayer *newLayer = new VectorLayer();
        m_layer.insert(QString::fromStdString(layer.name().toStdString()), newLayer);

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
                    newLayer->m_features.append(unknownFeatureFromProto(feature));
                    break;
            }
        }
    }
}

QRect VectorTile::boundingRect() const
{
    QRect rect;
    for (const auto &layer : m_layer) {
        //qDebug() << rect;
        rect = rect.united(layer->boundingRect());
    }
    return rect;
}
