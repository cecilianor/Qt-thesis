#ifndef VECTORTILES_H
#define VECTORTILES_H

//Qt header files
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QMap>
#include <QPainterPath>
#include <QRect>

// STL header files
#include <map>      // For std::map
#include <optional> // For std::optional
#include <memory>   // For std::unique_ptr

/*
 * This abstract class is the base for all the classes representing different layer features.
 */
class AbstractLayerFeature {

public:
    AbstractLayerFeature() {}
    virtual ~AbstractLayerFeature() {};

    enum class featureType : int8_t {
        polygon,
        line,
        point,
        unknown
    };

    virtual featureType type() const = 0;
    QVector<unsigned int> tags;
    QMap<QString, QVariant> featureMetaData;
private:
    int m_id;
};

/*
 * This class represents a plygon feature. the class contains the id and geometry of the feature.
 */
class PolygonFeature : public AbstractLayerFeature
{
public:
    PolygonFeature() {}
    AbstractLayerFeature::featureType type() const override;
    QPainterPath const& polygon() const;
    QPainterPath& polygon();

private:
    QPainterPath m_polygon;
};

/*
 * This class represents a linsestring feature. the class contains the id and geometry of the feature.
 */
class LineFeature : public AbstractLayerFeature
{
public:
    LineFeature(){}
    AbstractLayerFeature::featureType type() const override;
    QPainterPath const& line() const;
    QPainterPath& line();

private:
    QPainterPath m_line;
};

/*
 * This class represents a point feature. the class contains the id and geometry of the feature.
 */
class PointFeature : public AbstractLayerFeature
{
public:
    PointFeature(){}
    AbstractLayerFeature::featureType type() const override;
    void addPoint(QPoint point);
    QList<QPoint> points() const;

private:
    QList<QPoint> m_points;
};

/*
 * This class represents an unkow feature.
 * these features will not be further processed.
 */
class UnknownFeature : public AbstractLayerFeature
{
public:
    UnknownFeature(){}
    AbstractLayerFeature::featureType type() const override;
};

/*
 *This class represents a single layer in a vector tile.
 *the class contains a list with all the features in the layer as well as other layer details.
 */
class TileLayer {

public:
    TileLayer(int version, QString name, int extent);
    TileLayer(TileLayer&&) = default;
    TileLayer(const TileLayer&) = delete;
    ~TileLayer() = default;

    int version() const;

    QString name() const;

    int extent() const;

    std::vector<std::unique_ptr<AbstractLayerFeature>> m_features;

private:
    const int m_version;
    const QString m_name;
    const int m_extent;
};

/*
 * This class represents a vector tile deserialized form a protobuf file.
 * the class contains all map with all the layers within the tile.
 */
class VectorTile {

public:
    VectorTile();
    // The move operation of internal memory
    // is automatically handled by the std::unique_ptr
    VectorTile(VectorTile&&) = default;
    // Explicitly deleted copy-constructor so that we know we can never
    // mistakenly copy this class around.
    VectorTile(const VectorTile&) = delete;
    // Internal memory cleanup is automatically handled by the std::unique_ptr
    ~VectorTile() = default;

    bool DeserializeMessage(QByteArray data);
    static std::optional<VectorTile> fromByteArray(const QByteArray &bytes);
    static std::optional<VectorTile> fromFile(const QString &path);
    std::map<QString, std::unique_ptr<TileLayer>> m_layers;
};

namespace Bach {
    inline QString testDataDir = "testdata/";

    std::optional<VectorTile> tileFromByteArray(const QByteArray &bytes);
}

#endif // VECTORTILES_H
