#ifndef LAYERSTYLE_H
#define LAYERSTYLE_H

#include <QColor>
#include <QFont>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPen>
#include <QString>
#include <QtTypes>
#include <vector>
#include <optional>

/*
 *  All the layers styles follow the maptiler layer style specification :
 *  https://docs.maptiler.com/gl-style-specification/layers/
 */

/*!
 * \brief The AbstractLayerStyle class
 * Abstract parent class for all specific layerstyle types.
 */
class AbstractLayerStyle
{
public:
    virtual ~AbstractLayerStyle() {}

    enum class LayerType {
        background,
        fill,
        line,
        symbol,
        notImplemented,
    };

    static std::unique_ptr<AbstractLayerStyle> fromJson(const QJsonObject &json);
    virtual LayerType type() const = 0;
    QString m_id;
    QString m_sourceLayer;
    QString m_source;
    int m_minZoom = 0;
    int m_maxZoom = 24;
    QString m_visibility;
    QJsonArray m_filter;
};

class BackgroundStyle : public AbstractLayerStyle
{
public:
    static std::unique_ptr<BackgroundStyle> fromJson(const QJsonObject &json);

    LayerType type() const override
    {
        return LayerType::background;
    }

    QVariant getColorAtZoom(int zoomLevel) const;
    QVariant getOpacityAtZoom(int zoomLevel) const;

private:
    QVariant m_backgroundColor;
    QVariant m_backgroundOpacity;

};

class FillLayerStyle : public AbstractLayerStyle
{
public:
    static std::unique_ptr<FillLayerStyle> fromJson(const QJsonObject &json);

    LayerType type() const override
    {
        return LayerType::fill;
    }

    QVariant getFillColorAtZoom(int zoomLevel) const;
    QVariant getFillOpacityAtZoom(int zoomLevel) const;
    QVariant getFillOutLineColorAtZoom(int zoomLevel) const;

    bool m_antialias;
private:
    QVariant m_fillColor;
    QVariant m_fillOpacity;
    QVariant m_fillOutlineColor;
};

class LineLayerStyle : public AbstractLayerStyle
{
private:
    QString m_lineCap;
    QString m_lineJoin;
    QVariant m_lineColor;
    QVariant m_lineOpacity;
    QVariant m_lineWidth;

public:
    static std::unique_ptr<LineLayerStyle> fromJson(const QJsonObject &json);

    LayerType type() const override {
        return AbstractLayerStyle::LayerType::line;
    }

    QVariant getLineColorAtZoom(int zoomLevel) const;
    QVariant getLineOpacityAtZoom(int zoomLevel) const;
    QVariant getLineWidthAtZoom(int zoomLevel) const;

    Qt::PenJoinStyle getJoinStyle() const;
    Qt::PenCapStyle getCapStyle() const;

    QList<qreal> m_lineDashArray;
};

class SymbolLayerStyle : public AbstractLayerStyle
{
public:
    static std::unique_ptr<SymbolLayerStyle> fromJson(const QJsonObject &json);

    LayerType type() const override {
        return AbstractLayerStyle::LayerType::symbol;
    }

    QVariant getTextSizeAtZoom(int zoomLevel) const;
    QVariant getTextColorAtZoom(int zoomLevel) const;
    QVariant getTextOpacityAtZoom(int zoomLevel) const;
    QVariant getSymbolSpacingAtZoom(int zoomLevel) const;
    QVariant getTextMaxAngleAtZoom(int zoomLevel) const;
    QVariant getTextLetterSpacingAtZoom(int zoomLevel) const;

    QVariant m_textField;
    QStringList m_textFont;

    QVariant m_textMaxWidth = 10;

    QVariant m_textHaloWidth;
    QVariant m_textHaloColor;
private:
    QVariant m_textSize;

    QVariant m_textColor;

    QVariant m_textOpacity;

    QVariant m_symbolSpacing;
    QVariant m_textLetterSpacing;
    QVariant m_textMaxAngle;
};

class NotImplementedStyle : public AbstractLayerStyle
{
public:
    static std::unique_ptr<NotImplementedStyle> fromJson(const QJsonObject &json);
    AbstractLayerStyle::LayerType type() const override
    {
        return LayerType::notImplemented;
    }
};

class StyleSheet
{
public:
    StyleSheet() = default;
    StyleSheet(StyleSheet&&) = default;
    // Explicitly marked as deleted in order to prevent
    // accidental copies.
    StyleSheet(const StyleSheet&) = delete;
    // Explicitly marked as deleted in order to prevent
    // accidental copies.
    StyleSheet& operator=(const StyleSheet&) = delete;
    StyleSheet& operator=(StyleSheet&&) = default;

    static std::optional<StyleSheet> fromJson(const QJsonDocument&);
    static std::optional<StyleSheet> fromJsonBytes(const QByteArray& input);
    static std::optional<StyleSheet> fromJsonFile(const QString& path);

    QString m_id;
    int m_version;
    QString m_name;
    std::vector<std::unique_ptr<AbstractLayerStyle>> m_layerStyles;
};

template <class T>
inline T getStopOutput(QList<QPair<int, T>> list, int currentZoom)
{
    if (currentZoom <= list.begin()->first) {
        return list.begin()->second;
    }

    for(int i = 0; i < list.size(); i++)
    {
        if (currentZoom <= list[i].first) {
            return list[i-1].second;
        }
    }
    return list.last().second;
}

namespace Bach {

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
    QColor getColorFromString(QString colorString);
}

#endif // LAYERSTYLE_H
