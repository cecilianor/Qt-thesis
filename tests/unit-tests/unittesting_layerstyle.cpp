#include <QObject>
#include <QTest>

#include "Layerstyle.h"

class UnitTesting : public QObject
{
    Q_OBJECT

private:
    const QString path = ":/unitTestResources/styleTest.json";
    QFile styleFile;
    QJsonDocument styleSheetDoc;
    StyleSheet styleSheet;
    AbstractLayerStyle *backgroundLayer;
    AbstractLayerStyle *fillLayer;
    AbstractLayerStyle *lineLayer;
    AbstractLayerStyle *symbolLayer;
    AbstractLayerStyle *unknownLayer;

private slots:
    void initTestCase();
    void getStopOutput_returns_basic_values();
    void parseSheet_returns_basic_values();
    void test_background_layer_parsing();
    void test_fill_layer_parsing();
    void test_line_layer_parsing();
    void test_symbol_layer_parsing();
    void test_unknown_layer_parsing();
    void cleanupTestCase();
};

QTEST_MAIN(UnitTesting)

void UnitTesting::initTestCase()
{
    if (!QFile::exists(path))
        QFAIL("File \"" + path.toUtf8() + "\" does not exist");

    // Open the JSON file and check that the operation was successful.
    styleFile.setFileName(path);
    if (!styleFile.open(QIODevice::ReadOnly))
        QFAIL("Failed to open file \"" + path.toUtf8() + "\"");

    // Parse the JSON file into a QJsonDocument for further processing.
    QJsonParseError parserError;
    styleSheetDoc = QJsonDocument::fromJson(styleFile.readAll(), &parserError);

    // Check for parsing errors.
    if (parserError.error != QJsonParseError::NoError)
        QFAIL("JSON parsing error: " + parserError.errorString().toUtf8());

    styleSheet.parseSheet(styleSheetDoc);
    backgroundLayer = styleSheet.m_layerStyles.at(0).get();
    fillLayer = styleSheet.m_layerStyles.at(1).get();
    lineLayer = styleSheet.m_layerStyles.at(2).get();
    symbolLayer = styleSheet.m_layerStyles.at(3).get();
    unknownLayer = styleSheet.m_layerStyles.at(4).get();
}


#include "unittesting_layerstyle.moc"
// This include needs to match the name of this .cpp file.
//Test the functionality of the function to determine stopoutputs.
void UnitTesting::getStopOutput_returns_basic_values(){
    QList<QPair<int, float>> stops({{4,0.8},{9, 1.1}, {11, 1.75}, {18, 2.5},{22, 2.72}});
    //List of pairs that represent the zoom level and the expected output for it.
    QList<QPair<int, float>> values({{0, 0.8}, {3, 0.8}, {4, 0.8}, {8, 0.8}, {9, 0.8}, {10, 1.1}, {16, 1.75}, {18, 1.75}, {20, 2.5}, {23, 2.72}});
    for(auto value : values){
        auto result = getStopOutput(stops, value.first);
        auto errorMsg = QString("At value #%1. Expected %2, but got %3")
                            .arg(value.first)
                            .arg(value.second)
                            .arg(result);
        QVERIFY2(result == value.second, errorMsg.toUtf8());
    }
}


void UnitTesting::test_background_layer_parsing()
{
    QString testError;
    QString expectedId = "Background";
    QString expectedVisibility = "visible";
    int expectedMinZoom = 0;
    int expectedMaxZoom = 24;

    testError = QString("The layer style is expected to be of type BackgroundLayerStyle");
    QVERIFY2(backgroundLayer->type() == AbstractLayerStyle::LayerType::background, testError.toUtf8());
    auto const& backgroundStyle = *static_cast<BackgroundStyle const*>(backgroundLayer);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(backgroundStyle.m_id);
    QVERIFY2(backgroundStyle.m_id == expectedId, testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg(expectedVisibility)
                    .arg(backgroundStyle.m_visibility);
    QVERIFY2(backgroundStyle.m_visibility == expectedVisibility, testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(expectedMinZoom)
                    .arg(backgroundStyle.m_minZoom);
    QVERIFY2(backgroundStyle.m_minZoom == expectedMinZoom, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(expectedMaxZoom)
                    .arg(backgroundStyle.m_maxZoom);
    QVERIFY2(backgroundStyle.m_maxZoom == expectedMaxZoom, testError.toUtf8());

    QColor expectedColorForStop1 = QColor::fromHslF(60/359.,20/100.,85/100.);
    QColor expectedColorForStop2 = QColor::fromHslF(60/359.,24/100.,90/100.);
    bool hueMatch;
    bool saturationMatch;
    bool lightnessMatch;
    bool alphaMatch;
    for(int i = 0; i < 21; i++){
        testError =  QString("The background-color does not match at zoom %1").arg(i);
        QColor backgroundColor = backgroundStyle.getColorAtZoom(i).value<QColor>();
        hueMatch = backgroundColor.hslHue() == expectedColorForStop1.hslHue();
        saturationMatch = backgroundColor.hslSaturation() == expectedColorForStop1.hslSaturation();
        lightnessMatch = backgroundColor.lightnessF() == expectedColorForStop1.lightnessF();
        alphaMatch = backgroundColor.alphaF() == expectedColorForStop1.alphaF();
        QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());
    }

    for(int i = 21; i < 25; i++){
        testError =  QString("The background-color does not match at zoom %1").arg(i);
        QColor backgroundColor = backgroundStyle.getColorAtZoom(i).value<QColor>();
        hueMatch = backgroundColor.hslHue() == expectedColorForStop2.hslHue();
        saturationMatch = backgroundColor.hslSaturation() == expectedColorForStop2.hslSaturation();
        lightnessMatch = backgroundColor.lightnessF() == expectedColorForStop2.lightnessF();
        alphaMatch = backgroundColor.alphaF() == expectedColorForStop2.alphaF();
        QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());
    }
}



void UnitTesting::test_fill_layer_parsing()
{
    QString testError;
    QString expectedId = "Glacier";
    QString expectedSource = "maptiler_planet";
    QString expectedSourceLayer = "globallandcover";
    QString expectedVisibility = "visible";
    int expectedMinZoom = 0;
    int expectedMaxZoom = 8;
    bool expectedAntiAliasing = true;
    QColor expectedColor = QColor::fromHslF(0/359.,0/100.,100/100., 0.7);
    bool hueMatch;
    bool saturationMatch;
    bool lightnessMatch;
    bool alphaMatch;
    int expectedFilterSize = 3;

    testError = QString("The layer style is expected to be of type FillLayerStyle");
    QVERIFY2(fillLayer->type() == AbstractLayerStyle::LayerType::fill, testError.toUtf8());
    auto const& filllayerStyle = *static_cast<FillLayerStyle const*>(fillLayer);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(filllayerStyle.m_id);
    QVERIFY2(filllayerStyle.m_id == expectedId, testError.toUtf8());

    testError =  QString("The layerStyle source does not match, expected %1 but got %2")
                    .arg(expectedSource)
                    .arg(filllayerStyle.m_source);
    QVERIFY2(filllayerStyle.m_source == expectedSource, testError.toUtf8());

    testError =  QString("The layerStyle source layer does not match, expected %1 but got %2")
                    .arg(expectedSourceLayer)
                    .arg(filllayerStyle.m_sourceLayer);
    QVERIFY2(filllayerStyle.m_sourceLayer == expectedSourceLayer, testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg(expectedVisibility)
                    .arg(filllayerStyle.m_visibility);
    QVERIFY2(filllayerStyle.m_visibility == expectedVisibility, testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(expectedMinZoom)
                    .arg(filllayerStyle.m_minZoom);
    QVERIFY2(filllayerStyle.m_minZoom == expectedMinZoom, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(expectedMaxZoom)
                    .arg(filllayerStyle.m_maxZoom);
    QVERIFY2(filllayerStyle.m_maxZoom == expectedMaxZoom, testError.toUtf8());

    testError =  QString("The layerStyle anti-aliasing property does not match, expected %1 but got %2")
                    .arg(expectedAntiAliasing)
                    .arg(filllayerStyle.m_antialias);
    QVERIFY2(filllayerStyle.m_antialias == expectedAntiAliasing, testError.toUtf8());

    testError =  QString("The fill color variable type is not correct at zoom %1").arg(1);
    QVariant colorVariant = filllayerStyle.getFillColorAtZoom(1);
    QVERIFY2(colorVariant.typeId() == QMetaType::Type::QColor, testError.toUtf8());

    QColor fillColor = colorVariant.value<QColor>();
    hueMatch = fillColor.hslHue() == expectedColor.hslHue();
    saturationMatch = fillColor.hslSaturation() == expectedColor.hslSaturation();
    lightnessMatch = fillColor.lightnessF() == expectedColor.lightnessF();
    alphaMatch = fillColor.alphaF() == expectedColor.alphaF();
    testError =  QString("The background-color does not match at zoom %1").arg(1);
    QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());

    testError =  QString("The Filter json array size does not match, expected %1 but got %2")
                    .arg(expectedFilterSize)
                    .arg(filllayerStyle.m_filter.size());
    QVERIFY2(filllayerStyle.m_filter.size() == expectedFilterSize, testError.toUtf8());
}

void UnitTesting::test_line_layer_parsing()
{
    QString testError;
    QString expectedId = "River";
    QString expectedSource = "maptiler_planet";
    QString expectedSourceLayer = "waterway";
    QString expectedVisibility = "visible";
    int expectedMinZoom = 0;
    int expectedMaxZoom = 24;
    QColor expectedColor = QColor::fromHslF(205/359.,56/100.,73/100.);
    bool hueMatch;
    bool saturationMatch;
    bool lightnessMatch;
    bool alphaMatch;
    int expectedLineWidthStop1 = 1;
    int expectedLineWidthStop2 = 3;
    int expectedLineOpacitySize = 5;
    int expectedFilterSize = 3;

    testError = QString("The layer style is expected to be of type LineLayerStyle");
    QVERIFY2(lineLayer->type() == AbstractLayerStyle::LayerType::line, testError.toUtf8());
    auto const& lineLayerStyle = *static_cast<LineLayerStyle const*>(lineLayer);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(lineLayerStyle.m_id);
    QVERIFY2(lineLayerStyle.m_id == expectedId, testError.toUtf8());

    testError =  QString("The layerStyle source does not match, expected %1 but got %2")
                    .arg(expectedSource)
                    .arg(lineLayerStyle.m_source);
    QVERIFY2(lineLayerStyle.m_source == expectedSource, testError.toUtf8());

    testError =  QString("The layerStyle source layer does not match, expected %1 but got %2")
                    .arg(expectedSourceLayer)
                    .arg(lineLayerStyle.m_sourceLayer);
    QVERIFY2(lineLayerStyle.m_sourceLayer == expectedSourceLayer, testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg(expectedVisibility)
                    .arg(lineLayerStyle.m_visibility);
    QVERIFY2(lineLayerStyle.m_visibility == expectedVisibility, testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(expectedMinZoom)
                    .arg(lineLayerStyle.m_minZoom);
    QVERIFY2(lineLayerStyle.m_minZoom == expectedMinZoom, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(expectedMaxZoom)
                    .arg(lineLayerStyle.m_maxZoom);
    QVERIFY2(lineLayerStyle.m_maxZoom == expectedMaxZoom, testError.toUtf8());

    testError =  QString("The line color variable type is not correct at zoom %1").arg(1);
    QVariant colorVariant = lineLayerStyle.getLineColorAtZoom(1);
    QVERIFY2(colorVariant.typeId() == QMetaType::Type::QColor, testError.toUtf8());

    QColor lineColor = colorVariant.value<QColor>();
    hueMatch = lineColor.hslHue() == expectedColor.hslHue();
    saturationMatch = lineColor.hslSaturation() == expectedColor.hslSaturation();
    lightnessMatch = lineColor.lightnessF() == expectedColor.lightnessF();
    alphaMatch = lineColor.alphaF() == expectedColor.alphaF();
    testError =  QString("The line color does not match at zoom %1").arg(1);
    QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());


    for(int i = 0; i < 19; i++){
        int lineWidth = lineLayerStyle.getLineWidthAtZoom(i).toInt();
        testError =  QString("The line width does not match at zoom %1, expected %2 but got %3")
                        .arg(i)
                        .arg(expectedLineWidthStop1)
                        .arg(lineWidth);
        QVERIFY2(lineWidth == expectedLineWidthStop1, testError.toUtf8());
    }

    int lineWidth = lineLayerStyle.getLineWidthAtZoom(19).toInt();
    testError =  QString("The line width does not match at zoom 19, expected %1 but got %2")
                    .arg(expectedLineWidthStop2)
                    .arg(lineWidth);
    QVERIFY2(lineWidth == expectedLineWidthStop2, testError.toUtf8());

    testError =  QString("The line opacity variable type is not correct");
    QVERIFY2(lineLayerStyle.getLineOpacityAtZoom(1).typeId() == QMetaType::Type::QJsonArray, testError.toUtf8());

    int lineOpacitySize = lineLayerStyle.getLineOpacityAtZoom(1).toJsonArray().size();
    testError =  QString("The line opacity json array size does not match, expected %1 but got %2")
                    .arg(expectedLineOpacitySize)
                    .arg(lineOpacitySize);
    QVERIFY2(lineOpacitySize == expectedLineOpacitySize, testError.toUtf8());

    testError =  QString("The Filter size json array does not match, expected %1 but got %2")
                    .arg(expectedFilterSize)
                    .arg(lineLayerStyle.m_filter.size());
    QVERIFY2(lineLayerStyle.m_filter.size() == expectedFilterSize, testError.toUtf8());
}

// Tests symbol layer parsing
void UnitTesting::test_symbol_layer_parsing()
{
    QString testError;
    QString expectedId = "Airport labels";
    QString expectedSource = "maptiler_planet";
    QString expectedSourceLayer = "aerodrome_label";
    QString expectedVisibility = "visible";
    int expectedMinZoom = 10;
    int expectedMaxZoom = 24;
    QStringList expectedFont = {"Noto Sans Regular"};
    int expectedTextSizeStop1 = 10;
    int expectedTextSizeStop2 = 12;
    int expectedTextSizeStop3 = 14;
    int expectedTextFieldSize = 3;
    QColor expectedColor = QColor::fromHslF(0/359.,0/100.,12/100.);
    bool hueMatch;
    bool saturationMatch;
    bool lightnessMatch;
    bool alphaMatch;
    int expectedFilterSize = 2;

    testError = QString("The layer style is expected to be of type SymbolLayerStyle");
    QVERIFY2(symbolLayer->type() == AbstractLayerStyle::LayerType::symbol, testError.toUtf8());
    auto const& symbolLayerStyle = *static_cast<SymbolLayerStyle const*>(symbolLayer);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(symbolLayerStyle.m_id);
    QVERIFY2(symbolLayerStyle.m_id == expectedId, testError.toUtf8());

    testError =  QString("The layerStyle source does not match, expected %1 but got %2")
                    .arg(expectedSource)
                    .arg(symbolLayerStyle.m_source);
    QVERIFY2(symbolLayerStyle.m_source == expectedSource, testError.toUtf8());

    testError =  QString("The layerStyle source layer does not match, expected %1 but got %2")
                    .arg(expectedSourceLayer)
                    .arg(symbolLayerStyle.m_sourceLayer);
    QVERIFY2(symbolLayerStyle.m_sourceLayer == expectedSourceLayer, testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg(expectedVisibility)
                    .arg(symbolLayerStyle.m_visibility);
    QVERIFY2(symbolLayerStyle.m_visibility == expectedVisibility, testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(expectedMinZoom)
                    .arg(symbolLayerStyle.m_minZoom);
    QVERIFY2(symbolLayerStyle.m_minZoom == expectedMinZoom, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(expectedMaxZoom)
                    .arg(symbolLayerStyle.m_maxZoom);
    QVERIFY2(symbolLayerStyle.m_maxZoom == expectedMaxZoom, testError.toUtf8());

    testError =  QString("The layerStyle text font does not match");
    QVERIFY2(symbolLayerStyle.m_textFont == expectedFont, testError.toUtf8());

    for(int i = 0; i < 15; i++){
        int size = symbolLayerStyle.getTextSizeAtZoom(i).toInt();
        testError =  QString("The layerStyle text size does not match, expected %1 but got %2")
                        .arg(expectedTextSizeStop1)
                        .arg(size);
        QVERIFY2(size == expectedTextSizeStop1, testError.toUtf8());
    }

    for(int i = 15; i < 17; i++){
        int size = symbolLayerStyle.getTextSizeAtZoom(i).toInt();
        testError =  QString("The layerStyle text size does not match, expected %1 but got %2")
                        .arg(expectedTextSizeStop2)
                        .arg(size);
        QVERIFY2(size == expectedTextSizeStop2, testError.toUtf8());
    }

    for(int i = 17; i < 21; i++){
        int size = symbolLayerStyle.getTextSizeAtZoom(i).toInt();
        testError =  QString("The layerStyle text size does not match, expected %1 but got %2")
                        .arg(expectedTextSizeStop3)
                        .arg(size);
        QVERIFY2(size == expectedTextSizeStop3, testError.toUtf8());
    }

    testError =  QString("The text field variable type is not correct");
    QVERIFY2(symbolLayerStyle.m_textField.typeId() == QMetaType::Type::QJsonArray, testError.toUtf8());

    int textField = symbolLayerStyle.m_textField.toJsonArray().size();
    testError =  QString("The text field json array size does not match, expected %1 but got %2")
                    .arg(expectedTextFieldSize)
                    .arg(textField);
    QVERIFY2(textField == expectedTextFieldSize, testError.toUtf8());


    testError =  QString("The text color variable type is not correct at zoom %1").arg(1);
    QVariant colorVariant = symbolLayerStyle.getTextColorAtZoom(1);
    QVERIFY2(colorVariant.typeId() == QMetaType::Type::QColor, testError.toUtf8());

    QColor textColor = colorVariant.value<QColor>();
    hueMatch = textColor.hslHue() == expectedColor.hslHue();
    saturationMatch = textColor.hslSaturation() == expectedColor.hslSaturation();
    lightnessMatch = textColor.lightnessF() == expectedColor.lightnessF();
    alphaMatch = textColor.alphaF() == expectedColor.alphaF();
    testError =  QString("The text color does not match at zoom %1").arg(1);
    QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());

    testError =  QString("The Filter size json array does not match, expected %1 but got %2")
                    .arg(expectedFilterSize)
                    .arg(symbolLayerStyle.m_filter.size());
    QVERIFY2(symbolLayerStyle.m_filter.size() == expectedFilterSize, testError.toUtf8());
}


void UnitTesting::test_unknown_layer_parsing()
{
    QString testError;
    testError = QString("The layer style is expected to be of type NotImpleneted");
    QVERIFY2(unknownLayer->type() == AbstractLayerStyle::LayerType::notImplemented, testError.toUtf8());
}

//Test the parsing functionality of the StyleSheet class.
void UnitTesting::parseSheet_returns_basic_values()
{
    QString testError;
    QString expectedId = "basic-v2";
    QString expectedName = "Basic";
    int expectedVersion = 8;
    int expectedNumberOfLayers = 5;

    testError = QString("The style Sheet object id does not match, expected %1 but got %2")
                    .arg(expectedId, styleSheet.m_id);
    QVERIFY2(styleSheet.m_id == expectedId, testError.toUtf8());

    testError = QString("The style Sheet object version does not match, expected %1 but got %2")
                    .arg(expectedVersion, styleSheet.m_version);
    QVERIFY2(styleSheet.m_version == expectedVersion, testError.toUtf8());

    testError = QString("The style Sheet object name does not match, expected %1 but got %2")
                    .arg(expectedName, styleSheet.m_name);
    QVERIFY2(styleSheet.m_name == expectedName, testError.toUtf8());

    testError = QString("The style Sheet object does not contain the correct amount of layers, expected %1 but got %2")
                    .arg(expectedNumberOfLayers, styleSheet.m_layerStyles.size());
    QVERIFY2(styleSheet.m_layerStyles.size() == expectedNumberOfLayers, testError.toUtf8());

    for(int i = 0; i < styleSheet.m_layerStyles.size(); i++){
        testError = QString("The style layer pointer at index %1 is invalid").arg(i);
        QVERIFY2(styleSheet.m_layerStyles.at(i) != nullptr, testError.toUtf8());
    }


}

void UnitTesting::cleanupTestCase()
{
    styleFile.close();
}
