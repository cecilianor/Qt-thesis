#ifndef VECTORTILES_H
#define VECTORTILES_H

#include <QByteArray>
#include <QRect>
#include <QMap>
#include <QList>
#include <QPainterPath>


class LayerFeature {

public:
    LayerFeature();

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

class PolygonFeature : protected LayerFeature
{
public:
    PolygonFeature(int id);
    LayerFeature::featureType type() const override { return LayerFeature::featureType::polygon; }
    QRect boundingRect() const override;
    QPainterPath polygon() const;

private:
    QPainterPath m_polygon;
};


class LineFeature : protected LayerFeature
{
public:
    LineFeature(int id);
    LayerFeature::featureType type() const override { return LayerFeature::featureType::line; }
    QRect boundingRect() const override;
    QPainterPath line() const;

private:
    QPainterPath m_line;
};


class PointFeature : protected LayerFeature
{
public:
    PointFeature(int id);
    LayerFeature::featureType type() const override { return LayerFeature::featureType::point; }
    void addPoint(QPoint point);

    QList<QPoint> points() const;

private:
    QList<QPoint> m_points;
};

class UnknownFeature : protected LayerFeature
{
public:
    UnknownFeature(int id);
    LayerFeature::featureType type() const override { return LayerFeature::featureType::unknown; }
};

class TileLayer {

public:
    TileLayer(int version, QString name, int extent);
    ~TileLayer();

    QRect boundingRect() const;

    int version() const;

    QString name() const;

    int extent() const;

    QList<LayerFeature*> m_features;

private:
    const int m_version;
    const QString m_name;
    const int m_extent;

};

class VectorTile {

public:
    VectorTile();
    ~VectorTile();

    void DeserializeMessage(QByteArray data);
    QRect boundingRect() const;
    QMap<QString, TileLayer*> m_layer;
};
#endif // VECTORTILES_H
