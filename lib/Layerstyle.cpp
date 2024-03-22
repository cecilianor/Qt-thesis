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
    //All parameters for QColor::fromHslF need to be between 0 and 1.
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
    //In case the color has a different format than expected.
    return QColor::fromString(colorString);
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
    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle::fromJson

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("background-color"))
    {
        QJsonValue backgroundColor = paint.value("background-color");
        if(backgroundColor.isObject()) { //Case where the property is an object that has "Stops"
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : backgroundColor.toObject().value("stops").toArray())
            {
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_backgroundColor.setValue(stops);
        }else if(backgroundColor.isArray()){ //Case where the property is an expression
            returnLayer->m_backgroundColor.setValue(backgroundColor.toArray());
        }else{ //Case where the property is a color value
            returnLayer->m_backgroundColor.setValue(getColorFromString(backgroundColor.toString()));
        }

    }

    if(paint.contains("background-opacity")){
        QJsonValue backgroundOpacity= paint.value("background-opacity");
        if(backgroundOpacity.isObject()) { //Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            for(const auto &stop : backgroundOpacity.toObject().value("stops").toArray())
            {
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_backgroundOpacity.setValue(stops);
        }else if(backgroundOpacity.isArray()){ //Case where the property is an expression
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toDouble());
        }
    }

    return returnLayer;
}

/*Returns the color for the sepcific zoom. The QVaraiant will contain a QColor if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the color.
     *
     * Returns a QVariant containing either a QColor or a QJsonArray with the information.
     */
QVariant BackgroundStyle::getColorAtZoom(int zoomLevel) const
{
    if(m_backgroundColor.isNull()){ // The default color in case no color is provided by the style sheet.
        return QColor(Qt::GlobalColor::black);
    }else if(m_backgroundColor.typeId() != QMetaType::Type::QColor && m_backgroundColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_backgroundColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QColor(Qt::GlobalColor::black);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_backgroundColor;
    }
}

/*Returns the opacity for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the opacity.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the opacity information.
     */
QVariant BackgroundStyle::getOpacityAtZoom(int zoomLevel) const
{
    if(m_backgroundOpacity.isNull()){ // The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    }else if(m_backgroundOpacity.typeId() != QMetaType::Type::Double && m_backgroundOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_backgroundOpacity.value<QList<QPair<int, float>>>();
        if(stops.size() == 0) return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_backgroundColor;
    }
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

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)

    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    //Get the antialiasing property from the style sheet or set it to true as a default.
    returnLayer->m_antialias = paint.contains("fill-antialias") ? paint.value("fill-antialias").toBool() : true;
    if(paint.contains("fill-color")){
        QJsonValue fillColor = paint.value("fill-color");
        if(fillColor.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : fillColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillColor.setValue(stops);
        }else if(fillColor.isArray()){ //Case where the property is an expression
            returnLayer->m_fillColor.setValue(fillColor.toArray());
        }else{ //Case where the property is a color value
            returnLayer->m_fillColor.setValue(getColorFromString(fillColor.toString()));
        }
    }

    if(paint.contains("fill-opacity")){
        QJsonValue fillOpacity = paint.value("fill-opacity");
        if(fillOpacity.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            for(const auto &stop : fillOpacity.toObject().value("stops").toArray()){
                int zoomSopt = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomSopt, opacityStop));
            }
            returnLayer->m_fillOpacity.setValue(stops);
        }else if(fillOpacity.isArray()){ //Case where the property is an expression
            returnLayer->m_fillOpacity.setValue(fillOpacity.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_fillOpacity.setValue(fillOpacity.toDouble());
        }
    }

    if(paint.contains("fill-outline-color")){
        QJsonValue fillOutlineColor = paint.value("fill-outline-color");
        if(fillOutlineColor.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : fillOutlineColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
            }
            returnLayer->m_fillOutlineColor.setValue(stops);
        }else if(fillOutlineColor.isArray()){ //Case where the property is an expression
            returnLayer->m_fillOutlineColor.setValue(fillOutlineColor.toArray());
        }else{ //Case where the property is a color value
            returnLayer->m_fillOutlineColor.setValue(getColorFromString(fillOutlineColor.toString()));
        }
    }

    return returnLayer;
}


/*Returns the color for the sepcific zoom. The QVaraiant will contain a QColor if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the color.
     *
     * Returns a QVariant containing either a QColor or a QJsonArray with the information.
     */
QVariant FillLayerStyle::getFillColorAtZoom(int zoomLevel) const
{
    if(m_fillColor.isNull()){ // The default color in case no color is provided by the style sheet.
        return QColor(Qt::GlobalColor::black);
    }else if(m_fillColor.typeId() != QMetaType::Type::QColor && m_fillColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_fillColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QColor(Qt::GlobalColor::black);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_fillColor;
    }
}


/*Returns the opacity for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the opacity.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the information.
     */
QVariant FillLayerStyle::getFillOpacityAtZoom(int zoomLevel) const
{
    if(m_fillOpacity.isNull()){// The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    }else if(m_fillOpacity.typeId() != QMetaType::Type::QColor && m_fillOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_fillOpacity.value<QList<QPair<int, float>>>();
        if(stops.size() == 0) return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_fillOpacity;
    }
}


/*Returns the outline color for the sepcific zoom. The QVaraiant will contain a QColor if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the color.
     *
     * Returns a QVariant containing either a Qcolor or a QJsonArray with the information.
     */
QVariant FillLayerStyle::getFillOutLineColorAtZoom(int zoomLevel) const
{
    if(m_antialias == false) return QVariant(); //The outline requires the antialising to be true.
    if(m_fillOutlineColor.isNull()){ // No default value is specified for outline color.
        return QVariant();
    }
    if(m_fillOutlineColor.typeId() != QMetaType::Type::QColor && m_fillOutlineColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_fillOutlineColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QVariant();
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_fillOutlineColor;
    }
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
        if(lineColor.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : lineColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int , QColor>(zoomStop, colorStop));
            }
            returnLayer->m_lineColor.setValue(stops);
        }else if(lineColor.isArray()){ //Case where the property is an expression
            returnLayer->m_lineColor.setValue(lineColor.toArray());
        }else{ //Case where the property is a color value
            returnLayer->m_lineColor.setValue(getColorFromString(lineColor.toString()));
        }
    }

    if(paint.contains("line-opacity")){
        QJsonValue lineOpacity = paint.value("line-opacity");
        if(lineOpacity.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            for(const auto &stop : lineOpacity.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int , float>(zoomStop, opacityStop));
            }
            returnLayer->m_lineOpacity.setValue(stops);
        }else if(lineOpacity.isArray()){ //Case where the property is an expression
            returnLayer->m_lineOpacity.setValue(lineOpacity.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_lineOpacity.setValue(lineOpacity.toDouble());
        }
    }

    if(paint.contains("line-width")){
        QJsonValue lineWidth = paint.value("line-width");
        if(lineWidth.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, int>> stops;
            for(const auto &stop : lineWidth.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                int widthStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, widthStop));
            }
            returnLayer->m_lineWidth.setValue(stops);
        }else if(lineWidth.isArray()){ //Case where the property is an expression
            returnLayer->m_lineWidth.setValue(lineWidth.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_lineWidth.setValue(lineWidth.toInt());
        }
    }


    return returnLayer;
}

/*Returns the color for the sepcific zoom. The QVaraiant will contain a QColor if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the color.
     *
     * Returns a QVariant containing either a QColor or a QJsonArray with the information.
     */
QVariant LineLayerStyle::getLineColorAtZoom(int zoomLevel) const
{
    if(m_lineColor.isNull()){ // The default color in case no color is provided by the style sheet.
        return QVariant(QColor(Qt::GlobalColor::black));
    }else if(m_lineColor.typeId() != QMetaType::Type::QColor && m_lineColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_lineColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QVariant(QColor(Qt::GlobalColor::black));
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_lineColor;
    }
}

/*Returns the opacity for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the opacity.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the information.
     */
QVariant LineLayerStyle::getLineOpacityAtZoom(int zoomLevel) const
{
    if(m_lineOpacity.isNull()){ // The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    }else if(m_lineOpacity.typeId() != QMetaType::Type::Double && m_lineOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_lineOpacity.value<QList<QPair<int, float>>>();
        if(stops.size() == 0) return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return m_lineOpacity;
    }
}

/*Returns the line width for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the width.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the information.
     */

QVariant LineLayerStyle::getLineWidthAtZoom(int zoomLevel) const
{
    if(m_lineWidth.isNull()){ // The default width in case no width is provided by the style sheet.
        return QVariant(1);
    }else if(m_lineWidth.typeId() != QMetaType::Type::Int && m_lineWidth.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_lineWidth.value<QList<QPair<int, int>>>();
        if(stops.size() == 0) return QVariant(1);
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

    //parsing layout properties
    QJsonObject layout = json.value("layout").toObject();
    //visibility property is parsed in AbstractLayereStyle* AbstractLayereStyle::fromJson(const QJsonObject &json)

    if(layout.contains("text-size")){
        QJsonValue textSize = layout.value("text-size");
        if(textSize.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, int>> stops;
            for(const auto &stop : textSize.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                int sizeStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, sizeStop));
            }
            returnLayer->m_textSize.setValue(stops);
        }else if(textSize.isArray()){ //Case where the property is an expression
            returnLayer->m_textSize.setValue(textSize.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_textSize.setValue(textSize.toInt());
        }
    }

    if(layout.contains("text-font")){
        returnLayer->m_textFont = layout.value("text-font").toVariant().toStringList();
    }else{ // Default value for the font if no value was provided
        returnLayer->m_textFont = {"Open Sans Regular","Arial Unicode MS Regular"};
    }

    if(layout.contains("text-field")){
        if(layout.value("text-field").isArray()){ //Case where the property is an expression
            returnLayer->m_textField = QVariant(layout.value("text-field").toArray());
        }else{ //Case where the property is a string value
            returnLayer->m_textField = QVariant(layout.value("text-field").toString());
        }

    }

    if(layout.contains("text-max-width")){
        returnLayer->m_textMaxWidth = layout.value("text-max-width").toInt();
    }else{
        returnLayer->m_textMaxWidth = 10;
    }
    //parsing paint properties
    QJsonObject paint = json.value("paint").toObject();
    if(paint.contains("text-color")){
        QJsonValue textColor = paint.value("text-color");
        if(textColor.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, QColor>> stops;
            for(const auto &stop : textColor.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_textColor.setValue(stops);
        }else if(textColor.isArray()){ //Case where the property is an expression
            returnLayer->m_textColor.setValue(textColor.toArray());
        }else{ //Case where the property is a color value
            returnLayer->m_textColor.setValue(getColorFromString(textColor.toString()));
        }
    }

    if(paint.contains("text-opacity")){
        QJsonValue textOpacity = paint.value("text-opacity");
        if(textOpacity.isObject()){ //Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            for(const auto &stop : textOpacity.toObject().value("stops").toArray()){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_textOpacity.setValue(stops);
        }else if(textOpacity.isArray()){ //Case where the property is an expression
            returnLayer->m_textOpacity.setValue(textOpacity.toArray());
        }else{ //Case where the property is a numeric value
            returnLayer->m_textOpacity.setValue(textOpacity.toDouble());
        }
    }

    if(paint.contains("text-halo-color")){
        QColor haloColor = getColorFromString(paint.value("text-halo-color").toString());
        returnLayer->m_textHaloColor = haloColor;
    }else{
        returnLayer->m_textHaloColor = QColor(Qt::GlobalColor::black);
    }

    if(paint.contains("text-halo-width")){
        returnLayer->m_textHaloWidth =paint.value("text-halo-width").toInt();
    }else{
        returnLayer->m_textHaloWidth = 0;
    }

    return returnLayer;
}


/*Returns the text size for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the size.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the information.
     */

QVariant SymbolLayerStyle::getTextSizeAtZoom(int zoomLevel) const
{

    if(m_textSize.isNull()){ // The default size in case no size is provided by the style sheet.
        return QVariant(16);
    }else if(m_textSize.typeId() != QMetaType::Type::Double && m_textSize.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_textSize.value<QList<QPair<int, int>>>();
        if(stops.size() == 0) return QVariant(16);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textSize);
    }
}

/*Returns the color for the sepcific zoom. The QVaraiant will contain a QColor if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the color.
     *
     * Returns a QVariant containing either a QColor or a QJsonArray with the information.
     */
QVariant SymbolLayerStyle::getTextColorAtZoom(int zoomLevel) const
{
    if(m_textColor.isNull()){ // The default color in case no color is provided by the style sheet.
        return QVariant(QColor(Qt::GlobalColor::black));
    }else if(m_textColor.typeId() != QMetaType::Type::QColor && m_textColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_textColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QVariant(QColor(Qt::GlobalColor::black));
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textColor);
    }
}


/*Returns the opacity for the sepcific zoom. The QVaraiant will contain a float if
     * the value is not an expression or a QJsonArray otherwise.
     *
     * Parameters:
     *     zoomLevel the zoom level for which to calculate the opacity.
     *
     * Returns a QVariant containing either a float or a QJsonArray with the information.
     */

QVariant SymbolLayerStyle::getTextOpacityAtZoom(int zoomLevel) const
{
    if(m_textOpacity.isNull()){ // The default color in case no color is provided by the style sheet.
        return QVariant(1);
    }else if(m_textOpacity.typeId() != QMetaType::Type::Double && m_textOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_textOpacity.value<QList<QPair<int, float>>>();
        if(stops.size() == 0) return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    }else{
        return QVariant(m_textOpacity);
    }
}

/*
 * ----------------------------------------------------------------------------
 */


//For any layer with type value other than "background", "fill", "line", or "symbol".
NotImplementedStyle* NotImplementedStyle::fromJson(const QJsonObject &json)
{
    NotImplementedStyle* returnLayer = new NotImplementedStyle();
    return returnLayer;
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

/*Parses the entire style sheet and populates the layers array of the StyleSheet object.
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

std::optional<StyleSheet> StyleSheet::fromJson(const QJsonDocument& input)
{
    StyleSheet out;
    out.parseSheet(input);
    return out;
}

