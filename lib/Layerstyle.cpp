#include <QRegularExpression>
#include <QtMath>

#include "Layerstyle.h"

/*!
 * \brief AbstractLayerStyle::fromJson parses different layer style types.
 *
 * The layerType data type is converted to strings.
 *
 * Supported layer types:
 * - "background"
 * - "fill"
 * - "line"
 * - "symbol"
 *
 * - Otherwise: NotImplementedStyle will be used.
 *
 * \param jsonObj is the JsonObject to parse
 * \return The Abstract Layer style with all parsed data.
 */
std::unique_ptr<AbstractLayerStyle> AbstractLayerStyle::fromJson(const QJsonObject &json)
{
    QString layerType = json.value("type").toString();
    std::unique_ptr<AbstractLayerStyle> returnLayerPtr;
    if(layerType == "background"){
        returnLayerPtr = BackgroundStyle::fromJson(json);
    }else if( layerType == "fill"){
        returnLayerPtr = FillLayerStyle::fromJson(json);
    }else if(layerType == "line"){
        returnLayerPtr =  LineLayerStyle::fromJson(json);
    }else if(layerType == "symbol"){
        returnLayerPtr = SymbolLayerStyle::fromJson(json);
    }else{
        returnLayerPtr = NotImplementedStyle::fromJson(json);
    }
    AbstractLayerStyle *newLayer = returnLayerPtr.get();
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
    return returnLayerPtr;
}

/* Stylesheet parsing functions are documented and implemented below. */

/*!
 * \brief StyleSheet::~StyleSheet is the StyleSheet class destructor.
 */
StyleSheet::~StyleSheet()
{
}

/*!
 * \brief StyleSheet::parseSheet parses a style sheet and populates the
 * layers array of a styleSheetObject.
 *
 * \param styleSheet is a style sheet to parse, passed as a reference to
 * a QJsonDocument.
 */
void StyleSheet::parseSheet(const QJsonDocument &styleSheet)
{
   QJsonObject styleSheetObject = styleSheet.object();
    m_id = styleSheetObject.value("id").toString();
    m_version = styleSheetObject.value("version").toInt();
    m_name = styleSheetObject.value("name").toString();

    QJsonArray layers = styleSheetObject.value("layers").toArray();
    for(const auto &layer : layers){
        m_layerStyles.push_back(AbstractLayerStyle::fromJson(layer.toObject()));
    }
}

/*!
 * \brief StyleSheet::fromJson parses a style sheet.
 *
 * \param styleSheet is a style sheet to parse, passed as a reference to
 * a QJsonDocument.
 *
 * \return  is either the parsed data or a nullopt of there was no data.
 */
std::optional<StyleSheet> StyleSheet::fromJson(const QJsonDocument &input)
{
    StyleSheet out;
    out.parseSheet(input);
    return out;
}
