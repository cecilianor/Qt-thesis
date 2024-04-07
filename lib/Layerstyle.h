#ifndef LAYERSTYLE_H
#define LAYERSTYLE_H

#include <QtTypes>
#include <QString>
#include <QColor>
#include <QImage>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPen>
#include<vector>

#include <optional>

/*
 *  all the layers styles follow the maptiler layer style specification : https://docs.maptiler.com/gl-style-specification/layers/
 */

class AbstractLayerStyle
{
public:
    AbstractLayerStyle() {};
    enum class LayerType : quint8
    {
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
    BackgroundStyle(){};
    ~BackgroundStyle(){};
    static std::unique_ptr<BackgroundStyle> fromJson(const QJsonObject &json);
    AbstractLayerStyle::LayerType type() const override {return AbstractLayerStyle::LayerType::background;};
    QVariant getColorAtZoom(int zoomLevel) const;
    QVariant getOpacityAtZoom(int zoomLevel) const;
    //void addBackgroundPatternImage(QImage img);//Not used for first iteration
    //QImage m_BackgroundPattern;//Not used for first iteration
private:
    QVariant m_backgroundColor;
    QVariant m_backgroundOpacity;

};

class FillLayerStyle : public AbstractLayerStyle
{
public:
    FillLayerStyle(){};
    ~FillLayerStyle(){};
    static std::unique_ptr<FillLayerStyle> fromJson(const QJsonObject &json);
    AbstractLayerStyle::LayerType type() const override {return AbstractLayerStyle::LayerType::fill;};
    QVariant getFillColorAtZoom(int zoomLevel) const;
    QVariant getFillOpacityAtZoom(int zoomLevel) const;
    QVariant getFillOutLineColorAtZoom(int zoomLevel) const;
    //Layout properties -------------------------------------------
    //float m_fillSortKey;//Not used for first iteration

    //Paint properties  -------------------------------------------
    bool m_antialias;
    //QImage m_fillPatern;//Not used for first iteration
    //QString m_fillTranslateAnchor = QString("map");//Not used for first iteration
private:
    //Layout properties -------------------------------------------

    //Paint properties  -------------------------------------------
    QVariant m_fillColor;
    QVariant m_fillOpacity;
    QVariant m_fillOutlineColor;
    //QList<QPair<int, QPair<int, int>>> m_fillTranslate;//Not used for first iteration
};
class LineLayerStyle : public AbstractLayerStyle
{
public:
    LineLayerStyle(){};
    ~LineLayerStyle(){};
    static std::unique_ptr<LineLayerStyle> fromJson(const QJsonObject &json);
    LayerType type() const override {return AbstractLayerStyle::LayerType::line;};
    QVariant getLineColorAtZoom(int zoomLevel) const;
    QVariant getLineOpacityAtZoom(int zoomLevel) const;
    QVariant getLineWidthAtZoom(int zoomLevel) const;
    Qt::PenJoinStyle getJoinStyle() const;
    Qt::PenCapStyle getCapStyle() const;
    //Layout properties -------------------------------------------

    //Paint properties  -------------------------------------------
    QList<int> m_lineDashArray;
    //QImage m_linePattern;//Not used for first iteration
    //float m_lineSortKey;//Not used for first iteration
    //QString lineTranslateAnchor = QString("map");//Not used for first iteration

private:
    //Layout properties -------------------------------------------
    //QList<QPair<int, float>> m_lineMiterLimit;//Not used for first iteration
    //QList<QPair<int, float>> m_lineRoundLimit;//Not used for first iteration
    QString m_lineCap;
    QString m_lineJoin;
    //Paint properties  -------------------------------------------
    //QList<QPair<int, int>> m_lineBlur;
    QVariant m_lineColor;
    //QList<QPair<int, float>> m_lineGapWidth;//Not used for first iteration
    //QList<QPair<int, QColor>> m_lineGradient; //Could use a list of QPair<int, QGradient> however QGradient is specified for QBrush and not QPen -- Not used for first iteration
    //QList<QPair<int, int>> m_lineOffset;//Not used for first iteration
    QVariant m_lineOpacity;
    //QList<QPair<int, QPair<int, int>>> m_lineTranslate;//Not used for first iteration
    QVariant m_lineWidth;
};

class SymbolLayerStyle : public AbstractLayerStyle
{
public:
    SymbolLayerStyle(){};
    ~SymbolLayerStyle(){};
    static std::unique_ptr<SymbolLayerStyle> fromJson(const QJsonObject &json);
    AbstractLayerStyle::LayerType type() const override {return AbstractLayerStyle::LayerType::symbol;};
    QVariant getTextSizeAtZoom(int zoomLevel) const;
    QVariant getTextColorAtZoom(int zoomLevel) const;
    QVariant getTextOpacityAtZoom(int zoomLevel) const;

    //Layout properties -------------------------------------------
    //bool m_iconAllowOverlap = false;//Not used for first iteration
    //QString m_iconAnchor = QString("center");//Not used for first iteration
    //bool m_iconIgnorePlacement = false;//Not used for first iteration
    //QImage m_iconImage;//Not used for first iteration
    //bool m_iconKeepUpright = false;//Not used for first iteration
    //bool m_iconOptional = false;//Not used for first iteration
    //QString m_iconPitchAllignment = QString("auto");//Not used for first iteration
    //QString m_iconRotationAlignment = QString("auto");//Not used for first iteration
    //QString m_iconTextFit = QString("none");//Not used for first iteration
    //bool m_symbolAboidEdges = false;//Not used for first iteration
    //QString m_symbolPlacement = QString("point");//Not used for first iteration
    //float m_symbolSortKey;//Not used for first iteration
    //QString m_symbolZOrder = QString("auto");//Not used for first iteration
    //bool m_textAllowOverlap = false;//Not used for first iteration
    //QString m_textAnchor = QString("center");//Not used for first iteration
    QVariant m_textField;
    QStringList m_textFont;
    //bool m_textIgnorePlacement = false;//Not used for first iteration
    //QString m_textJustify = QString("center");//Not used for first iteration
    //bool m_textKeenUpright = true;//Not used for first iteration
    QVariant m_textMaxWidth = 10;
    //QList<QPair<int, QPair<int, int>>> m_textOffset;//Not used for first iteration
    //bool m_textOptional = false;//Not used for first iteration
    //QString m_textPitchAlignment = QString("auto");//Not used for first iteration
    //QString m_textRotateAlignment = QString("auto");//Not used for first iteration
    //QString m_textTransform = QString("none");//Not used for first iteration
    //QString m_terxtVariableAnchor;//Not used for first iteration
    //QString m_textWritingMode;//Not used for first iteration


    //Paint properties  -------------------------------------------
    //QString m_iconTranslateAnchor = QString("map"); //Not used for first iteration
    //QString m_textTranslateAnchor = QString("map"); //Not used for first iteration
    QVariant m_textHaloWidth;
    QVariant m_textHaloColor;
private:
    //Layout properties -------------------------------------------
     //QList<QPair<int, QPair<int, int>>> m_iconOffset;//Not used for first iteration
     //QList<QPair<int, int>> m_iconPadding;//Not used for first iteration
     //QList<QPair<int, int>> m_icontRotate;//Not used for first iteration
     //QList<QPair<int, float>> m_iconSize;//Not used for first iteration
     //QList<QPair<int, int>> m_symbolSpacing;//Not used for first iteration
     //QList<QPair<int, int>> m_textLetterSpacing;//Not used for first iteration
     //QList<QPair<int, float>> m_textMaxAngle;//Not used for first iteration
     //QList<QPair<int,int>> m_textPaddig;//Not used for first iteration
     //QList<QPair<int, float>> m_textRadialOffset;//Not used for first iteration
     //QList<QPair<int, float>> m_textRotate;//Not used for first iteration
     QVariant m_textSize;

    //Paint properties  -------------------------------------------
     //QList<QPair<int, QColor>> m_iconColor;//Not used for first iteration
     //QList<QPair<int, float>> m_iconHaloBlur;//Not used for first iteration
     //QList<QPair<int, QColor>> m_iconHaloColor;//Not used for first iteration
     //QList<QPair<int, int>> m_iconHaloWidth;//Not used for first iteration
     //QList<QPair<int, QPair<int, int>>> m_iconTranslate;//Not used for first iteration
     QVariant m_textColor;
     //QList<QPair<int, int>> m_textHaloBlur;//Not used for first iteration

     QVariant m_textOpacity;
     //QList<QPair<int, QPair<int, int>>> m_textTranslate;//Not used for first iteration
};

class NotImplementedStyle : public AbstractLayerStyle
{
public:
    NotImplementedStyle(){};
    ~NotImplementedStyle(){};
    static std::unique_ptr<NotImplementedStyle> fromJson(const QJsonObject &json);
    AbstractLayerStyle::LayerType type() const override {return AbstractLayerStyle::LayerType::notImplemented;};
};

class StyleSheet
{
public:
    StyleSheet(){};
    StyleSheet(StyleSheet&&) = default;
    StyleSheet(const StyleSheet&) = delete; //Prevent accidental copying.
    ~StyleSheet();
    StyleSheet& operator=(StyleSheet&&) = default;
    static std::optional<StyleSheet> fromJson(const QJsonDocument&);

    void parseSheet(const QJsonDocument &styleSheet);
    QString m_id;
    int m_version;
    QString m_name;
    std::vector<std::unique_ptr<AbstractLayerStyle>> m_layerStyles;

};

template <class T>
inline T getStopOutput(QList<QPair<int, T>> list, int currentZoom)
{
    if(currentZoom <= list.begin()->first) return list.begin()->second;
    for(int i = 0; i < list.size(); i++)
    {
        if(currentZoom <= list.at(i).first){
            return list.at(i-1).second;
        }
    }
    return list.last().second;
}

#endif // LAYERSTYLE_H
