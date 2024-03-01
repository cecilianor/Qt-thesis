#include "Layerstyle.h"
#include <QRegularExpression>
#include <QtMath>

/* Create a QColor obecject from an hsl color string.
     *The string is expected to be in one of the following formats:
     *"hsl(hue, stauration%, lightness%)"
     *"hsla(hue, stauration%, lightness%, alpha)"
     *
     * Parameters:
     *      colorString expects a QString containing the color.
     *
     * Returns a QColor object.
     */
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


/* Perform a linear interpolation of a numerical value based on QPair inputs.
     *
     * Parameters:
     *      stop1 expects a QPair<int, T> wich is the first stop in the interpolation.
     *      stop2 expects a QPair<int, T> wich is the second stop in the interpolation.
     *      currentZoom expects an integer to be used as the interpolation input.
     *
     * Returns the output of the interpolation as type T.
     */
template <class T>
T interpolate(QPair<int, T> stop1, QPair<int, T> stop2, int currentZoom)
{
    T lerpedValue = stop1.second + (currentZoom - stop1.first)*(stop2.second - stop1.second)/(stop2.first - stop1.first);
    return lerpedValue;
}


/* Perform a linear interpolation of a QColor value based on QPair inputs.
     *
     * Parameters:
     *      stop1 expects a QPair<int, QColor> wich is the first stop in the interpolation.
     *      stop2 expects a QPair<int, QColor> wich is the second stop in the interpolation.
     *      currentZoom expects an integer to be used as the interpolation input.
     *
     * Returns the output of the interpolation as a QColor object.
     */
static QColor interpolateColor(QPair<int, QColor> stop1, QPair<int, QColor> stop2, int currentZoom)
{
    if (stop1.second.hue() == stop2.second.hue()) {
        return stop1.second;
    }

    int lerpedSaturation = stop1.second.hslSaturation() + (stop2.second.hslSaturation() - stop1.second.hslSaturation()) * (currentZoom - stop1.first)/(stop2.first - stop1.first);

    int lerpedLightness = stop1.second.lightness() + (stop2.second.lightness() - stop1.second.lightness()) * (currentZoom - stop1.first)/(stop2.first - stop1.first);

    int angularDistance = stop2.second.hslHue() - stop1.second.hslHue();

    if(angularDistance > 180){
        angularDistance = angularDistance - 360;
    }else if(angularDistance < - 180){
        angularDistance = angularDistance + 360;
    }
    int lerpedHue = stop1.second.hslHue() + angularDistance * (currentZoom - stop1.first)/(stop2.first - stop1.first);

    float lerpedAlpha = stop1.second.alphaF() + (stop2.second.alphaF() - stop1.second.alphaF()) * (currentZoom - stop1.first)/(stop2.first - stop1.first);

    QColor lerpedColor;
    lerpedColor.setHslF(lerpedHue, lerpedSaturation, lerpedLightness, lerpedAlpha);
    return lerpedColor;


}

/* Determines which two interpolation stops are apropritate to use for a given zoom
     *value from a QList of QPairs.
     *
     * Parameters:
     *     list expects a QList of QPairs conatining all the stops.
     *     currentZoom expect an integer which is the input value to be used in the interpolation.
     *
     * Returns the return value from the interpolation function of type T.
     *
     * This function assumes the stops are in sorted order, could be useful to check!
     */
template <class T>
T getLerpedValue(QList<QPair<int, T>> list, int currentZoom)
{
    // We could use some error handling for the case where
    // the list passed is empty.
    if (list.length() == 0) {
        qDebug() << "Major developer error!!\n";
        std::abort();
    }

    /* If we only have one element in the list, there's nothing to lerp
     * and we just return that data point.
     *
     * If our input value is below the lowest key in the dataset, then
     * we also just return the first data point.
     *
     * If our input value is higher than the highest key in the dataset, then
     * we return the data of the highest key.
     *
     * If these conditions are not met, we search for the two indices where our input value falls
     * between and do the interpolating between these two indices.
     */
    if(list.length() == 1 || currentZoom <= list.first().first) {
        return list.first().second;
    } else if (currentZoom >= list.last().first) {
        return list.last().second;
    }else{
        // Find the first stop that has zoom-level >= to currentZoom.
        bool indexFound = false;
        int index = 0;
        for (int i = 0; i < list.size(); i++) {
            const auto &item = list[i];
            if (currentZoom >= item.first) {
                indexFound = true;
                index = i;
                break;
            }
        }
        // Could use better error handling here. Check that the first index is always found.
        if (!indexFound) {
            qDebug() << "Major developer error!!\n";
            std::abort();
        }

        // Check that the range is not outside bounds.
        if (index + 1 >= list.size()) {
            qDebug() << "Major developer error!!\n";
            std::abort();
        }
        return interpolate(list.at(index), list.at(index + 1), currentZoom);
    }
}

/* Determines which two interpolation stops are apropritate to use for a given zoom
     *value from a QList of QPairs. This function is specific for color interpolation.
     *
     * Parameters:
     *     list expects a QList of QPairs conatining all the stops.
     *     currentZoom expect an integer which is the input value to be used in the interpolation.
     *
     * Returns the return value from the interpolation function of type QColor.
     */
static QColor getLerpedColorValue(QList<QPair<int, QColor>> list, int currentZoom)
{
    if(list.length() == 1 || currentZoom <= list.begin()->first){
        return list.begin()->second;
    }else{
        if(currentZoom >= list.at(list.length() - 1).first){
            return list.at(list.length() - 1).second;
        }else{
            int index = 0;
            while(currentZoom < list.at(index).first)
            {
                index++;
            }

            return interpolateColor(list.at(index), list.at(index), currentZoom);
        }
    }
}

/*
 * ----------------------------------------------------------------------------
 */

/*Parses the data from a QJsonObject containing styling properties of a layer of type background.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type BackgroundStyle to the newly created layer with the parsed properties.
     */
BackgroundStyle *BackgroundStyle::fromJson(const QJsonObject &json)
{
    BackgroundStyle* returnLayer = new BackgroundStyle();
    returnLayer->m_id = json.value("id").toString();
    returnLayer->m_source = json.value("source").toString();
    returnLayer->m_sourceLayer = json.value("source-layer").toString();
    returnLayer->m_maxZoom =  json.contains("maxzoom") ? json.value("maxzoom").toInt() : 24;
    returnLayer->m_minZoom =  json.contains("minzoom") ? json.value("minzoom").toInt() : 0;

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    returnLayer->m_visibility = layout.contains("visibility") ? layout.value("visibility").toString() : QString("visible");

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("background-color"))
    {
        if(paint.value("background-color").isObject()) {
            for(const auto &stop : paint.value("background-color").toObject().value("stops").toArray())
            {
                returnLayer->addBackgroundColorStop(stop.toArray().first().toInt(), stop.toArray().last().toString());
            }
        }else{
            returnLayer->addBackgroundColorStop(-1, paint.value("background-color").toString());
        }

    }

    if(paint.contains("background-opacity")){
        if(paint.value("background-opacity").isObject()) {
            for(const auto &stop : paint.value("background-opacity").toObject().value("stops").toArray())
            {
                returnLayer->addBackgroundOpacityStop(stop.toArray().first().toInt(), stop.toArray().last().toDouble());
            }
        }else{
            returnLayer->addBackgroundOpacityStop(-1, paint.value("background-opacity").toDouble());
        }
    }

    return returnLayer;
}

//The following functions are getters and setters for the different background layer properties.
QColor BackgroundStyle::getColorAtZoom(int zoomLevel) const
{
    return getLerpedColorValue(m_backgroundColor, zoomLevel);
}

float BackgroundStyle::getOpacityAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_backgroundOpacity, zoomLevel);
}

void BackgroundStyle::addBackgroundColorStop(int zoomLevel, QString hslColor)
{
    m_backgroundColor.append(QPair<int, QColor>(zoomLevel, getColorFromString(hslColor)));
}

void BackgroundStyle::addBackgroundOpacityStop(int zoomLevel, float opacity)
{
    m_backgroundOpacity.append(QPair<int, float>(zoomLevel, opacity));
}
/*
 * ----------------------------------------------------------------------------
 */

/*Parses the data from a QJsonObject containing styling properties of a layer of type fill.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type FillLayerStyle to the newly created layer with the parsed properties.
     */
FillLayerStyle *FillLayerStyle::fromJson(const QJsonObject &json)
{
    FillLayerStyle* returnLayer = new FillLayerStyle();
    returnLayer->m_id = json.value("id").toString();
    returnLayer->m_source = json.value("source").toString();
    returnLayer->m_sourceLayer = json.value("source-layer").toString();
    returnLayer->m_maxZoom =  json.contains("maxzoom") ? json.value("maxzoom").toInt() : 24;
    returnLayer->m_minZoom =  json.contains("minzoom") ? json.value("minzoom").toInt() : 0;

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    returnLayer->m_visibility = layout.contains("visibility") ? layout.value("visibility").toString() : QString("visible");

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    returnLayer->m_antialias = paint.contains("fill-antialias") ? paint.value("fill-antialias").toBool() : true;
    if(paint.contains("fill-color")){
        if(paint.value("fill-color").isObject())
        {
            for(const auto &stop : paint.value("fill-color").toObject().value("stops").toArray()){
                returnLayer->addFillColorStop(stop.toArray().first().toInt(), stop.toArray().last().toString());
            }
        }else{
            returnLayer->addFillColorStop(-1, paint.value("fill-color").toString());
        }
    }

    if(paint.contains("fill-opacity")){
        if(paint.value("fill-opacity").isObject())
        {
            for(const auto &stop : paint.value("fill-opacity").toObject().value("stops").toArray()){
                returnLayer->addFillOpacityStop(stop.toArray().first().toInt(), stop.toArray().last().toDouble());
            }
        }else{
            returnLayer->addFillOpacityStop(-1, paint.value("fill-opacity").toDouble());
        }
    }

    if(paint.contains("fill-outline-color")){
        if(paint.value("fill-outline-color").isObject())
        {
            for(const auto &stop : paint.value("fill-outline-color").toObject().value("stops").toArray()){
                returnLayer->addFillOutlineColorStop(stop.toArray().first().toInt(), stop.toArray().last().toString());
            }
        }else{
            returnLayer->addFillOutlineColorStop(-1, paint.value("fill-outline-color").toString());
        }
    }

    return returnLayer;
}

//The following functions are getters and setters for the different background layer properties.
QColor FillLayerStyle::getFillColorAtZoom(int zoomLevel) const
{
    return getLerpedColorValue(m_fillColor, zoomLevel);
}

float FillLayerStyle::getFillOpacityAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_fillOpacity, zoomLevel);
}

QColor FillLayerStyle::getFillOutLineColorAtZoom(int zoomLevel) const
{
    return getLerpedColorValue(m_fillOutlineColor, zoomLevel);
}

void FillLayerStyle::addFillColorStop(int zoomLevel, QString hslColor)
{
    m_fillColor.append(QPair<int, QColor>(zoomLevel, getColorFromString(hslColor)));
}

void FillLayerStyle::addFillOpacityStop(int zoomLevel, float opacity)
{
    m_fillOpacity.append(QPair<int, float>(zoomLevel, opacity));
}

void FillLayerStyle::addFillOutlineColorStop(int zoomLevel, QString hslColor)
{
    m_fillOutlineColor.append(QPair<int, QColor>(zoomLevel, getColorFromString(hslColor)));
}

/*
 * ----------------------------------------------------------------------------
 */

/*Parses the data from a QJsonObject containing styling properties of a layer of type line.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type LineLayerStyle to the newly created layer with the parsed properties.
     */
LineLayerStyle *LineLayerStyle::fromJson(const QJsonObject &json)
{

    LineLayerStyle* returnLayer = new LineLayerStyle();
    returnLayer->m_id = json.value("id").toString();
    returnLayer->m_source = json.value("source").toString();
    returnLayer->m_sourceLayer = json.value("source-layer").toString();
    returnLayer->m_maxZoom =  json.contains("maxzoom") ? json.value("maxzoom").toInt() : 24;
    returnLayer->m_minZoom =  json.contains("minzoom") ? json.value("minzoom").toInt() : 0;
    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    returnLayer->m_visibility = layout.contains("visibility") ? layout.value("visibility").toString() : QString("visible");
    returnLayer->m_lineCap = layout.contains("line-cap") ? layout.value("line-cap").toString() : QString("butt");
    returnLayer->m_lineJoin = layout.contains("line-join") ? layout.value("line-join").toString() : QString("miter");
    if(layout.contains("line-miter-limit")){
        if(layout.value("line-miter-limit").isObject()){
            for(const auto &stop : layout.value("line-miter-limit").toObject().value("stops").toArray()){
                returnLayer->addLineMitterLimitStop(stop.toArray().first().toInt(), stop.toArray().last().toDouble());
            }
        }else{
            returnLayer->addLineMitterLimitStop(-1, layout.value("line-miter-limit").toDouble());
        }
    }
    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("line-color")){
        if(paint.value("line-color").isObject()){
            for(const auto &stop : paint.value("line-color").toObject().value("stops").toArray()){
                returnLayer->addLineColorStop(stop.toArray().first().toInt(), stop.toArray().last().toString());
            }
        }else{
            returnLayer->addLineColorStop(-1, paint.value("line-color").toString());
        }
    }

    if(paint.contains("line-dasharray")){
        for(const auto &length: paint.value("line-dasharray").toArray()){
            returnLayer->m_lineDashArray.append(length.toInt());
        }
    }

    if(paint.contains("line-opacity")){
        if(paint.value("line-opacity").isObject()){
            for(const auto &stop : paint.value("line-opacity").toObject().value("stops").toArray()){
                returnLayer->addLineOpacityStop(stop.toArray().first().toInt(), stop.toArray().last().toDouble());
            }
        }else{
            returnLayer->addLineColorStop(-1, paint.value("line-opacity").toString());
        }
    }

    if(paint.contains("line-width")){
        if(paint.value("line-width").isObject()){
            for(const auto &stop : paint.value("line-width").toObject().value("stops").toArray()){
                returnLayer->addLinewidthStop(stop.toArray().first().toInt(), stop.toArray().last().toInt());
            }
        }else{
            returnLayer->addLinewidthStop(-1, paint.value("line-width").toInt());
        }
    }


    return returnLayer;
}

//The following functions are getters and setters for the different background layer properties.
QColor LineLayerStyle::getLineColorAtZoom(int zoomLevel) const
{
    return getLerpedColorValue(m_lineColor, zoomLevel);
}

float LineLayerStyle::getLineOpacityAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_lineOpacity, zoomLevel);
}

float LineLayerStyle::getLineMitterLimitAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_lineMiterLimit, zoomLevel);
}

int LineLayerStyle::getLineWidthAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_lineWidth, zoomLevel);
}

void LineLayerStyle::addLineColorStop(int zoomLevel, QString hslColor)
{
    m_lineColor.append(QPair<int, QColor>(zoomLevel, getColorFromString(hslColor)));
}

void LineLayerStyle::addLineOpacityStop(int zoomLevel, float opacity)
{
    m_lineOpacity.append(QPair<int, float>(zoomLevel, opacity));
}

void LineLayerStyle::addLineMitterLimitStop(int zoomLevel, float limit)
{
    m_lineMiterLimit.append(QPair<int, float>(zoomLevel, limit));
}

void LineLayerStyle::addLinewidthStop(int zoomLevel, int width)
{
    m_lineWidth.append(QPair<int, int>(zoomLevel, width));
}

/*
 * ----------------------------------------------------------------------------
 */

/*Parses the data from a QJsonObject containing styling properties of a layer of type symbol.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type SymbolLayerStyle to the newly created layer with the parsed properties.
     */
SymbolLayerStyle *SymbolLayerStyle::fromJson(const QJsonObject &json)
{

    SymbolLayerStyle* returnLayer = new SymbolLayerStyle();
    returnLayer->m_id = json.value("id").toString();
    returnLayer->m_source = json.value("source").toString();
    returnLayer->m_sourceLayer = json.value("source-layerr").toString();
    returnLayer->m_maxZoom =  json.contains("maxzoom") ? json.value("maxzoom").toInt() : 24;
    returnLayer->m_minZoom =  json.contains("minzoom") ? json.value("minzoom").toInt() : 0;
    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    returnLayer->m_visibility = layout.contains("visibility") ? layout.value("visibility").toString() : QString("visible");

    if(layout.contains("text-size")){
        if(layout.value("text-size").isObject()){
            for(const auto &stop : layout.value("text-size").toObject().value("stops").toArray()){
                returnLayer->addTextSizeStop(stop.toArray().first().toInt(), stop.toArray().last().toInt());
            }
        }else{
            returnLayer->addTextSizeStop(-1, layout.value("text-size").toInt());
        }
    }

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("text-color")){
        if(paint.value("text-color").isObject()){
            for(const auto &stop : paint.value("text-color").toObject().value("stops").toArray()){
                returnLayer->addTextColorStop(stop.toArray().first().toInt(), stop.toArray().last().toString());
            }
        }else{
            returnLayer->addTextColorStop(-1, paint.value("text-color").toString());
        }
    }

    if(paint.contains("text-opacity")){
        if(paint.value("text-opacity").isObject()){
            for(const auto &stop : paint.value("text-opacity").toObject().value("stops").toArray()){
                returnLayer->addTextPacity(stop.toArray().first().toInt(), stop.toArray().last().toDouble());
            }
        }else{
            returnLayer->addTextPacity(-1, paint.value("text-opacity").toDouble());
        }
    }

    return returnLayer;
}


//The following functions are getters and setters for the different background layer properties.
int SymbolLayerStyle::getTextSizeAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_textSize, zoomLevel);
}

QColor SymbolLayerStyle::getTextColorAtZoom(int zoomLevel) const
{
    return getLerpedColorValue(m_textColor, zoomLevel);
}

float SymbolLayerStyle::getTextOpacityAtZoom(int zoomLevel) const
{
    return getLerpedValue(m_textOpacity, zoomLevel);
}

void SymbolLayerStyle::addTextSizeStop(int zoomLevel, int size)
{
    m_textSize.append(QPair<int, float>(zoomLevel, size));
}

void SymbolLayerStyle::addTextColorStop(int zoomLevel, QString hslColor)
{
    m_textColor.append(QPair<int, QColor>(zoomLevel, getColorFromString(hslColor)));
}

void SymbolLayerStyle::addTextPacity(int zoomLevel, float opacity)
{
    m_textOpacity.append(QPair<int, float>(zoomLevel, opacity));
}

/*
 * ----------------------------------------------------------------------------
 */
/*Parses basic properties of layerstyles that are not background, fill, line, or symbol.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type NotImplementedStyle to the newly created layer with the parsed properties.
     */
NotImplementedStyle* NotImplementedStyle::fromJson(const QJsonObject &json)
{
    NotImplementedStyle* returnLayer = new NotImplementedStyle();
    returnLayer->m_id = json.value("id").toString();
    returnLayer->m_source = json.value("source").toString();
    returnLayer->m_sourceLayer = json.value("source-layer").toString();
    return returnLayer;
}



/*
 * ----------------------------------------------------------------------------
 */


/*Passes the json object to the apropriate function to be parsed into a layerStyle object.
     *
     * Parameters:
     *     json expects a refrence to the json object containing the data.
     *
     * Returns a pointer of type AbstractLayereStyle to the newly created layer with the parsed properties.
     */
AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)
{
    QString layerType = json.value("type").toString();
    if(layerType == "background"){
        return BackgroundStyle::fromJson(json);
    }else if( layerType == "fill"){
        return FillLayerStyle::fromJson(json);
    }else if(layerType == "line"){
        return LineLayerStyle::fromJson(json);
    }else if(layerType == "symbol"){
        return SymbolLayerStyle::fromJson(json);
    }else{
        return NotImplementedStyle::fromJson(json);
    }
}


/*
 * ----------------------------------------------------------------------------
 */

//Destructor for the StyleSheet class.
StyleSheet::~StyleSheet()
{
    for(auto layer : m_layerStyles) {
        delete layer;
    }
}

/*Parce a json document by iterating throught the layers and parsing the styling properties
     *of the different layer types.
     *The function populates the m_layers list of the StyleSheet bject with pointers to
     *AbstractLayerStyle objects containing the styling information.
     *
     * Parameters:
     *     styleSheet expects a refrence to the json document containing the style sheet.
     *
     */
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
