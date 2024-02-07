#include "tilerenderrules.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>


template <class T>
T findValueForZoom (QMap<int, T> map, int zoomLevel, T defaultValue) {
    if (map.isEmpty()) {
        return defaultValue;
    }
    for (int key : map.keys()) {
        if (key > zoomLevel) {
            return map[key];
        }
    }
    return map.last();
}

static QColor colorFromString(QString string)
{
    string.remove(" ");
    if (string.startsWith("hsl(")) {
        QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%\\)");
        QRegularExpressionMatch match = re.match(string);
        if (match.capturedTexts().length() >= 4) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.);
        }
    }
    if (string.startsWith("hsla(")) {
        QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%,(\\d?\\.?\\d*)\\)");
        QRegularExpressionMatch match = re.match(string);
        if (match.capturedTexts().length() >= 5) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.,
                                    match.capturedTexts().at(4).toFloat());
        }
    }
    qDebug() << "failed parsing colorstring" << string;
    return QColor::fromString(string);
}


TileLayerFilter::TileLayerFilter()
{

}

TileLayerFilter TileLayerFilter::fromJson(const QJsonObject &json)
{
    return TileLayerFilter();
}

TileLayerStyle::TileLayerStyle()
{

}

TileLayerStyle::~TileLayerStyle()
{

}

TileLayerStyle* TileLayerStyle::fromJson(const QJsonObject &json)
{
    QString typeStr = json.value("type").toString();

    TileLayerStyle *newLayer = nullptr;
    if (typeStr == "background")
        newLayer = BackGroundLayerStyle::fromJson(json);
    else if (typeStr == "fill")
        newLayer = FillLayerStyle::fromJson(json);
    else if (typeStr == "line")
        newLayer = LineLayerStyle::fromJson(json);
    else
        newLayer = NotImplementedStyle::fromJson(json);
    Q_ASSERT(newLayer);

    newLayer->m_id = json.value("id").toString();
    newLayer->m_source = json.value("source").toString();
    newLayer->m_sourceLayer = json.value("source-layer").toString();
    newLayer->m_minzoom = json.value("minzoom").toInt(0);
    newLayer->m_maxzoom = json.value("minzoom").toInt(24);
    //TODO newLayer->m_visibility;

    newLayer->m_filter = TileLayerFilter::fromJson(json.value("filter").toObject());

    return newLayer;
}



BackGroundLayerStyle::BackGroundLayerStyle()
{

}

BackGroundLayerStyle::~BackGroundLayerStyle()
{

}

BackGroundLayerStyle* BackGroundLayerStyle::fromJson(const QJsonObject &json)
{
    BackGroundLayerStyle *newLayer = new BackGroundLayerStyle();

    qDebug() << "created backgroundLayer" << json.value("id").toString();

    QJsonObject paint = json.value("paint").toObject();

    QJsonValue fillColor = paint.value("background-color");
    if (fillColor.isObject()) {
        for (const auto &stop : fillColor.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            QColor singleColor = colorFromString(stop.toArray().last().toString());
            newLayer->m_backgroundColor.insert(stopValue, singleColor);
            qDebug() << "        created backgroundcolor stop" << newLayer->m_backgroundColor[stopValue].toRgb() << "from" << stop.toArray().last().toString();
        }
    } else {
        QColor singleColor = colorFromString(fillColor.toString());
        newLayer->m_backgroundColor.insert(-1, singleColor);
        qDebug() << "        created single backgroundcolor" << newLayer->m_backgroundColor[-1].toRgb() << "from" << fillColor.toString();
    }

    QJsonValue fillOpacity = paint.value("background-opacity");
    if (fillOpacity.isObject()) {
        for (const auto &stop : fillOpacity.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            float singleOpacity = stop.toArray().last().toDouble();
            newLayer->m_backgroundOpacity.insert(stopValue, singleOpacity);
        }
    } else {
        float singleOpacity = fillOpacity.toDouble();
        newLayer->m_backgroundOpacity.insert(-1, singleOpacity);
    }

    return newLayer;
}

QColor BackGroundLayerStyle::backgroundColor(int zoomLevel)
{
    return findValueForZoom<QColor>(m_backgroundColor, zoomLevel, Qt::transparent);
}

float BackGroundLayerStyle::backGroundOpacity(int zoomLevel)
{
    return findValueForZoom<float>(m_backgroundOpacity, zoomLevel, 1.);
}

FillLayerStyle::FillLayerStyle()
{

}

FillLayerStyle::~FillLayerStyle()
{

}

QColor FillLayerStyle::fillColor(int zoomLevel)
{
    return findValueForZoom<QColor>(m_fillColor, zoomLevel, Qt::black);
}

float FillLayerStyle::fillOpacity(int zoomLevel)
{
    return findValueForZoom<float>(m_fillOpacity, zoomLevel, 1.);
}

FillLayerStyle* FillLayerStyle::fromJson(const QJsonObject &json)
{
    FillLayerStyle *newLayer = new FillLayerStyle();

    qDebug() << "created fillLayer" << json.value("id").toString();

    QJsonObject paint = json.value("paint").toObject();

    QJsonValue fillColor = paint.value("fill-color");
    if (fillColor.isObject()) {
        for (const auto &stop : fillColor.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            QColor singleColor = colorFromString(stop.toArray().last().toString());
            newLayer->m_fillColor.insert(stopValue, singleColor);
            qDebug() << "        created fillcolor stop" << newLayer->m_fillColor[stopValue].toRgb() << "from" << stop.toArray().last().toString();
        }
    } else {
        QColor singleColor = colorFromString(fillColor.toString());
        newLayer->m_fillColor.insert(-1, singleColor);
        qDebug() << "        created single fillcolor" << newLayer->m_fillColor[-1].toRgb() << "from" << fillColor.toString();
    }

    QJsonValue fillOpacity = paint.value("fill-opacity");
    if (fillOpacity.isObject()) {
        for (const auto &stop : fillOpacity.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            float singleOpacity = stop.toArray().last().toDouble();
            newLayer->m_fillOpacity.insert(stopValue, singleOpacity);
        }
    } else {
        float singleOpacity = fillOpacity.toDouble();
        newLayer->m_fillOpacity.insert(-1, singleOpacity);
    }

    return newLayer;
}

LineLayerStyle::LineLayerStyle()
{

}

LineLayerStyle::~LineLayerStyle()
{

}

LineLayerStyle* LineLayerStyle::fromJson(const QJsonObject &json)
{
    LineLayerStyle *newLayer = new LineLayerStyle();

    qDebug() << "created lineLayer" << json.value("id").toString();

    //QMap<int, float> m_lineOpacity;

    QJsonObject paint = json.value("paint").toObject();

    QJsonValue lineColor = paint.value("line-color");
    if (lineColor.isObject()) {
        for (const auto &stop : lineColor.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            QColor singleColor = colorFromString(stop.toArray().last().toString());
            newLayer->m_lineColor.insert(stopValue, singleColor);
            qDebug() << "        created linecolor stop" << newLayer->m_lineColor[stopValue].toRgb() << "from" << stop.toArray().last().toString();
        }
    } else {
        QColor singleColor = colorFromString(lineColor.toString());
        newLayer->m_lineColor.insert(-1, singleColor);
        qDebug() << "        created single linecolor" << newLayer->m_lineColor[-1].toRgb() << "from" << lineColor.toString();
    }

    QJsonValue lineWidth = paint.value("line-width");
    if (lineWidth.isObject()) {
        for (const auto &stop : lineWidth.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            float singleOpacity = stop.toArray().last().toDouble();
            newLayer->m_lineWidth.insert(stopValue, singleOpacity);
            qDebug() << "        created line width stop" << newLayer->m_lineWidth[stopValue];

        }
    } else {
        float singleOpacity = lineWidth.toDouble();
        newLayer->m_lineWidth.insert(-1, singleOpacity);
        qDebug() << "        created single line width" << newLayer->m_lineWidth[-1];
    }

    QJsonValue lineOpacity = paint.value("line-opacity");
    if (lineOpacity.isObject()) {
        for (const auto &stop : lineOpacity.toObject().value("stops").toArray()) {
            int stopValue = stop.toArray().first().toInt();
            float singleOpacity = stop.toArray().last().toDouble();
            newLayer->m_lineOpacity.insert(stopValue, singleOpacity);
        }
    } else {
        float singleOpacity = lineOpacity.toDouble();
        newLayer->m_lineOpacity.insert(-1, singleOpacity);
    }

    return newLayer;
}

QColor LineLayerStyle::lineColor(int zoomLevel)
{
    return findValueForZoom<QColor>(m_lineColor, zoomLevel, Qt::black);
}

float LineLayerStyle::lineOpacity(int zoomLevel)
{
    return findValueForZoom<float>(m_lineOpacity, zoomLevel, 1.);
}

float LineLayerStyle::lineWidth(int zoomLevel)
{
    return findValueForZoom<float>(m_lineWidth, zoomLevel, 1.);
}

NotImplementedStyle::NotImplementedStyle()
{

}

NotImplementedStyle::~NotImplementedStyle()
{

}

NotImplementedStyle* NotImplementedStyle::fromJson(const QJsonObject &json)
{
    return new NotImplementedStyle();
}

TileLayerSource::TileLayerSource(QString url, QString attribution, sourceType type)
    : m_url(url)
    , m_attribution(attribution)
    , m_type(type)
{
    qDebug() << "Created TileLayerSource";
}

TileRenderRules::TileRenderRules()
{

}

TileRenderRules::~TileRenderRules()
{
    for(auto layer : m_layer) {
        delete layer;
    }
}

TileRenderRules* TileRenderRules::fromJson(const QJsonDocument &doc)
{
    TileRenderRules *rules = new TileRenderRules();

    QJsonObject main = doc.object();

    rules->m_name = main.value("name").toString();
    rules->m_id = main.value("id").toString();
    rules->m_version = main.value("version").toInt();

    QJsonObject sources = main.value("sources").toObject();

    for (const auto &id : sources.keys()) {
        QJsonObject source = sources[id].toObject();
        QString url = source.value("url").toString();
        QString attribution = source.value("attribution").toString();
        QString typeStr = source.value("type").toString();
        TileLayerSource::sourceType type;
        if (typeStr == "raster")
            type = TileLayerSource::sourceType::raster;
        else
            type = TileLayerSource::sourceType::raster;
        rules->m_sources.insert(id, TileLayerSource(url, attribution, type));
    }

    QJsonArray layers = main.value("layers").toArray();

    for (const auto &layer : layers ) {
        rules->m_layer.append( TileLayerStyle::fromJson(layer.toObject()));
    }

    return rules;
}

