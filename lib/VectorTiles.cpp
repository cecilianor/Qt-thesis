//Qt header files
#include <QProtobufSerializer>

// Other header files
#include "VectorTiles.h"
#include "vector_tile.qpb.h"

/*!
 * \brief PolygonFeature::type
 * \return the type of the feature
 */
AbstractLayerFeature::featureType PolygonFeature::type() const
{
    return AbstractLayerFeature::featureType::polygon;
}

/*!
 * \brief PolygonFeature::polygon
 * getter for the feature's geometry
 * \return the QPainterPath that contains the decoded geometry as a const refrence
 */
QPainterPath const& PolygonFeature::polygon() const
{
    return m_polygon;
}

/*!
 * \brief PolygonFeature::polygon
 * getter for the feature's geometry
 * \return a refrence to the QPainterPath that contains the decoded geometry
 */
QPainterPath& PolygonFeature::polygon() {
    return m_polygon;
}

/*
 * ----------------------------------------------------------------------------
 */

/*!
 * \brief LineFeature::type
 * \return the type of the feature
 */
AbstractLayerFeature::featureType LineFeature::type() const
{
    return AbstractLayerFeature::featureType::line;
}

/*!
 * \brief LineFeature::line
 * getter for the feature's geometry
 * \return the QPainterPath that contains the decoded geometry as a const refrence
 */
QPainterPath const& LineFeature::line() const
{
    return m_line;
}

/*!
 * \brief LineFeature::line
 * getter for the feature's geometry
 * \return a refrence to the QPainterPath that contains the decoded geometry
 */
QPainterPath& LineFeature::line()
{
    return m_line;
}

/*
 * ----------------------------------------------------------------------------
 */

/*!
 * \brief PointFeature::type
 * \return the type of the feature
 */
AbstractLayerFeature::featureType PointFeature::type() const
{
    return AbstractLayerFeature::featureType::point;
}

/*!
 * \brief PointFeature::addPoint
 * getter for the feature's geometry
 * \return a refrence to the QPainterPath that contains the decoded geometry
 */
void PointFeature::addPoint(QPoint point)
{
    m_points.append(point);
}

/*!
 * \brief PointFeature::points
 * getter for the feature's geometry
 * \return the QPainterPath that contains the decoded geometry as a const refrence
 */
QList<QPoint> PointFeature::points() const
{
    return m_points;
}
/*
 * ----------------------------------------------------------------------------
 */

/*!
 * \brief TileLayer::TileLayer
 * class constructor.
 * \param version major version number of the tile
 * \param name name of the tile, used as the ID
 * \param extent the dimentions for the coordinate system used in the tile geometry
 */
TileLayer::TileLayer(int version, QString name, int extent)
    : m_version(version),
    m_name(name),
    m_extent(extent) {
}

/*!
 * \brief TileLayer::version
 * Getter for the tile version
 * \return the tile version
 */
int TileLayer::version() const
{
    return m_version;
}

/*!
 * \brief TileLayer::name
 * Getter for the tile name
 * \return the tile name
 */
QString TileLayer::name() const
{
    return m_name;
}

/*!
 * \brief TileLayer::extent
 * Getter for the tile extent
 * \return the tileExtent
 */
int TileLayer::extent() const
{
    return m_extent;
}

/*
 * ----------------------------------------------------------------------------
 */

/*!
 * \brief polygonFeatureFromProto
 * Decode the geometry of a layer's polygon feature from the desrialized protocol buffer.
 * \param feature a refrence to the layer's feature from the deserialized protocol buffer
 * \return a pointer of type PolygonFeature conatining the decoded geometry as a QPainterPath.
 */
std::unique_ptr<AbstractLayerFeature> polygonFeatureFromProto(
    const vector_tile::Tile::Feature &feature)
{
    PolygonFeature *newFeature = new PolygonFeature;
    auto featurePtr = std::unique_ptr<AbstractLayerFeature>(newFeature);

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
    return featurePtr;
}


/*!
 * \brief lineFeatureFromProto
 *  Decode the geometry of a layer's line feature from the desrialized protocol buffer.
 * \param feature a refrence to the layer's feature from the deserialized protocol buffer
 * \return a pointer of type LineFeature conatining the decoded geometry as a QPainterPath.
 */
std::unique_ptr<AbstractLayerFeature> lineFeatureFromProto(
    const vector_tile::Tile::Feature &feature)
{
    LineFeature *newFeature = new LineFeature;
    auto featurePtr = std::unique_ptr<AbstractLayerFeature>(newFeature);
    qint32 x = 0;
    qint32 y = 0;
    QPainterPath path;
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
                path.moveTo(x, y);
            } else if (commandId == 2) {
                path.lineTo(x, y);
            }
            count--;
        }
    }
    newFeature->line() = path;
    return featurePtr;
}

/*!
 * \brief textLineFeatureFromProto
 *  Decode the geometry of a layer's line feature from the desrialized protocol buffer.
 *  This function deals with decoding line geometry for text features. It only includes the
 *  line with the largest length in the QPainterPath.
 * \param feature a refrence to the layer's feature from the deserialized protocol buffer
 * \return a pointer of type LineFeature conatining the decoded geometry as a QPainterPath.
 */
std::unique_ptr<AbstractLayerFeature> textLineFeatureFromProto(
    const vector_tile::Tile::Feature &feature)
{
    LineFeature *newFeature = new LineFeature;
    auto featurePtr = std::unique_ptr<AbstractLayerFeature>(newFeature);

    qint32 x = 0;
    qint32 y = 0;
    QPainterPath path;
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
                if(path.length() > newFeature->line().length()){
                    newFeature->line() = path;
                    path.clear();
                }
                path.moveTo(x, y);
            } else if (commandId == 2) {
                path.lineTo(x, y);
            }
            count--;
        }
    }
    return featurePtr;
}

/*!
 * \brief pointFeatureFromProto
 * Decode the geometry of a layer's point feature from the desrialized protocol buffer.
 * \param feature a refrence to the layer's feature from the deserialized protocol buffer.
 * \return a pointer of type PointFeature conatining the decoded geometry as a QList<QPoint>.
 */
std::unique_ptr<AbstractLayerFeature> pointFeatureFromProto(
    const vector_tile::Tile::Feature &feature)
{
    PointFeature *newFeature = new PointFeature;
    auto featurePtr = std::unique_ptr<AbstractLayerFeature>(newFeature);

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
    return featurePtr;
}


/*!
 * \brief populateFeatureMetaData
 * Extracts the feature's metadata from the layers keys and values lists
 * \param feature a pointer to the feature whose metadata is to be extracted.
 * \param keys a list of the keys in the encoded feature metadata
 * \param values a list of values that's used to decode the feature's keys list
 */
void populateFeatureMetaData(
    AbstractLayerFeature *feature,
    const QList<QString> &keys,
    const QList<vector_tile::Tile_QtProtobufNested::Value> &values)
{
    //The feature's keys list is the features metadata encoded using the tile's values list.
    //The kayes list length is always expected to be even.
    //the nth element in the keys list corresponds to the (n/2)th element in the metadata map after decoding,
    // and the n+1 th element in the keys list correspond to the value for the (n/2) element in the metadata map after decoding.
    //The keys elements' values map to the tile values list's indecies.
    if (!feature || feature->tags.length() < 2)
        return;

    for(int i = 0; i <= feature->tags.length() - 2; i += 2){
        int keyIndex = feature->tags.at(i);
        int valueIndex = feature->tags.at(i + 1);
        QString key = keys.at(keyIndex);

        const vector_tile::Tile_QtProtobufNested::Value &value = values.at(valueIndex);

        if (value.hasStringValue()) {
            feature->featureMetaData.insert(key, QVariant(value.stringValue()));
        } else if (value.hasFloatValue()) {
            feature->featureMetaData.insert(key, QVariant(value.floatValue()));
        } else if (value.hasDoubleValue()) {
            feature->featureMetaData.insert(key, QVariant(value.doubleValue()));
        } else if (value.hasIntValue()) {
            feature->featureMetaData.insert(key, QVariant::fromValue<QtProtobuf::int64>(value.intValue()));
        } else if (value.hasUintValue()) {
            feature->featureMetaData.insert(key, QVariant::fromValue<QtProtobuf::uint64>(value.uintValue()));
        } else if (value.hasSintValue()) {
            feature->featureMetaData.insert(key, QVariant::fromValue<QtProtobuf::sint64>(value.sintValue()));
        } else if (value.hasBoolValue()) {
            feature->featureMetaData.insert(key, QVariant(value.boolValue()));
        }
    }
}

/*
 * ----------------------------------------------------------------------------
 */

VectorTile::VectorTile() {
}


std::optional<VectorTile> VectorTile::fromByteArray(const QByteArray &bytes)
{
    return Bach::tileFromByteArray(bytes);
}

std::optional<VectorTile> VectorTile::fromFile(const QString &path)
{
    QFile file{ path };
    bool openSuccess = file.open(QFile::ReadOnly);
    if (!openSuccess) {
        return std::nullopt;
    }
    return fromByteArray(file.readAll());
}

/*!
 * \brief VectorTile::fromByteArray
 * Deserialize and extracts all the layers in the tile protocol buffer,
 * then iterates through each layer's features and
 * calls the apropriate function to decode the feature's geometry and metadata.
 * \param data a QByteArray containing the raw protocol buffer.
 * \return true if the tile was succesfully decoded, or false otherwise
 */
std::optional<VectorTile> Bach::tileFromByteArray(const QByteArray &bytes)
{
    QProtobufSerializer serializer;

    vector_tile::Tile tile;
    tile.deserialize(&serializer, bytes);

    if (serializer.deserializationError()!= QAbstractProtobufSerializer::NoError) {
        return std::nullopt;
    }

    VectorTile output;

    for (const auto &layer : tile.layers()) {
        std::unique_ptr<TileLayer> newLayerPtr = std::make_unique<TileLayer>(
            layer.version(),
            layer.name(),
            layer.extent());

        TileLayer *newLayer = newLayerPtr.get();

        output.m_layers.insert({layer.name(), std::move(newLayerPtr)});

        QList<QString> layerKeys = layer.keys().toList();
        QList layerValues = layer.values().toList();
        for(const auto &feature : layer.features()) {
            switch (feature.type()) {
            case vector_tile::Tile::GeomType::POLYGON:
                {
                    std::unique_ptr<AbstractLayerFeature> newFeaturePtr = polygonFeatureFromProto(feature);
                    AbstractLayerFeature *newFeature = newFeaturePtr.get();
                    newFeature->tags = feature.tags().toList();
                    populateFeatureMetaData(newFeature, layerKeys, layerValues);
                    newLayer->m_features.push_back(std::move(newFeaturePtr));
                }
                break;
            case vector_tile::Tile::GeomType::LINESTRING:
                {
                    std::unique_ptr<AbstractLayerFeature> newFeaturePtr;
                    if (newLayer->name() == "transportation_name") {
                        newFeaturePtr= textLineFeatureFromProto(feature);
                    } else {
                        newFeaturePtr = lineFeatureFromProto(feature);
                    }
                    AbstractLayerFeature *newFeature = newFeaturePtr.get();
                    newFeature->tags = feature.tags().toList();
                    populateFeatureMetaData(newFeature, layerKeys, layerValues);
                    newLayer->m_features.push_back(std::move(newFeaturePtr));
                }
                break;
            case vector_tile::Tile::GeomType::POINT:
                {
                    std::unique_ptr<AbstractLayerFeature> newFeaturePtr = pointFeatureFromProto(feature);
                    AbstractLayerFeature *newFeature = newFeaturePtr.get();
                    newFeature->tags = feature.tags().toList();
                    populateFeatureMetaData(newFeature, layerKeys, layerValues);
                    newLayer->m_features.push_back(std::move(newFeaturePtr));
                }
                break;
            case vector_tile::Tile::GeomType::UNKNOWN:
                break;
            }
        }
    }
    return output;
}























