#ifndef LAYERSTYLE_H
#define LAYERSTYLE_H

#include <QtTypes>
#include <QString>
#include <QColor>
#include <Qimage>
#include <QFont>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/*
 *  all the layers styles forllow the maptiler layer style specification : https://docs.maptiler.com/gl-style-specification/layers/
 */

class AbstractLayereStyle
{
public:
    AbstractLayereStyle() {};
    enum class LayerType : quint8
    {
        background,
        fill,
        line,
        symbol,
        notImplemented,
    };

    static AbstractLayereStyle* fromJson(const QJsonObject &json);
    virtual LayerType type() const = 0;
    QString m_id;
    QString m_sourceLayer;
    QString m_source;
    int m_minZoom = 0;
    int m_maxZoom = 24;
    QString m_visibility;
};

class BackgroundStyle : public AbstractLayereStyle
{
public:
    BackgroundStyle(){};
    ~BackgroundStyle(){};
    static BackgroundStyle* fromJson(const QJsonObject &json);
    AbstractLayereStyle::LayerType type() const override {return AbstractLayereStyle::LayerType::background;};
    QColor getColorAtZoom(int zoomLevel) const;
    float getOpacityAtZoom(int zoomLevel) const;
    void addBackgroundColorStop(int zoomLevel, QString hslColor);
    void addBackgroundOpacityStop(int zoomLevel, float opacity);
    //void addBackgroundPatternImage(QImage img);//Not used for first iteration
    //QImage m_BackgroundPattern;//Not used for first iteration
private:
    QList<QPair<int, QColor>> m_backgroundColor;
    QList<QPair<int, float>> m_backgroundOpacity;

};

class FillLayerStyle : public AbstractLayereStyle
{
public:
    FillLayerStyle(){};
    ~FillLayerStyle(){};
    static FillLayerStyle* fromJson(const QJsonObject &json);
    AbstractLayereStyle::LayerType type() const override {return AbstractLayereStyle::LayerType::fill;};
    QColor getFillColorAtZoom(int zoomLevel) const;
    float getFillOpacityAtZoom(int zoomLevel) const;
    QColor getFillOutLineColorAtZoom(int zoomLevel) const;
    void addFillColorStop(int zoomLevel, QString hslColor);
    void addFillOpacityStop(int zoomLevel, float opacity);
    void addFillOutlineColorStop(int zoomLevel, QString hslColor);
    //Layout properties -------------------------------------------
    //float m_fillSortKey;//Not used for first iteration

    //Paint properties  -------------------------------------------
    bool m_antialias;
    //QImage m_fillPatern;//Not used for first iteration
    //QString m_fillTranslateAnchor = QString("map");//Not used for first iteration
private:
    //Layout properties -------------------------------------------

    //Paint properties  -------------------------------------------
    QList<QPair<int, QColor>> m_fillColor;
    QList<QPair<int, float>> m_fillOpacity;
    QList<QPair<int, QColor>> m_fillOutlineColor;
    //QList<QPair<int, QPair<int, int>>> m_fillTranslate;//Not used for first iteration
};
class LineLayerStyle : public AbstractLayereStyle
{
public:
    LineLayerStyle(){};
    ~LineLayerStyle(){};
    static LineLayerStyle* fromJson(const QJsonObject &json);
    LayerType type() const override {return AbstractLayereStyle::LayerType::line;};
    QColor getLineColorAtZoom(int zoomLevel) const;
    float getLineOpacityAtZoom(int zoomLevel) const;
    float getLineMitterLimitAtZoom(int zoomLevel) const;
    int getLineWidthAtZoom(int zoomLevel) const;
    void addLineColorStop(int zoomLevel, QString hslColor);
    void addLineOpacityStop(int zoomLevel, float opacity);
    void addLineMitterLimitStop(int zoomLevel, float limit);
    void addLinewidthStop(int zoomLevel, int width);
    //Layout properties -------------------------------------------
    QString m_lineCap;
    QString m_lineJoin;
    //Paint properties  -------------------------------------------
    QList<int> m_lineDashArray;
    //QImage m_linePattern;//Not used for first iteration
    //float m_lineSortKey;//Not used for first iteration
    //QString lineTranslateAnchor = QString("map");//Not used for first iteration

private:
    //Layout properties -------------------------------------------
    QList<QPair<int, float>> m_lineMiterLimit;
    //QList<QPair<int, float>> m_lineRoundLimit;//Not used for first iteration
    //Paint properties  -------------------------------------------
    //QList<QPair<int, int>> m_lineBlur;
    QList<QPair<int, QColor>> m_lineColor;
    //QList<QPair<int, float>> m_lineGapWidth;//Not used for first iteration
    //QList<QPair<int, QColor>> m_lineGradient; //Could use a list of QPair<int, QGradient> however QGradient is specified for QBrush and not QPen -- Not used for first iteration
    //QList<QPair<int, int>> m_lineOffset;//Not used for first iteration
    QList<QPair<int, float>> m_lineOpacity;
    //QList<QPair<int, QPair<int, int>>> m_lineTranslate;//Not used for first iteration
    QList<QPair<int, int>> m_lineWidth;
};

class SymbolLayerStyle : public AbstractLayereStyle
{
public:
    SymbolLayerStyle(){};
    ~SymbolLayerStyle(){};
    static SymbolLayerStyle* fromJson(const QJsonObject &json);
    AbstractLayereStyle::LayerType type() const override {return AbstractLayereStyle::LayerType::symbol;};
    int getTextSizeAtZoom(int zoomLevel) const;
    QColor getTextColorAtZoom(int zoomLevel) const;
    float getTextOpacityAtZoom(int zoomLevel) const;
    void addTextSizeStop(int zoomLevel, int size);
    void addTextColorStop(int zoomLevel, QString hslColor);
    void addTextPacity(int zoomLevel, float opacity);

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
    QString m_textField;
    //QFont m_textFont = QFont({"Open Sans Regular", "Arial Unicode MS Regular"}); // is this correct way to initialize the object????
    //bool m_textIgnorePlacement = false;//Not used for first iteration
    //QString m_textJustify = QString("center");//Not used for first iteration
    //bool m_textKeenUpright = true;//Not used for first iteration
    //float m_textMaxWidth = 10;//Not used for first iteration
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
     QList<QPair<int, int>> m_textSize;

    //Paint properties  -------------------------------------------
     //QList<QPair<int, QColor>> m_iconColor;//Not used for first iteration
     //QList<QPair<int, float>> m_iconHaloBlur;//Not used for first iteration
     //QList<QPair<int, QColor>> m_iconHaloColor;//Not used for first iteration
     //QList<QPair<int, int>> m_iconHaloWidth;//Not used for first iteration
     //QList<QPair<int, QPair<int, int>>> m_iconTranslate;//Not used for first iteration
     QList<QPair<int, QColor>> m_textColor;
     //QList<QPair<int, int>> m_textHaloBlur;//Not used for first iteration
     //QList<QPair<int, int>> m_textHaloWidth;//Not used for first iteration
     QList<QPair<int, float>> m_textOpacity;
     //QList<QPair<int, QPair<int, int>>> m_textTranslate;//Not used for first iteration
};

class NotImplementedStyle : public AbstractLayereStyle
{
public:
    NotImplementedStyle(){};
    ~NotImplementedStyle(){};
    static NotImplementedStyle* fromJson(const QJsonObject &json);
    AbstractLayereStyle::LayerType type() const override {return AbstractLayereStyle::LayerType::notImplemented;};
};

class StyleSheet
{
public:
    StyleSheet(){};
    ~StyleSheet();
    void parseSheet(const QJsonDocument &styleSheet);
    QString m_id;
    int m_version;
    QString m_name;
    //QMap<QString, QString> m_sources;//should have a separate class for tile sources //Not used for first iteration
    QVector<AbstractLayereStyle*> m_layerStyles;

};

#endif // LAYERSTYLE_H