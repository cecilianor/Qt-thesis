#include <QRegularExpression>
#include <QtMath>

#include "Layerstyle.h"


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


    if (layout.contains("text-max-angle")){
        QJsonValue textSize = layout.value("text-max-angle");
        if (textSize.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, int>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(textSize.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                int angleStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, angleStop));
            }
            returnLayer->m_textMaxAngle.setValue(stops);
        } else if (textSize.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textMaxAngle.setValue(textSize.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_textMaxAngle.setValue(textSize.toInt());
        }
    }


    if (layout.contains("symbol-spacing")){
        QJsonValue textSize = layout.value("symbol-spacing");
        if (textSize.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, int>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(textSize.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                int sizeStop = stop.toArray().last().toInt();
                stops.append(QPair<int, int>(zoomStop, sizeStop));
            }
            returnLayer->m_symbolSpacing.setValue(stops);
        } else if (textSize.isArray()){
            // Case where the property is an expression.
            returnLayer->m_symbolSpacing.setValue(textSize.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_symbolSpacing.setValue(textSize.toInt());
        }
    }

    if (layout.contains("text-letter-spacing")){
        QJsonValue textSize = layout.value("text-letter-spacing");
        if (textSize.isObject()){
            // Case where the property is an object that has "Stops".
            QList<QPair<int, float>> stops;
            for (const auto &stop
                 : static_cast<const QJsonArray&>(textSize.toObject().value("stops").toArray())){
                int zoomStop = stop.toArray().first().toInt();
                float spaceStop = stop.toArray().last().toInt();
                stops.append(QPair<int, float>(zoomStop, spaceStop));
            }
            returnLayer->m_textLetterSpacing.setValue(stops);
        } else if (textSize.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textLetterSpacing.setValue(textSize.toArray());
        } else {
            // Case where the property is a numeric value.
            returnLayer->m_textLetterSpacing.setValue(textSize.toDouble());
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
                QColor colorStop = Bach::getColorFromString(stop.toArray().last().toString());
                stops.append(QPair<int, QColor>(zoomStop, colorStop));
            }
            returnLayer->m_textColor.setValue(stops);
        }else if (textColor.isArray()){
            // Case where the property is an expression.
            returnLayer->m_textColor.setValue(textColor.toArray());
        } else {
            // Case where the property is a color value.
            returnLayer->m_textColor.setValue(Bach::getColorFromString(textColor.toString()));
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
        QColor haloColor = Bach::getColorFromString(paint.value("text-halo-color").toString());
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
    } else if (m_textSize.typeId() != QMetaType::Type::Int
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
 * \brief SymbolLayerStyle::getSymbolSpacingAtZoom returns spacing between symbols for road names.
 *
 * The returned QVariant will contain an int if the value is not an expression,
 *  or a QJsonArray otherwise
 *
 * \param zoomLevel is the zoom level for which to calculate the size.
 * \return a QVariant containing either an int or a QJsonArray with data.
 */
QVariant SymbolLayerStyle::getSymbolSpacingAtZoom(int zoomLevel) const
{
    if (m_textSize.isNull()){
        // The default size in case no size is provided by the style sheet.
        return QVariant(250);
    } else if (m_textSize.typeId() != QMetaType::Type::Int
               && m_textSize.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_textSize.value<QList<QPair<int, int>>>();
        if (stops.size() == 0)
            return QVariant(250);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textSize);
    }
}

/*!
 * \brief SymbolLayerStyle::getTextMaxAngleAtZoom returns the maximum allowe angle difference
 * between two adjacent characters in curved text.
 *
 * The returned QVariant will contain an int if the value is not an expression,
 *  or a QJsonArray otherwise
 *
 * \param zoomLevel is the zoom level for which to calculate the size.
 * \return a QVariant containing either an int or a QJsonArray with data.
 */
QVariant SymbolLayerStyle::getTextMaxAngleAtZoom(int zoomLevel) const
{
    if (m_textMaxAngle.isNull()){
        // The default size in case no size is provided by the style sheet.
        return QVariant(45);
    } else if (m_textMaxAngle.typeId() != QMetaType::Type::Int
               && m_textMaxAngle.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, int>> stops = m_textMaxAngle.value<QList<QPair<int, int>>>();
        if (stops.size() == 0)
            return QVariant(45);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textMaxAngle);
    }
}

/*!
 * \brief SymbolLayerStyle::gettextLetterSpacingAtZoom returns spacing between characters
 * in ems
 *
 * The returned QVariant will contain an float if the value is not an expression,
 *  or a QJsonArray otherwise
 *
 * \param zoomLevel is the zoom level for which to calculate the size.
 * \return a QVariant containing either a float or a QJsonArray with data.
 */
QVariant SymbolLayerStyle::gettextLetterSpacingAtZoom(int zoomLevel) const
{
    if (m_textLetterSpacing.isNull()){
        // The default size in case no size is provided by the style sheet.
        return QVariant(0);
    } else if (m_textLetterSpacing.typeId() != QMetaType::Type::Double
               && m_textLetterSpacing.typeId() != QMetaType::Type::QJsonArray){
        QList<QPair<int, float>> stops = m_textLetterSpacing.value<QList<QPair<int, float>>>();
        if (stops.size() == 0)
            return QVariant(0);
        return QVariant(getStopOutput(stops, zoomLevel));
    } else {
        return QVariant(m_textLetterSpacing);
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
