#ifndef TILERENDERRULES_H
#define TILERENDERRULES_H

#include <QString>
#include <QList>
#include <QMap>
#include <QColor>
#include <QPixmap>
#include <QJsonDocument>

class TileLayerFilter
{
public:
    TileLayerFilter();

    static TileLayerFilter fromJson(const QJsonObject &json);
};

class TileLayerStyle
{
public:
    TileLayerStyle();
    virtual ~TileLayerStyle();

    enum class layerType : int8_t {
        background,
        fill,
        line,
        symbol,
        raster,
        circle,
        fillExtrusion,
        heatmap,
        hillshade,
        sky,
        model,
        unknown
    };

    static TileLayerStyle* fromJson(const QJsonObject &json);
    virtual layerType type() const = 0;

    const QString &source() const { return m_source; }
    const QString &sourceLayer() const { return m_sourceLayer; }
    QString m_id;

    QString m_source;
    QString m_sourceLayer;

    int m_minzoom = 0;
    int m_maxzoom = 24;

    TileLayerFilter m_filter;

    bool m_visibility = true;
};

class BackGroundLayerStyle : public TileLayerStyle
{
public:
    BackGroundLayerStyle();
    ~BackGroundLayerStyle();

    static BackGroundLayerStyle* fromJson(const QJsonObject &json);
    layerType type() const override { return layerType::background; };

    QColor backgroundColor(int zoomLevel);
    float backGroundOpacity(int zoomLevel);

private:

    QMap<int, QColor> m_backgroundColor;
    QMap<int, float> m_backgroundOpacity;
    //m_backgroundEmissiveStrength;
    //QImage m_backgroundPattern;
    //m_visibility;
    //
};

class FillLayerStyle : public TileLayerStyle
{
public:
    FillLayerStyle();
    ~FillLayerStyle();

    static FillLayerStyle* fromJson(const QJsonObject &json);
    layerType type() const override { return layerType::fill; };

    QColor fillColor(int zoomLevel);
    float fillOpacity(int zoomLevel);
private:
    //maps to implement feature-state
    QMap<int, QColor> m_fillColor;
    QMap<int, float> m_fillOpacity;

    //Not implemented for now
    //QMap<int, float> m_fillOutlineColor;
    //bool m_fillAntialias = true;
    //QPixmap m_fillPattern;
    //m_fillSortKey;
    //m_fillTranslate;
    //m_fillTranslateAnchor;
};

class LineLayerStyle : public TileLayerStyle
{
public:
    LineLayerStyle();
    ~LineLayerStyle();

    static LineLayerStyle* fromJson(const QJsonObject &json);
    layerType type() const override { return layerType::line; };

    QColor lineColor(int zoomLevel);
    float lineOpacity(int zoomLevel);
    float lineWidth(int zoomLevel);
private:
    //maps to implement feature-state
    Qt::PenCapStyle m_lineCap = Qt::FlatCap;
    Qt::PenJoinStyle m_lineJoin = Qt::MiterJoin;
    QMap<int, QColor> m_lineColor;
    QMap<int, float> m_lineOpacity;
    QMap<int, float> m_lineWidth;

    //Not implemented for now
    //float m_lineBlur = 0;
    //QList<float> m_lineDashArray;
    //QMap<int, float> m_lineGapWidth;
    //m_lineGradient;
    //float m_lineMiterLimit;
    //QMap<int, real> m_lineOffset;
    //QPixmap m_linePattern;
    //float m_lineRoundLimit;
    //m_lineSortKey;
    //m_lineTranslate;
    //m_lineTransalteAnchor;
    //m_lineTrimOffset;
};

class NotImplementedStyle : public TileLayerStyle
{
public:
    NotImplementedStyle();
    ~NotImplementedStyle();

    static NotImplementedStyle* fromJson(const QJsonObject &json);
    layerType type() const override { return layerType::unknown; };
};

class TileLayerSource
{
public:

    enum class sourceType : int8_t {
        vector,
        raster
    };

    TileLayerSource(QString url, QString attribution, sourceType type);

    QString m_url;
    QString m_attribution;
    sourceType m_type;
};


class TileRenderRules
{
public:
    TileRenderRules();
    ~TileRenderRules();

    static TileRenderRules* fromJson(const QJsonDocument &doc);

    const QList<TileLayerStyle*> &layer() { return m_layer; }

private:
    QString m_name;
    QString m_id;
    int m_version;

    QMap<QString, TileLayerSource> m_sources;
    QList<TileLayerStyle*> m_layer;


private:

};

#endif // TILERENDERRULES_H
