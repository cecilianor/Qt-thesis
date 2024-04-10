#include <QRegularExpression>
#include <QtMath>

#include "LayerStyle.h"

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
    //visibility property is parsed in:
    // AbstractLayerStyle* AbstractLayerStyle::fromJson(const QJsonObject &jsonObj)

    //Parsing paint properties.
    QJsonObject paint = jsonObj.value("paint").toObject();

    // Get the antialiasing property from the style sheet or set it to true as a default.
    returnLayer->m_antialias = paint.contains("fill-antialias")
                                   ? paint.value("fill-antialias").toBool() : true;

    if (paint.contains("fill-color")){
        QJsonValue fillColor = paint.value("fill-color");
        if (fillColor.isObject()){
            // Case where the property is an object that has "stops".
            QList<QPair<int, QColor>> stops;
            QJsonArray arr = fillColor.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, colorStop> data to `stops`.
            for (QJsonValueConstRef stop : arr){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = Bach::getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillColor.setValue(stops);
        }else if (fillColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_fillColor.setValue(fillColor.toArray());
        }else {
            // Case where the property is a color value.
            returnLayer->m_fillColor.setValue(Bach::getColorFromString(fillColor.toString()));
        }
    }

    if (paint.contains("fill-opacity")){
        QJsonValue fillOpacity = paint.value("fill-opacity");
        if (fillOpacity.isObject()){
            // Case where the property is an object that has "stops".
            QList<QPair<int, float>> stops;
            QJsonArray arr = fillOpacity.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, opacityStop> to `stops`.
            for (QJsonValueConstRef stop : arr){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();
                stops.append(QPair<int, float>(zoomStop, opacityStop));
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
            // Case where the property is an object that has "stops".
            QList<QPair<int, QColor>> stops;
            QJsonArray arr =fillOutlineColor.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, colorStop> to `stops`.
            for (QJsonValueConstRef stop: arr){
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = Bach::getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_fillOutlineColor.setValue(stops);
        } else if (fillOutlineColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_fillOutlineColor.setValue(fillOutlineColor.toArray());
        } else {
            //Case where the property is a color value.
            returnLayer->m_fillOutlineColor.setValue(Bach::getColorFromString(fillOutlineColor.toString()));
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
