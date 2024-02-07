#ifndef VECTORTILES_H
#define VECTORTILES_H

#include <QByteArray>
#include <QRect>
#include <QMap>
#include <QList>
#include <QPainterPath>

/*
 * This abstract class is the base for all the classes representing different layer features.
 */
class AbstractLayerFeature {

public:
    AbstractLayerFeature();

    enum class featureType : int8_t {
        polygon,
        line,
        point,
        unknown
    };

    virtual featureType type() const = 0;
    virtual QRect boundingRect() const { return QRect(); }

private:
    const int m_id;
};

/*
 * This class represents a plygon feature. the class contains the id and geometry of the feature.
 */
class PolygonFeature : protected AbstractLayerFeature
{
public:
    PolygonFeature(int id);
    AbstractLayerFeature::featureType type() const override { return LayerFeature::featureType::polygon; }
    QRect boundingRect() const override;
    QPainterPath polygon() const;

private:
    QPainterPath m_polygon;
};

/*
 * This class represents a linsestring feature. the class contains the id and geometry of the feature.
 */
class LineFeature : protected AbstractLayerFeature
{
public:
    LineFeature(int id);
    AbstractLayerFeature::featureType type() const override { return LayerFeature::featureType::line; }
    QRect boundingRect() const override;
    QPainterPath line() const;

private:
    QPainterPath m_line;
};

/*
 * This class represents a point feature. the class contains the id and geometry of the feature.
 */
class PointFeature : protected AbstractLayerFeature
{
public:
    PointFeature(int id);
    AbstractLayerFeature::featureType type() const override { return LayerFeature::featureType::point; }
    void addPoint(QPoint point);

    QList<QPoint> points() const;

private:
    QList<QPoint> m_points;
};

/*
 * This class represents an unkow feature.
 * these features will not be further processed.
 */
class UnknownFeature : protected AbstractLayerFeature
{
public:
    UnknownFeature(int id);
    AbstractLayerFeature::featureType type() const override { return LayerFeature::featureType::unknown; }
};

/*
 *This class represents a single layer in a vector tile.
 *the class contains a list with all the features in the layer as well as other layer details.
 */
class TileLayer {

public:
    TileLayer(int version, QString name, int extent);
    ~TileLayer();

    QRect boundingRect() const;

    int version() const;

    QString name() const;

    int extent() const;

    QList<AbstractLayerFeature*> m_features;

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
    ~VectorTile();

    void DeserializeMessage(QByteArray data);
    QRect boundingRect() const;
    QMap<QString, TileLayer*> m_layer;
};
#endif // VECTORTILES_H