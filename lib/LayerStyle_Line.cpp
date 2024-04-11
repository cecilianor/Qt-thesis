#include <QRegularExpression>
#include <QtMath>

#include "LayerStyle.h"

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
        QJsonArray arr = paint.value("line-dasharray").toArray();

        for (QJsonValueConstRef length : arr){
            returnLayer->m_lineDashArray.append(length.toInt());
        }
    }

    if (paint.contains("line-color")){
        QJsonValue lineColor = paint.value("line-color");
        if (lineColor.isObject()){
            //Case where the property is an object that has "stops".
            QList<QPair<int, QColor>> stops;
            QJsonArray arr = lineColor.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, colorStop> 
            // data to `stops`.
            for (QJsonValueConstRef stop : arr) {
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = Bach::getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int , QColor>(zoomStop, colorStop));
            }
            returnLayer->m_lineColor.setValue(stops);
        } else if (lineColor.isArray()){
            //Case where the property is an expression.
            returnLayer->m_lineColor.setValue(lineColor.toArray());
        } else {
            //Case where the property is a color value.
            returnLayer->m_lineColor.setValue(Bach::getColorFromString(lineColor.toString()));
        }
    }

    if (paint.contains("line-opacity")){
        QJsonValue lineOpacity = paint.value("line-opacity");
        if (lineOpacity.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, float>> stops;
            QJsonArray arr = lineOpacity.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, opacityStop>
            // data to `stops`.
            for (QJsonValueConstRef stop : arr) {
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
            QJsonArray arr = lineWidth.toObject().value("stops").toArray();

            // Loop over all stops and append a pair of <zoomStop, widthStop>
            // data to `stops`.
            for (QJsonValueConstRef stop : arr) {
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
