#include "VectorTiles.h"
#include "vector_tile.qpb.h"
#include <QProtobufSerializer>

/*
 *
 * all the functions follow the mepbox vector tile specification:  https://github.com/mapbox/vector-tile-spec/tree/master/2.1
 *
 *
 */

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

QPainterPath const& LineFeature::line() const
{
    return m_line;
}

QPainterPath& LineFeature::line()
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

/* Decode the geometry of a layer's polygon feature from the desrialized protocol buffer.
     *
     * Parameters:
     *      feature expects a refrence to the layer's feature from the deserialized protocol buffer
     *
     * Returns a pointer of type PolygonFeature conatininf the decoded geometry as a QPainterPath.
     */
PolygonFeature* polygonFeatureFromProto(const vector_tile::Tile::Feature &feature)
{
    PolygonFeature *newFeature = new PolygonFeature();

    qint32 x = 0;
    qint32 y = 0;
    //iterate through the geometry commands and parameters.
    for(int i = 0; i < feature.geometry().size(); ) {
        quint32 point = feature.geometry().at(i);
        //the command type is encoded as the 3 LSBs of the command integer.
        //With: command 1 = MoveTo; command 2 = LineTo; command 7 = ClosePAth (takes not parameters);
        quint32 commandId = point & 0x7;
        //the command count is encoded as the remaining 29 bits, and represents how many
        //times the command should be repeated (for command 1 and 2 the number of arguments
        //should be equal to 2 * commandCount).
        quint32 count = point >> 3;
        i++;
        if (commandId == 7) {
            newFeature->polygon().closeSubpath();
            continue;
        }
        while(count > 0 && i < feature.geometry().size() - 1) {
            point = feature.geometry().at(i);
             //this is the formula for decoding the command parameters.
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

/* Decode the geometry of a layer's line feature from the desrialized protocol buffer.
     *
     * Parameters:
     *      feature expects a refrence to the layer's feature from the deserialized protocol buffer
     *
     * Returns a pointer of type LineFeature conatininf the decoded geometry as a QPainterPath.
     */
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

/* Decode the geometry of a layer's point feature from the desrialized protocol buffer.
     *
     * Parameters:
     *      feature expects a refrence to the layer's feature from the deserialized protocol buffer
     *
     * Returns a pointer of type PointFeature conatininf the decoded geometry as a QList<QPoint>.
     */
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

/*Extracts the feature's metadata from the layers keys and values lists
 *
*/
void populateFeatureMetaData(AbstractLayerFeature* feature, QList<QString> &keys, QList<vector_tile::Tile_QtProtobufNested::Value> &values)
{

    if(!feature || feature->tags.length() < 2) return;
    for(int i = 0; i <= feature->tags.length() - 2; i += 2){
        int keyIndex = feature->tags.at(i);
        int valueIndex = feature->tags.at(i + 1);
        QString key = keys.at(keyIndex);
        auto value = values.at(valueIndex);
        if(value.hasStringValue()){
            feature->fetureMetaData.insert(key, QVariant(value.stringValue()));
        }else if(value.hasFloatValue()){
            feature->fetureMetaData.insert(key, QVariant(value.floatValue()));
        }else if(value.hasDoubleValue()){
            feature->fetureMetaData.insert(key, QVariant(value.doubleValue()));
        }else if(value.hasIntValue()){
            feature->fetureMetaData.insert(key, QVariant::fromValue<QtProtobuf::int64>(value.intValue()));
        }else if(value.hasUintValue()){
            feature->fetureMetaData.insert(key, QVariant::fromValue<QtProtobuf::uint64>(value.uintValue()));
        }else if(value.hasSintValue()){
            feature->fetureMetaData.insert(key, QVariant::fromValue<QtProtobuf::sint64>(value.sintValue()));
        }else if(value.hasBoolValue()){
            feature->fetureMetaData.insert(key, QVariant(value.boolValue()));
        }
    }
}

/*
 * ----------------------------------------------------------------------------
 */

VectorTile::VectorTile() {

}

VectorTile::~VectorTile() {
}

/* Deserialize and extracts all the layers in the tile protocol buffer,
     * then iterates therough each layer's features and
     * calls the apropriate function to decode the feature's geometry.
     *
     * Parameters:
     *      data expects a QByteArray containing the raw protocol buffer.
     *
     */
bool VectorTile::DeserializeMessage(QByteArray data)
{
    QProtobufSerializer serializer;

    vector_tile::Tile tile;
    tile.deserialize(&serializer, data);

    if (serializer.deserializationError()!= QAbstractProtobufSerializer::NoError) {
        qWarning().nospace() << "Unable to deserialize tile: "
                             << serializer.deserializationErrorString();
        return false;
    }

    //iterate throught the layer's features and call the apropriate decoding function on the feature.
    for (auto layer : tile.layers()) {
        //qDebug() << "Parsing layer" << layer.name();
        //qDebug() << " layer version: " << layer.version();
        //qDebug() << " layer extent: " << layer.extent();
        TileLayer *newLayer = new TileLayer(layer.version(), layer.name(), layer.extent());
        m_layers.insert(layer.name(), newLayer);
        AbstractLayerFeature* newFeature = nullptr;
        QList<QString> layerKeys = layer.keys().toList();
        auto lyerValues = layer.values().toList();
        for(const auto &feature : layer.features()) {
            switch (feature.type()) {
            case vector_tile::Tile::GeomType::POLYGON:
                newFeature = polygonFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::LINESTRING:
                newFeature = lineFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::POINT:
                newFeature = pointFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::UNKNOWN:
                break;
            }
        }
    }

    return true;
}

std::optional<VectorTile> Bach::tileFromByteArray(const QByteArray &bytes)
{
    QProtobufSerializer serializer;

    vector_tile::Tile tile;
    tile.deserialize(&serializer, bytes);

    if (serializer.deserializationError()!= QAbstractProtobufSerializer::NoError) {
        /*
        qError().nospace() << "Unable to deserialize tile: "
                             << serializer.deserializationErrorString();
        */
        return std::nullopt;
    }

    VectorTile output;

    for (auto layer : tile.layers()) {
        //qDebug() << "Parsing layer" << layer.name();
        //qDebug() << " layer version: " << layer.version();
        //qDebug() << " layer extent: " << layer.extent();
        TileLayer *newLayer = new TileLayer(layer.version(), layer.name(), layer.extent());
        output.m_layers.insert(layer.name(), newLayer);
        AbstractLayerFeature* newFeature = nullptr;
        QList<QString> layerKeys = layer.keys().toList();
        auto lyerValues = layer.values().toList();
        for(const auto &feature : layer.features()) {
            switch (feature.type()) {
            case vector_tile::Tile::GeomType::POLYGON:
                newFeature = polygonFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::LINESTRING:
                newFeature = lineFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::POINT:
                newFeature = pointFeatureFromProto(feature);
                newFeature->tags = feature.tags().toList();
                populateFeatureMetaData(newFeature, layerKeys, lyerValues);
                newLayer->m_features.append(newFeature);
                break;
            case vector_tile::Tile::GeomType::UNKNOWN:
                break;
            }
        }
    }

    return output;
}























