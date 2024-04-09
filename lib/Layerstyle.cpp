#include <QFile>
#include <QRegularExpression>
#include <QtMath>

#include "Layerstyle.h"

/*!
 * \brief getColorFromString creates a QColor object from an HSL color string.
 *
 * The string is expected to be in one of the following formats:
 *      "hsl(hue, stauration%, lightness%)"
 *      "hsla(hue, stauration%, lightness%, alpha)"
 *
 * \param colorString is a QString containing color data.
 *
 * \return a QColor object.
 */
static QColor getColorFromString(QString colorString)
{
    colorString.remove(" ");
    //All parameters for QColor::fromHslF need to be between 0 and 1.
    if (colorString.startsWith("hsl(")) {
        static QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%\\)");
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 4) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.);
        }
    }
    if (colorString.startsWith("hsla(")) {
        static QRegularExpression re(".*\\((\\d+),(\\d+)%,(\\d+)%,(\\d?\\.?\\d*)\\)");
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 5) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.,
                                    match.capturedTexts().at(4).toFloat());
        }
    }

    /* Should there be some kind of error handling here, or this intended in the final return statement? */
    //In case the color has a different format than expected.
    return QColor::fromString(colorString);
}

/*!
 * \brief BackgroundStyle::fromJson parses QJsonObject to obtain background
 * layer style properties.
 *
 * NOTE: Copilot was used to guide updating a range-based for loop correctly.
 * here. The learnt lesson was then applied to other similar looping functions
 * further down in the code.
 *
 * The subsequent loops were updated manually by Cecilia Norevik Bratlie,
 * not by Copilot or any other AI tool.
 *
 * \param jsonObj is a reference to a QJsonObject containing the data to parse.
 * \return a pointer of type BackgroundStyle to the newly created layer with
 * the parsed properties.
 */
std::unique_ptr<BackgroundStyle> BackgroundStyle::fromJson(const QJsonObject &jsonObj)
{
    std::unique_ptr<BackgroundStyle> returnLayerPtr = std::make_unique<BackgroundStyle>();
    BackgroundStyle* returnLayer = returnLayerPtr.get();
    // Parsing layout properties
    QJsonObject layout = jsonObj.value("layout").toObject();
    // Visibility property is parsed in AbstractLayerStyle::fromJson

    // Parsing paint properties.
    QJsonObject paint = jsonObj.value("paint").toObject();
    if (paint.contains("background-color"))
    {
        QJsonValue backgroundColor = paint.value("background-color");

        if (backgroundColor.isObject()) {
            //Case where the property is an object that has "Stops".
            QList<QPair<int, QColor>> stops;

            // A previous version of the range-based loop below looked like this:
            //
            //  for(const QJsonValueRef &stop : backgroundColor.toObject().value("stops").toArray())
            //
            // This produced a warning:
            // c++11 range-loop might detach Qt container (QJsonArray) [clazy-range-loop-detach]
            //
            // The fix for that was addressed after Googling possible solutions. See:
            //  https://forum.qt.io/topic/146125/c-11-range-loop-might-detach-qt-container-qset-clazy-range-loop-detach/2
            //
            // Copilot was then asked to update the code based on the discussion in that thread and using
            // the appropriate static cast instead of the alternate `qAsConst`.
            for (const auto &stop : static_cast<const QJsonArray&>
                 (backgroundColor.toObject().value("stops").toArray()))
            {
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }

            returnLayer->m_backgroundColor.setValue(stops);

        } else if (backgroundColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_backgroundColor.setValue(backgroundColor.toArray());
        } else {
            // Case where the property is a color value
            returnLayer->m_backgroundColor.setValue(getColorFromString(backgroundColor.toString()));
        }
    }

    if (paint.contains("background-opacity")){
        QJsonValue backgroundOpacity= paint.value("background-opacity");
        if (backgroundOpacity.isObject()) {
            // Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            for (const auto &stop : static_cast<const QJsonArray&>
                 (backgroundOpacity.toObject().value("stops").toArray()))
            {
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_backgroundOpacity.setValue(stops);
        } else if (backgroundOpacity.isArray()){
            // Case where the property is an expression
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toArray());
        } else {
            // Case where the property is a numeric value
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toDouble());
        }
    }
    return returnLayerPtr;
}
/*!
 * \brief BackgroundStyle::getColorAtZoom returns the color for
 * a given zoom level.
 *
 * The QVariant will contain a QColor if the value is not an expression
 * or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the color.
 * \return a QVariant containing a QColor or QJsonArray with color information.
 */
QVariant BackgroundStyle::getColorAtZoom(int zoomLevel) const
{
    if (m_backgroundColor.isNull()){
        // The default color in case no color is provided by the style sheet.
        return QColor(Qt::GlobalColor::black);
    } else if (m_backgroundColor.typeId() != QMetaType::Type::QColor
               && m_backgroundColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_backgroundColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0) return QColor(Qt::GlobalColor::black);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_backgroundColor;
    }
}

/*!
 * \brief BackgroundStyle::getOpacityAtZoom returns the opacity for
 * a given zoom level.
 *
 * The returned value will contain a float if the value is not an expression,
 * or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the opacity.
 * \return a QVariant containing a float or QJsonArray with opacity information.
 */
QVariant BackgroundStyle::getOpacityAtZoom(int zoomLevel) const
{
    if (m_backgroundOpacity.isNull()){ 
        // The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    } else if (m_backgroundOpacity.typeId() != QMetaType::Type::Double && m_backgroundOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_backgroundOpacity.value<QList<QPair<int, float>>>();
        if (stops.size() == 0)
            return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_backgroundColor;
    }
}

/*!
 * \brief FillLayerStyle::fromJson parses JSON style data of a layer
 * with the 'fill' type.
 *
 * \param jsonObj is a QJsonObject containing the data to parse.
 *
 * \return a pointer of type FillLayerStyle to the newly created
 * layer with the parsed properties
 */
std::unique_ptr<FillLayerStyle> FillLayerStyle::fromJson(const QJsonObject &jsonObj)
{
    std::unique_ptr<FillLayerStyle> returnLayerPtr = std::make_unique<FillLayerStyle>();
    FillLayerStyle* returnLayer = returnLayerPtr.get();

   //Parsing layout properties.
    QJsonObject layout = jsonObj.value("layout").toObject();
    //visibility property is parsed in AbstractLayerStyle* AbstractLayerStyle::fromJson(const QJsonObject &json)

    //Parsing paint properties.
    QJsonObject paint = jsonObj.value("paint").toObject();

    // Get the antialiasing property from the style sheet or set it to true as a default.
    returnLayer->m_antialias = paint.contains("fill-antialias") ? paint.value("fill-antialias").toBool() : true;

    if (paint.contains("fill-color")){
        QJsonValue fillColor = paint.value("fill-color");
        if (fillColor.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, QColor>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(fillColor.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillColor.setValue(stops);
        }else if (fillColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_fillColor.setValue(fillColor.toArray());
        }else {
            // Case where the property is a color value.
            returnLayer->m_fillColor.setValue(getColorFromString(fillColor.toString()));
        }
    }

    if (paint.contains("fill-opacity")){
        QJsonValue fillOpacity = paint.value("fill-opacity");
        if (fillOpacity.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, float>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(fillOpacity.toObject().value("stops").toArray())){
                int zoomSopt = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomSopt, opacityStop));
            }
            returnLayer->m_fillOpacity.setValue(stops);
        } else if (fillOpacity.isArray()){
            // Case where the property is an expression.
            returnLayer->m_fillOpacity.setValue(fillOpacity.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_fillOpacity.setValue(fillOpacity.toDouble());
        }
    }

    if (paint.contains("fill-outline-color")){
        QJsonValue fillOutlineColor = paint.value("fill-outline-color");
        if (fillOutlineColor.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, QColor>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(fillOutlineColor.toObject().value("stops").toArray())){
                /* BEWARE!*/
                // This could potentially be a bug. Talk to Eimen to see what to do here.
                /* BEWARE*/
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillOutlineColor.setValue(stops);
        } else if (fillOutlineColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_fillOutlineColor.setValue(fillOutlineColor.toArray());
        } else {
            //Case where the property is a color value.
            returnLayer->m_fillOutlineColor.setValue(getColorFromString(fillOutlineColor.toString()));
        }
    }

    return returnLayerPtr;
}


/*!
 * \brief FillLayerStyle::getFillColorAtZoom returns the color for a given zoom.
 *
 * The QVariant will contain a QColor if the value is not an expression,
 *  or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the color.
 * \return a QVariant containing either a QColor or a QJsonArray with the data.
 */
QVariant FillLayerStyle::getFillColorAtZoom(int zoomLevel) const
{
    if (m_fillColor.isNull()){
        // The default color in case no color is provided by the style sheet.
        return QColor(Qt::GlobalColor::black);
    } else if (m_fillColor.typeId() != QMetaType::Type::QColor
               && m_fillColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_fillColor.value<QList<QPair<int, QColor>>>();
        if (stops.size() == 0)
            return QColor(Qt::GlobalColor::black);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_fillColor;
    }
}

/*!
 * \brief FillLayerStyle::getFillOpacityAtZoom returns the opacity for the passed zoom.
 *
 * \param zoomLevel is the zoom level for which to calculate the opacity.
 * \return a QVariant containing either a float or a QJsonArray with the layer opacity information.
 */
QVariant FillLayerStyle::getFillOpacityAtZoom(int zoomLevel) const
{
    if (m_fillOpacity.isNull()){
        // The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    } else if (m_fillOpacity.typeId() != QMetaType::Type::QColor
               && m_fillOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_fillOpacity.value<QList<QPair<int, float>>>();
        if (stops.size() == 0) {
            return QVariant(1);
        }
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_fillOpacity;
    }
}

/*!
 * \brief FillLayerStyle::getFillOutLineColorAtZoom returns the outline color
 * for the given zoom level.
 *
 * The returned QVariant will contain a QColor if the value is not an
 * expression, or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the color.
 * \return a QVariant containing either a Qcolor or a QJsonArray with color data.
 */
QVariant FillLayerStyle::getFillOutLineColorAtZoom(int zoomLevel) const
{
    if (m_antialias == false)
        //The outline requires the antialising to be true.
        return QVariant();
    if (m_fillOutlineColor.isNull())
        // No default value is specified for outline color.
        return QVariant();

    if (m_fillOutlineColor.typeId() != QMetaType::Type::QColor
        && m_fillOutlineColor.typeId() != QMetaType::Type::QJsonArray) {
        QList<QPair<int, QColor>> stops = m_fillOutlineColor.value<QList<QPair<int, QColor>>>();
        if (stops.size() == 0)
            return QVariant();
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_fillOutlineColor;
    }
}

/*!
 * \brief LineLayerStyle::fromJson parses data from a QJsonObject
 * with styling properties of a 'line' type layer.
 *
 * \param jsonObj is the JSON object containing layer style data.
 * \return a pointer of type LineLayerStyle to the newly created layer.
 */
std::unique_ptr<LineLayerStyle> LineLayerStyle::fromJson(const QJsonObject &jsonObj)
{
    std::unique_ptr<LineLayerStyle> returnLayerPtr = std::make_unique<LineLayerStyle>();
    LineLayerStyle* returnLayer = returnLayerPtr.get();
// Parsing layout properties.
    // Visibility property is parsed in:
    // AbstractLayerStyle* AbstractLayerStyle::fromJson(const QJsonObject &jsonObj)
    QJsonObject layout = jsonObj.value("layout").toObject();
    returnLayer->m_lineCap = layout.contains("line-cap")
                                 ? layout.value("line-cap").toString() : QString("butt");
    returnLayer->m_lineJoin = layout.contains("line-join")
                                  ? layout.value("line-join").toString() : QString("miter");
    // Parsing paint properties.
    QJsonObject paint = jsonObj.value("paint").toObject();
    if (paint.contains("line-dasharray")){
        for (const auto &length
             : static_cast<const QJsonArray&>(paint.value("line-dasharray").toArray())){
            returnLayer->m_lineDashArray.append(length.toInt());
        }
    }

    if (paint.contains("line-color")){
        QJsonValue lineColor = paint.value("line-color");
        if (lineColor.isObject()){
            //Case where the property is an object that has "Stops".
            QList<QPair<int, QColor>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(lineColor.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int , QColor>(zoomStop, colorStop));
            }
            returnLayer->m_lineColor.setValue(stops);
        } else if (lineColor.isArray()){
            //Case where the property is an expression.
            returnLayer->m_lineColor.setValue(lineColor.toArray());
        } else {
            //Case where the property is a color value.
            returnLayer->m_lineColor.setValue(getColorFromString(lineColor.toString()));
        }
    }

    if (paint.contains("line-opacity")){
        QJsonValue lineOpacity = paint.value("line-opacity");
        if (lineOpacity.isObject()){ 
            // Case where the property is an object that has "Stops".
            QList<QPair<int, float>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(lineOpacity.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int , float>(zoomStop, opacityStop));
            }
            returnLayer->m_lineOpacity.setValue(stops);
        }else if (lineOpacity.isArray()){
            // Case where the property is an expression.
            returnLayer->m_lineOpacity.setValue(lineOpacity.toArray());
        } else { //Case where the property is a numeric value.
            returnLayer->m_lineOpacity.setValue(lineOpacity.toDouble());
        }
    }

    if(paint.contains("line-width")){
        QJsonValue lineWidth = paint.value("line-width");
        if(lineWidth.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, int>> stops;
            for (const auto &stop :
                 static_cast<const QJsonArray&>(lineWidth.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                int widthStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, widthStop));
            }
            returnLayer->m_lineWidth.setValue(stops);
        } else if (lineWidth.isArray()){
            // Case where the property is an expression.
            returnLayer->m_lineWidth.setValue(lineWidth.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_lineWidth.setValue(lineWidth.toInt());
        }
    }

    return returnLayerPtr;
}
/*!
 * \brief LineLayerStyle::getLineColorAtZoom returns the color for a given zoom.
 *
 * The returned data will contain a QColor if the value is not an expression,
 *  or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the color.
 * \return a QVariant containing either a QColor or a QJsonArray with data.
 */
QVariant LineLayerStyle::getLineColorAtZoom(int zoomLevel) const
{
    if (m_lineColor.isNull()){
        // The default color in case no color is provided by the style sheet.
        return QVariant(QColor(Qt::GlobalColor::black));
    } else if (m_lineColor.typeId() != QMetaType::Type::QColor
               && m_lineColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_lineColor.value<QList<QPair<int, QColor>>>();
        if(stops.size() == 0)
            return QVariant(QColor(Qt::GlobalColor::black));
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_lineColor;
    }
}

/*!
 * \brief LineLayerStyle::getLineOpacityAtZoom returns the opacity for
 * a given zoom level.
 *
 * The QVariant will contain a float if the value is not an expression,
 * or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the opacity.
 * \return the opacity for the given zoom level.
 */
QVariant LineLayerStyle::getLineOpacityAtZoom(int zoomLevel) const
{
    if (m_lineOpacity.isNull()){
        // The default opacity in case no opacity is provided by the style sheet.
        return QVariant(1);
    } else if (m_lineOpacity.typeId() != QMetaType::Type::Double
               && m_lineOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_lineOpacity.value<QList<QPair<int, float>>>();
        if (stops.size() == 0)
            return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_lineOpacity;
    }
}

/*!
 * \brief LineLayerStyle::getLineWidthAtZoom returns line width for a given zoom level.
 *
 * The QVariant will contain a float if the value is not an expression,
 * or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the line width.
 * \return a QVariant containing either a float or a QJsonArray with the
 */
QVariant LineLayerStyle::getLineWidthAtZoom(int zoomLevel) const
{
    if (m_lineWidth.isNull()){
        // The default width in case no width is provided by the style sheet.
        return QVariant(1);
    } else if (m_lineWidth.typeId() != QMetaType::Type::Int
               && m_lineWidth.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_lineWidth.value<QList<QPair<int, int>>>();
        if (stops.size() == 0)
            return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_lineWidth;
    }
}

/*!
 * \brief LineLayerStyle::getJoinStyle gets the join style of a pen.
 *
 * The style is determined by a QString containing "bevel" or "miter",
 * otherwise it'll be set to Qt::PenJoinStyle::RoundJoin.
 *
 * \return the pen join style.
 */
Qt::PenJoinStyle LineLayerStyle::getJoinStyle() const
{
    if (m_lineJoin == "bevel"){
        return Qt::PenJoinStyle::BevelJoin;
    } else if (m_lineJoin == "miter"){
        return Qt::PenJoinStyle::MiterJoin;
    } else {
        return Qt::PenJoinStyle::RoundJoin;
    }
}

/*!
 * \brief LineLayerStyle::getCapStyle gets the cap style of a pen.
 *
 * The style is determined by a QString containing "butt" or "round",
 * otherwise it'll be set to Qt::PenCapStyle::SquareCap.
 * \return the pen cap style.
 */
Qt::PenCapStyle LineLayerStyle::getCapStyle() const
{
    if (m_lineCap == "butt")
        return Qt::PenCapStyle::FlatCap;
     else if (m_lineCap == "round")
        return Qt::PenCapStyle::RoundCap;
     else
        return Qt::PenCapStyle::SquareCap;
}

/*!
 * \brief SymbolLayerStyle::fromJson parses style properties of a
 * layer of type 'symbol'.
 *
 * \param jsonObj is the JSON object containing layer style data.
 * \return a pointer of type SymbolLayerStyle to the newly created layer
 */
std::unique_ptr<SymbolLayerStyle> SymbolLayerStyle::fromJson(const QJsonObject &jsonObj)
{
    std::unique_ptr<SymbolLayerStyle> returnLayerPtr = std::make_unique<SymbolLayerStyle>();
    SymbolLayerStyle *returnLayer = returnLayerPtr.get();

    // Parsing layout properties.
    QJsonObject layout = jsonObj.value("layout").toObject();
    // Visibility property is parsed in AbstractLayerStyle* AbstractLayerStyle::fromJson(const QJsonObject &json)

    if (layout.contains("text-size")){
        QJsonValue textSize = layout.value("text-size");
        if (textSize.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, int>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(textSize.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                int sizeStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, sizeStop));
            }
            returnLayer->m_textSize.setValue(stops);
        } else if (textSize.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textSize.setValue(textSize.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_textSize.setValue(textSize.toInt());
        }
    }

    if (layout.contains("text-font")){
        returnLayer->m_textFont = layout.value("text-font").toVariant().toStringList();
    } else {
        // Default value for the font if no value was provided.
        returnLayer->m_textFont = {"Open Sans Regular","Arial Unicode MS Regular"};
    }

    if (layout.contains("text-field")){
        if (layout.value("text-field").isArray()){ 
            // Case where the property is an expression.
            returnLayer->m_textField = QVariant(layout.value("text-field").toArray());
        } else {
            // Case where the property is a string value.
            returnLayer->m_textField = QVariant(layout.value("text-field").toString());
        }
    }

    if (layout.contains("text-max-width")){
        returnLayer->m_textMaxWidth = layout.value("text-max-width").toInt();
    } else {
        returnLayer->m_textMaxWidth = 10;
    }
    // Parsing paint properties.
    QJsonObject paint = jsonObj.value("paint").toObject();
    if (paint.contains("text-color")){
        QJsonValue textColor = paint.value("text-color");
        if (textColor.isObject()){ 
            // Case where the property is an object that has "Stops".
            QList<QPair<int, QColor>> stops;
            for(const auto &stop
                 : static_cast<const QJsonArray&>(textColor.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_textColor.setValue(stops);
        }else if (textColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textColor.setValue(textColor.toArray());
        } else {
            // Case where the property is a color value.
            returnLayer->m_textColor.setValue(getColorFromString(textColor.toString()));
        }
    }

    if (paint.contains("text-opacity")){
        QJsonValue textOpacity = paint.value("text-opacity");
        if (textOpacity.isObject()){ 
            // Case where the property is an object that has "Stops".
            QList<QPair<int, float>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(textOpacity.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_textOpacity.setValue(stops);
        } else if (textOpacity.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textOpacity.setValue(textOpacity.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_textOpacity.setValue(textOpacity.toDouble());
        }
    }

    if (paint.contains("text-halo-color")){
        QColor haloColor = getColorFromString(paint.value("text-halo-color").toString());
        returnLayer->m_textHaloColor = haloColor;
    } else {
        returnLayer->m_textHaloColor = QColor(Qt::GlobalColor::black);
    }

    if (paint.contains("text-halo-width")){
        returnLayer->m_textHaloWidth =paint.value("text-halo-width").toInt();
    } else {
        returnLayer->m_textHaloWidth = 0;
    }

    return returnLayerPtr;
}

/*!
 * \brief SymbolLayerStyle::getTextSizeAtZoom returns the text size for a given zoom.
 *
 * The returned QVariant will contain a float if the value is not an expression,
 *  or a QJsonArray otherwise
 *
 * \param zoomLevel is the zoom level for which to calculate the size.
 * \return a QVariant containing either a float or a QJsonArray with data.
 */
QVariant SymbolLayerStyle::getTextSizeAtZoom(int zoomLevel) const
{
    if (m_textSize.isNull()){
        // The default size in case no size is provided by the style sheet.
        return QVariant(16);
    } else if (m_textSize.typeId() != QMetaType::Type::Double
               && m_textSize.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_textSize.value<QList<QPair<int, int>>>();
        if (stops.size() == 0)
            return QVariant(16);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textSize);
    }
}

/*!
 * \brief SymbolLayerStyle::getTextColorAtZoom returns the color for a given zoom level.
 *
 * \param zoomLevel is the zoom level for which to calculate the color.
 * \return a QVariant with either QColor or a QJsonArray with data.
 */
QVariant SymbolLayerStyle::getTextColorAtZoom(int zoomLevel) const
{
    if(m_textColor.isNull()){
        // The default color in case no color is provided by the style sheet.
        return QVariant(QColor(Qt::GlobalColor::black));
    } else if (m_textColor.typeId() != QMetaType::Type::QColor && m_textColor.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, QColor>> stops = m_textColor.value<QList<QPair<int, QColor>>>();
        if (stops.size() == 0)
            return QVariant(QColor(Qt::GlobalColor::black));
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textColor);
    }
}

/*!
 * \brief SymbolLayerStyle::getTextOpacityAtZoom returns the opacity for
 * a given zoom level.
 *
 * The returned QVariant will contain a float if the value is not an expression,
 * or a QJsonArray otherwise.
 *
 * \param zoomLevel is the zoom level for which to calculate the opacity.
 * \return a QVariant containing either a float or a QJsonArray with the data.
 */
QVariant SymbolLayerStyle::getTextOpacityAtZoom(int zoomLevel) const
{
    if (m_textOpacity.isNull()){ 
        // The default color in case no color is provided by the style sheet.
        return QVariant(1);
    } else if (m_textOpacity.typeId() != QMetaType::Type::Double 
                && m_textOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_textOpacity.value<QList<QPair<int, float>>>();
        if (stops.size() == 0) return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textOpacity);
    }
}

/*!
 * \brief NotImplementedStyle::fromJson returns the NotImplementedStyle
 * layer style.
 *
 * This is used for any layer with type value other than "background",
 * "fill", "line", or "symbol".
 *
 * \param jsonObj is a JSonObject. It's passed here to match Layer style types.
 * \return the NotImplementedStyle.
 */
std::unique_ptr<NotImplementedStyle> NotImplementedStyle::fromJson(const QJsonObject &json)
{
    std::unique_ptr<NotImplementedStyle> returnLayerPtr = std::make_unique<NotImplementedStyle>();
    NotImplementedStyle* returnLayer = new NotImplementedStyle();
    return returnLayerPtr;
}

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

std::optional<StyleSheet> StyleSheet::fromJsonBytes(const QByteArray& input)
{
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(input, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return std::nullopt;
    }
    return fromJson(jsonDoc);
}

/*!
 * \brief fromJsonFile
 * Load a StyleSheet from a JSON file.
 *
 * Uses QFile internally.
 *
 * \param path Path to the file.
 *
 * \return Returns the StyleSheet object if successful. Returns null if not.
 */
std::optional<StyleSheet> StyleSheet::fromJsonFile(const QString& path)
{
    QFile file{ path };
    bool openSuccess = file.open(QFile::ReadOnly);
    if (!openSuccess) {
        return std::nullopt;
    }
    return fromJsonBytes(file.readAll());
}
