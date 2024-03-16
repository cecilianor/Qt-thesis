#include "Layerstyle.h"
#include <QRegularExpression>
#include <QtMath>

static QColor getColorFromString(QString colorString)
{
    colorString.remove(" ");
    if (colorString.startsWith("hsl(")) {
        QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%\\)");
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 4) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.);
        }
    }
    if (colorString.startsWith("hsla(")) {
        QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%,(\\d?\\.?\\d*)\\)");
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 5) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.,
                                    match.capturedTexts().at(4).toFloat());
        }
    }
    return QColor::fromString(colorString);
}





template <class T>
T getStopOutput(QList<QPair<int, T>> list, int currentZoom)
{
    for(const auto stop : list)
    {
        if(currentZoom < stop.first){
            return stop.second;
        }
    }
    return list.last().second;
}
/*
 * ----------------------------------------------------------------------------
 */

BackgroundStyle *BackgroundStyle::fromJson(const QJsonObject &json)
{
    BackgroundStyle* returnLayer = new BackgroundStyle();
    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("background-color"))
    {
        QJsonValue backgroundColor = paint.value("background-color");
        if(backgroundColor.isObject()) {
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : backgroundColor.toObject().value("stops").toArray())
            {
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_backgroundColor.setValue(stops);
        }else if(backgroundColor.isArray()){
            returnLayer->m_backgroundColor.setValue(backgroundColor.toArray());
        }else{
            returnLayer->m_backgroundColor.setValue(getColorFromString(backgroundColor.toString()));
        }

    }

    if(paint.contains("background-opacity")){
        QJsonValue backgroundOpacity= paint.value("background-opacity");
        if(backgroundOpacity.isObject()) {
            QList<QPair<int, float>> stops;
            for(const auto &stop : backgroundOpacity.toObject().value("stops").toArray())
            {
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
             returnLayer->m_backgroundOpacity.setValue(stops);
        }else if(backgroundOpacity.isArray()){
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toArray());
        }else{
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toDouble());
        }
    }

    return returnLayer;
}

QVariant BackgroundStyle::getColorAtZoom(int zoomLevel) const
{
    if(m_backgroundColor.isNull()){
        return QColor(Qt::GlobalColor::black);
    }else if(m_backgroundColor.typeId() != QMetaType::Type::QColor && m_backgroundColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_backgroundColor.value<QList<QPair<int, QColor>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_backgroundColor;
    }
}

QVariant BackgroundStyle::getOpacityAtZoom(int zoomLevel) const
{
    if(m_backgroundOpacity.isNull()){
        return QVariant(1);
    }else if(m_backgroundOpacity.typeId() != QMetaType::Type::Double && m_backgroundOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_backgroundOpacity.value<QList<QPair<int, float>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_backgroundColor;
    }
}
/*
 * ----------------------------------------------------------------------------
 */


FillLayerStyle *FillLayerStyle::fromJson(const QJsonObject &json)
{
    FillLayerStyle* returnLayer = new FillLayerStyle();

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    returnLayer->m_antialias = paint.contains("fill-antialias") ? paint.value("fill-antialias").toBool() : true;
    if(paint.contains("fill-color")){
        QJsonValue fillColor = paint.value("fill-color");
        if(fillColor.isObject())
        {
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : fillColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillColor.setValue(stops);
        }else if(fillColor.isArray()){
            returnLayer->m_fillColor.setValue(fillColor.toArray());
        }else{
            returnLayer->m_fillColor.setValue(getColorFromString(fillColor.toString()));
        }
    }

    if(paint.contains("fill-opacity")){
        QJsonValue fillOpacity = paint.value("fill-opacity");
        if(fillOpacity.isObject())
        {
            QList<QPair<int, float>> stops;
            for(const auto &stop : fillOpacity.toObject().value("stops").toArray()){
                int zoomSopt = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomSopt, opacityStop));
            }
            returnLayer->m_fillOpacity.setValue(stops);
        }else if(fillOpacity.isArray()){
            returnLayer->m_fillOpacity.setValue(fillOpacity.toArray());
        }else{
            returnLayer->m_fillOpacity.setValue(fillOpacity.toDouble());
        }
    }

    if(paint.contains("fill-outline-color")){
        QJsonValue fillOutlineColor = paint.value("fill-outline-color");
        if(fillOutlineColor.isObject())
        {
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : fillOutlineColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
            }
            returnLayer->m_fillOutlineColor.setValue(stops);
        }else if(fillOutlineColor.isArray()){
            returnLayer->m_fillOutlineColor.setValue(fillOutlineColor.toArray());
        }else{
            returnLayer->m_fillOutlineColor.setValue(getColorFromString(fillOutlineColor.toString()));
        }
    }

    return returnLayer;
}

QVariant FillLayerStyle::getFillColorAtZoom(int zoomLevel) const
{
    if(m_fillColor.isNull()){
        return QColor(Qt::GlobalColor::black);
    }else if(m_fillColor.typeId() != QMetaType::Type::QColor && m_fillColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_fillColor.value<QList<QPair<int, QColor>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_fillColor;
    }
}

QVariant FillLayerStyle::getFillOpacityAtZoom(int zoomLevel) const
{
    if(m_fillOpacity.isNull()){
        return QVariant(1);
    }else if(m_fillOpacity.typeId() != QMetaType::Type::QColor && m_fillOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_fillOpacity.value<QList<QPair<int, float>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_fillOpacity;
    }
}

QVariant FillLayerStyle::getFillOutLineColorAtZoom(int zoomLevel) const
{
    if(m_antialias == false) return QVariant();
    if(m_fillOutlineColor.isNull()){
        return QVariant();
    }
    if(m_fillOutlineColor.typeId() != QMetaType::Type::QColor && m_fillOutlineColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_fillOutlineColor.value<QList<QPair<int, QColor>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_fillOutlineColor;
    }
}


/*
 * ----------------------------------------------------------------------------
 */


LineLayerStyle *LineLayerStyle::fromJson(const QJsonObject &json)
{

    LineLayerStyle* returnLayer = new LineLayerStyle();

    //parsing layout properties
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)
    QJsonObject layout = json.value("layout").toObject();
    returnLayer->m_lineCap = layout.contains("line-cap") ? layout.value("line-cap").toString() : QString("butt");
    returnLayer->m_lineJoin = layout.contains("line-join") ? layout.value("line-join").toString() : QString("miter");
    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("line-dasharray")){
        for(const auto &length: paint.value("line-dasharray").toArray()){
            returnLayer->m_lineDashArray.append(length.toInt());
        }
    }

    if(paint.contains("line-color")){
        QJsonValue lineColor = paint.value("line-color");
        if(lineColor.isObject()){
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : lineColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int , QColor>(zoomStop, colorStop));
            }
            returnLayer->m_lineColor.setValue(stops);
        }else if(lineColor.isArray()){
            returnLayer->m_lineColor.setValue(lineColor.toArray());
        }else{
            returnLayer->m_lineColor.setValue(getColorFromString(lineColor.toString()));
        }
    }

    if(paint.contains("line-opacity")){
        QJsonValue lineOpacity = paint.value("line-opacity");
        if(lineOpacity.isObject()){
            QList<QPair<int, float>> stops;
            for(const auto &stop : lineOpacity.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int , float>(zoomStop, opacityStop));
            }
            returnLayer->m_lineOpacity.setValue(stops);
        }else if(lineOpacity.isArray()){
            returnLayer->m_lineOpacity.setValue(lineOpacity.toArray());
        }else{
            returnLayer->m_lineOpacity.setValue(lineOpacity.toDouble());
        }
    }

    if(paint.contains("line-width")){
        QJsonValue lineWidth = paint.value("line-width");
        if(lineWidth.isObject()){
            QList<QPair<int, int>> stops;
            for(const auto &stop : lineWidth.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                int widthStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, widthStop));
            }
            returnLayer->m_lineWidth.setValue(stops);
        }else if(lineWidth.isArray()){
            returnLayer->m_lineWidth.setValue(lineWidth.toArray());
        }else{
            returnLayer->m_lineWidth.setValue(lineWidth.toInt());
        }
    }


    return returnLayer;
}


QVariant LineLayerStyle::getLineColorAtZoom(int zoomLevel) const
{
    if(m_lineColor.isNull()){
        return QVariant(QColor(Qt::GlobalColor::black));
    }else if(m_lineColor.typeId() != QMetaType::Type::QColor && m_lineColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_lineColor.value<QList<QPair<int, QColor>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_lineColor;
    }
}


QVariant LineLayerStyle::getLineOpacityAtZoom(int zoomLevel) const
{
    if(m_lineOpacity.isNull()){
        return QVariant(1);
    }else if(m_lineOpacity.typeId() != QMetaType::Type::Double && m_lineOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_lineOpacity.value<QList<QPair<int, float>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_lineOpacity;
    }
}


QVariant LineLayerStyle::getLineWidthAtZoom(int zoomLevel) const
{
    if(m_lineWidth.isNull()){
        return QVariant(1);
    }else if(m_lineWidth.typeId() != QMetaType::Type::Int && m_lineWidth.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_lineWidth.value<QList<QPair<int, int>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_lineWidth;
    }
}

Qt::PenJoinStyle LineLayerStyle::getJoinStyle() const
{
    if(m_lineJoin == "bevel"){
        return Qt::PenJoinStyle::BevelJoin;
    }else if(m_lineJoin == "miter"){
        return Qt::PenJoinStyle::MiterJoin;
    }else{
        return Qt::PenJoinStyle::RoundJoin;
    }
}

Qt::PenCapStyle LineLayerStyle::getCapStyle() const
{
    if(m_lineCap == "butt"){
        return Qt::PenCapStyle::FlatCap;
    }else if(m_lineCap == "round"){
        return Qt::PenCapStyle::RoundCap;
    }else{
        return Qt::PenCapStyle::SquareCap;
    }
}

/*
 * ----------------------------------------------------------------------------
 */


SymbolLayerStyle *SymbolLayerStyle::fromJson(const QJsonObject &json)
{

    SymbolLayerStyle* returnLayer = new SymbolLayerStyle();

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)

    if(layout.contains("text-size")){
        QJsonValue textSize = layout.value("text-size");
        if(textSize.isObject()){
            QList<QPair<int, int>> stops;
            for(const auto &stop : textSize.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                int sizeStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, sizeStop));
            }
            returnLayer->m_textSize.setValue(stops);
        }else if(textSize.isArray()){
            returnLayer->m_textSize.setValue(textSize.toArray());
        }else{
            returnLayer->m_textSize.setValue(textSize.toInt());
        }
    }

    if(layout.contains("text-font")){

        returnLayer->m_textFont = QFont(layout.value("text-font").toVariant().toStringList());
    }else{
        returnLayer->m_textFont = QFont({"Open Sans Regular","Arial Unicode MS Regular"});
    }

    if(layout.contains("text-field")){
        returnLayer->m_textField = QVariant(layout.value("text-field").toArray());
    }
    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("text-color")){
        QJsonValue textColor = paint.value("text-color");
        if(textColor.isObject()){
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : textColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_textColor.setValue(stops);
        }else if(textColor.isArray()){
            returnLayer->m_textColor.setValue(textColor.toArray());
        }else{
            returnLayer->m_textColor.setValue(getColorFromString(textColor.toString()));
        }
    }

    if(paint.contains("text-opacity")){
        QJsonValue textOpacity = paint.value("text-opacity");
        if(textOpacity.isObject()){
            QList<QPair<int, float>> stops;
            for(const auto &stop : textOpacity.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_textOpacity.setValue(stops);
        }else if(textOpacity.isArray()){
            returnLayer->m_textOpacity.setValue(textOpacity.toArray());
        }else{
            returnLayer->m_textOpacity.setValue(textOpacity.toDouble());
        }
    }

    return returnLayer;
}


QVariant SymbolLayerStyle::getTextSizeAtZoom(int zoomLevel) const
{

    if(m_textSize.isNull()){
        return QVariant(16);
    }else if(m_textSize.typeId() != QMetaType::Type::Double && m_textSize.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_textSize.value<QList<QPair<int, int>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textSize);
    }
}

QVariant SymbolLayerStyle::getTextColorAtZoom(int zoomLevel) const
{
    if(m_textColor.isNull()){
        return QVariant(QColor(Qt::GlobalColor::black));
    }else if(m_textColor.typeId() != QMetaType::Type::QColor && m_textColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_textColor.value<QList<QPair<int, QColor>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textColor);
    }
}

QVariant SymbolLayerStyle::getTextOpacityAtZoom(int zoomLevel) const
{
    if(m_textOpacity.isNull()){
        return QVariant(1);
    }else if(m_textOpacity.typeId() != QMetaType::Type::Double && m_textOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_textOpacity.value<QList<QPair<int, float>>>();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textOpacity);
    }
}

/*
 * ----------------------------------------------------------------------------
 */

NotImplementedStyle* NotImplementedStyle::fromJson(const QJsonObject &json)
{
    NotImplementedStyle* returnLayer = new NotImplementedStyle();
    return returnLayer;
}



/*
 * ----------------------------------------------------------------------------
 */



AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)
{
    QString layerType = json.value("type").toString();
    AbstractLayereStyle *newLayer = nullptr;
    if(layerType == "background"){
        newLayer = BackgroundStyle::fromJson(json);
    }else if( layerType == "fill"){
        newLayer = FillLayerStyle::fromJson(json);
    }else if(layerType == "line"){
        newLayer =  LineLayerStyle::fromJson(json);
    }else if(layerType == "symbol"){
        newLayer = SymbolLayerStyle::fromJson(json);
    }else{
        newLayer = NotImplementedStyle::fromJson(json);
    }

    newLayer->m_id = json.value("id").toString();
    newLayer->m_source = json.value("source").toString();
    newLayer->m_sourceLayer = json.value("source-layer").toString();
    newLayer->m_minZoom = json.value("minzoom").toInt(0);
    newLayer->m_maxZoom = json.value("maxzoom").toInt(24);
    QJsonValue layout = json.value("layout");
    if(layout != QJsonValue::Undefined){
        newLayer->m_visibility = (layout.toObject().contains("visibility")) ? layout.toObject().value("visibility").toString() : "none";
    } else {
        newLayer->m_visibility = QString("none");
    }

    if(json.contains("filter")){
        newLayer->m_filter = json.value("filter").toArray();
    }
    return newLayer;
}


/*
 * ----------------------------------------------------------------------------
 */


StyleSheet::~StyleSheet()
{
    //for(auto layer : m_layerStyles) {
        //delete layer;
    //}
}

void StyleSheet::parseSheet(const QJsonDocument &styleSheet)
{
    QJsonObject styleSheetObject = styleSheet.object();
    m_id = styleSheetObject.value("id").toString();
    m_version = styleSheetObject.value("version").toInt();
    m_name = styleSheetObject.value("name").toString();

    QJsonArray layers = styleSheetObject.value("layers").toArray();
    for(const auto &layer : layers){
        m_layerStyles.append(AbstractLayereStyle::fromJson(layer.toObject()));
    }
}

std::optional<StyleSheet> StyleSheet::fromJson(const QJsonDocument& input)
{
    StyleSheet out;
    out.parseSheet(input);
    return out;
}

