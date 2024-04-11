#include <QRegularExpression>
#include <QtMath>

#include "LayerStyle.h"

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
    // Parsing layout properties.
    QJsonObject layout = jsonObj.value("layout").toObject();
    // Visibility property is parsed in AbstractLayerStyle::fromJson.

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
            // The initial fix came about by Googling possible solutions. See:
            //  https://forum.qt.io/topic/146125/c-11-range-loop-might-detach-qt-container-qset-clazy-range-loop-detach/2
            //
            // Copilot was then asked to update the code based on the discussion in that thread and using
            // the appropriate static cast instead of the alternate `qAsConst`:
            //  for (const auto &stop
            //       : static_cast<const QJsonArray&>(backgroundColor.toObject().value("stops").toArray()))
            //
            // This is also bad, since it may not retain all the toObject.value.toArray data correctly.
            //
            // It was finally solved by discussing it in the team and changing it to:
            QJsonArray arr = backgroundColor.toObject().value("stops").toArray();

            for (QJsonValueConstRef stop : arr) {
                int zoomStop = stop.toArray().first().toInt();
                QColor colorStop = Bach::getColorFromString(stop.toArray().last().toString());

                // Append a QPair with <zoomStop, colorStop> to `stops`.
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_backgroundColor.setValue(stops);
        } else if (backgroundColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_backgroundColor.setValue(backgroundColor.toArray());
        } else {
            // Case where the property is a color value.
            returnLayer->m_backgroundColor.setValue(Bach::getColorFromString(backgroundColor.toString()));
        }
    }

    if (paint.contains("background-opacity")){
        QJsonValue backgroundOpacity= paint.value("background-opacity");
        if (backgroundOpacity.isObject()) {
            // Case where the property is an object that has "Stops"
            QList<QPair<int, float>> stops;
            QJsonArray arr = backgroundOpacity.toObject().value("stops").toArray();

            for (QJsonValueConstRef stop : arr){
                int zoomStop = stop.toArray().first().toInt();
                float opacityStop = stop.toArray().last().toDouble();

                // Append a QPair with <zoomStop, opacityStop> to `stops`.
                stops.append(QPair<int, float>(zoomStop, opacityStop));
            }
            returnLayer->m_backgroundOpacity.setValue(stops);
        } else if (backgroundOpacity.isArray()){
            // Case where the property is an expression.
            returnLayer->m_backgroundOpacity.setValue(backgroundOpacity.toArray());
        } else {
            // Case where the property is a numeric value.
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
               && m_backgroundColor.typeId() != QMetaType::Type::QJsonArray) {
        QList<QPair<int, QColor>> stops = m_backgroundColor.value<QList<QPair<int, QColor>>>();
        if (stops.size() == 0)
            return QColor(Qt::GlobalColor::black);
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
    } else if (m_backgroundOpacity.typeId() != QMetaType::Type::Double
               && m_backgroundOpacity.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_backgroundOpacity.value<QList<QPair<int, float>>>();
        if (stops.size() == 0)
            return QVariant(1);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return m_backgroundColor;
    }
}
