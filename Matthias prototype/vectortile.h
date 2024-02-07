#ifndef VECTORTILE_H
#define VECTORTILE_H

#include <QString>
#include <QPainterPath>
#include <QMap>

class VectorFeature
{
public:
    VectorFeature() {}

    enum class featureType : int8_t {
        polygon,
        line,
        point,
        unknown
    };

    virtual featureType type() const = 0;
    virtual QRect boundingRect() const { return QRect(); }

    int m_id;
};

class PolygonFeature : public VectorFeature
{
public:
    PolygonFeature() {}
    VectorFeature::featureType type() const override { return VectorFeature::featureType::polygon; }
    QRect boundingRect() const override { return m_polygon.boundingRect().toRect(); }

    QPainterPath m_polygon;
};

class LineFeature : public VectorFeature
{
public:
    LineFeature() {}
    VectorFeature::featureType type() const override { return VectorFeature::featureType::line; }
    QRect boundingRect() const override { return m_line.boundingRect().toRect(); }

    QPainterPath m_line;
};

class PointFeature : public VectorFeature
{
public:
    PointFeature() {}
    VectorFeature::featureType type() const override { return VectorFeature::featureType::point; }

    QList<QPoint> m_points;
};

class UnknownFeature : public VectorFeature
{
public:
    UnknownFeature() {}
    VectorFeature::featureType type() const override { return VectorFeature::featureType::unknown; }
};

class VectorLayer
{
public:
    VectorLayer();
    ~VectorLayer();

    QRect boundingRect() const;

    QList<VectorFeature*> m_features;
};

class VectorTile
{
public:
    VectorTile();
    ~VectorTile();

    void load(QByteArray data);
    QRect boundingRect() const;

    QMap<QString, VectorLayer*> m_layer;
};

#endif // VECTORTILE_H
