// Copyright (c) 2024 Cecilia Norevik Bratlie, Nils Petter Skålerud, Eimen Oueslati
// SPDX-License-Identifier: MIT

// Qt header files.
#include <QFile>
#include <QRegularExpression>
#include <QtMath>

// Other header files.
#include "LayerStyle.h"

/*!
 * \brief getColorFromString creates a QColor object from an HSL color string.
 *
 * The string is expected to be in one of the following formats:
 *      "hsl(hue, stauration%, lightness%)"
 *      "hsla(hue, stauration%, lightness%, alpha)"
 *
 * \param colorString is a QString containing color data
 *
 * \return a QColor object.
 */
QColor Bach::getColorFromString(QString colorString)
{
    colorString.remove(" ");
    //All parameters for QColor::fromHslF need to be between 0 and 1.
    if (colorString.startsWith("hsl(")) {
        static QRegularExpression re { ".*\\((\\d+),(\\d+)%,(\\d+)%\\)" };
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 4) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.);
        }
    }
    if (colorString.startsWith("hsla(")) {
        static QRegularExpression re { ".*\\((\\d+),(\\d+)%,(\\d+)%,(\\d?\\.?\\d*)\\)" };
        QRegularExpressionMatch match = re.match(colorString);
        if (match.capturedTexts().length() >= 5) {
            return QColor::fromHslF(match.capturedTexts().at(1).toInt()/359.,
                                    match.capturedTexts().at(2).toInt()/100.,
                                    match.capturedTexts().at(3).toInt()/100.,
                                    match.capturedTexts().at(4).toFloat());
        }
    }

    // In case the color has a different format than expected.
    // If the format cannot be handeled by QColor, we return black as a default color.
    QColor returnColor = QColor::fromString(colorString);
    if (!returnColor.isValid()) {
        return Qt::black;
    } else {
        return returnColor;
    }
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
 * \param jsonObj The QJsonObject to parse.
 * \return The Abstract Layer style with all parsed data.
 */
std::unique_ptr<AbstractLayerStyle> AbstractLayerStyle::fromJson(const QJsonObject &json)
{
    QString layerType = json.value("type").toString();
    std::unique_ptr<AbstractLayerStyle> returnLayerPtr;
    if (layerType == "background")
        returnLayerPtr = BackgroundStyle::fromJson(json);
    else if (layerType == "fill")
        returnLayerPtr = FillLayerStyle::fromJson(json);
    else if (layerType == "line")
        returnLayerPtr = LineLayerStyle::fromJson(json);
    else if (layerType == "symbol")
        returnLayerPtr = SymbolLayerStyle::fromJson(json);
    else
        returnLayerPtr = NotImplementedStyle::fromJson(json);

    // Set layer properties.
    AbstractLayerStyle *newLayer = returnLayerPtr.get();
    newLayer->m_id = json.value("id").toString();
    newLayer->m_source = json.value("source").toString();
    newLayer->m_sourceLayer = json.value("source-layer").toString();
    newLayer->m_minZoom = json.value("minzoom").toInt(0);
    newLayer->m_maxZoom = json.value("maxzoom").toInt(24);

    QJsonValue layout = json.value("layout");
    if(layout != QJsonValue::Undefined)
        newLayer->m_visibility = (layout.toObject().contains("visibility"))
                                     ? layout.toObject().value("visibility").toString() : "none";
     else
        newLayer->m_visibility = QString("none");

    if(json.contains("filter"))
        newLayer->m_filter = json.value("filter").toArray();

    return returnLayerPtr;
}

/*!
 * \brief StyleSheet::fromJson parses a style sheet.
 *
 * \param styleSheet The style sheet to parse, passed as a reference to
 * a QJsonDocument.
 *
 * \return is either the parsed data or a nullopt of there was no data.
 */
std::optional<StyleSheet> StyleSheet::fromJson(const QJsonDocument &styleSheetJson)
{
    StyleSheet out;

    QJsonObject styleSheetObject = styleSheetJson.object();
    out.m_id = styleSheetObject.value("id").toString();
    out.m_version = styleSheetObject.value("version").toInt();
    out.m_name = styleSheetObject.value("name").toString();

    QJsonArray layers = styleSheetObject.value("layers").toArray();
    for (const auto &layer : layers)
        out.m_layerStyles.push_back(AbstractLayerStyle::fromJson(layer.toObject()));

    return out;
}

/*!
 * \brief StyleSheet::fromJsonBytes
 * Converts a style sheet QByteArray to a StyleSheet.
 * If the conversion is unsuccessful, return std::nullopt instead.
 *
 * \param input A QByteArray encoding JSON data.
 * \return The parsed stylesheet or std::nullopt.
 */
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
    if (!openSuccess)
        return std::nullopt;

    return fromJsonBytes(file.readAll());
}
