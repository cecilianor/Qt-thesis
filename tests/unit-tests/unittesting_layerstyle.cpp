#include "unittesting.h"

#include "Layerstyle.h"

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


void testBackgroundLayerStyle(AbstractLayereStyle *layerStyle)
{
    QString testError;
    QString expectedId = "Background";
    QString expectedVisibility = "visible";
    int expectedMinZoom = 0;
    int expectedMaxZoom = 24;

    testError = QString("The layer style is expected to be of type BackgroundLayerStyle");
    QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::background, testError.toUtf8());
    auto const& backgroundStyle = *static_cast<BackgroundStyle const*>(layerStyle);

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


void testFillLyerStyle(AbstractLayereStyle *layerStyle)
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
    QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::fill, testError.toUtf8());
    auto const& filllayerStyle = *static_cast<FillLayerStyle const*>(layerStyle);

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

void testLineLayerStyle(AbstractLayereStyle *layerStyle)
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
    QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::line, testError.toUtf8());
    auto const& lineLyaerStyle = *static_cast<LineLayerStyle const*>(layerStyle);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(lineLyaerStyle.m_id);
    QVERIFY2(lineLyaerStyle.m_id == expectedId, testError.toUtf8());

    testError =  QString("The layerStyle source does not match, expected %1 but got %2")
                    .arg(expectedSource)
                    .arg(lineLyaerStyle.m_source);
    QVERIFY2(lineLyaerStyle.m_source == expectedSource, testError.toUtf8());

    testError =  QString("The layerStyle source layer does not match, expected %1 but got %2")
                    .arg(expectedSourceLayer)
                    .arg(lineLyaerStyle.m_sourceLayer);
    QVERIFY2(lineLyaerStyle.m_sourceLayer == expectedSourceLayer, testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg(expectedVisibility)
                    .arg(lineLyaerStyle.m_visibility);
    QVERIFY2(lineLyaerStyle.m_visibility == expectedVisibility, testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(expectedMinZoom)
                    .arg(lineLyaerStyle.m_minZoom);
    QVERIFY2(lineLyaerStyle.m_minZoom == expectedMinZoom, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(expectedMaxZoom)
                    .arg(lineLyaerStyle.m_maxZoom);
    QVERIFY2(lineLyaerStyle.m_maxZoom == expectedMaxZoom, testError.toUtf8());

    testError =  QString("The line color variable type is not correct at zoom %1").arg(1);
    QVariant colorVariant = lineLyaerStyle.getLineColorAtZoom(1);
    QVERIFY2(colorVariant.typeId() == QMetaType::Type::QColor, testError.toUtf8());

    QColor lineColor = colorVariant.value<QColor>();
    hueMatch = lineColor.hslHue() == expectedColor.hslHue();
    saturationMatch = lineColor.hslSaturation() == expectedColor.hslSaturation();
    lightnessMatch = lineColor.lightnessF() == expectedColor.lightnessF();
    alphaMatch = lineColor.alphaF() == expectedColor.alphaF();
    testError =  QString("The line color does not match at zoom %1").arg(1);
    QVERIFY2(hueMatch && saturationMatch && lightnessMatch && alphaMatch == true, testError.toUtf8());


    for(int i = 0; i < 19; i++){
        int lineWidth = lineLyaerStyle.getLineWidthAtZoom(i).toInt();
        testError =  QString("The line width does not match at zoom %1, expected %2 but got %3")
            .arg(i)
            .arg(expectedLineWidthStop1)
            .arg(lineWidth);
        QVERIFY2(lineWidth == expectedLineWidthStop1, testError.toUtf8());
    }

    int lineWidth = lineLyaerStyle.getLineWidthAtZoom(19).toInt();
    testError =  QString("The line width does not match at zoom 19, expected %1 but got %2")
                    .arg(expectedLineWidthStop2)
                    .arg(lineWidth);
    QVERIFY2(lineWidth == expectedLineWidthStop2, testError.toUtf8());

    testError =  QString("The line opacity variable type is not correct");
    QVERIFY2(lineLyaerStyle.getLineOpacityAtZoom(1).typeId() == QMetaType::Type::QJsonArray, testError.toUtf8());

    int lineOpacitySize = lineLyaerStyle.getLineOpacityAtZoom(1).toJsonArray().size();
    testError =  QString("The line opacity json array size does not match, expected %1 but got %2")
                    .arg(expectedLineOpacitySize)
                    .arg(lineOpacitySize);
    QVERIFY2(lineOpacitySize == expectedLineOpacitySize, testError.toUtf8());

    testError =  QString("The Filter size json array does not match, expected %1 but got %2")
                    .arg(expectedFilterSize)
                    .arg(lineLyaerStyle.m_filter.size());
    QVERIFY2(lineLyaerStyle.m_filter.size() == expectedFilterSize, testError.toUtf8());


}


void testPointLayerStyle(AbstractLayereStyle *layerStyle)
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
    QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::symbol, testError.toUtf8());
    auto const& symbolLayerStyle = *static_cast<SymbolLayerStyle const*>(layerStyle);

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

void testNotImplementedLayerStyle(AbstractLayereStyle *layerStyle)
{
     QString testError;
    testError = QString("The layer style is expected to be of type NotImpleneted");
     QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::notImplemented, testError.toUtf8());

}


//Test the parsing functionality of the StyleSheet class.
void UnitTesting::parseSheet_returns_basic_values()
{
    //Check that the file exists.
    QString path = ":/unitTestResources/styleTest.json";
    bool fileExist = QFile::exists(path);
    QString fileExistsError = "File \"" + path + "\" does not exist";
    QVERIFY2(fileExist == true, fileExistsError.toUtf8());

    //Open the json file and check that the operation was succesful.
    QFile styleFile(path);
    bool fileOpened = styleFile.open(QIODevice::ReadOnly);
    QString fileOpenError = "Could not open file";
    QVERIFY2(fileOpened == true, fileOpenError.toUtf8());

    //Parse the json file into a QJsonDocument for further processing.
    QJsonDocument doc;
    QJsonParseError parseError;
    doc = QJsonDocument::fromJson(styleFile.readAll(), &parseError);

    //Check for parsing errors.
    QString parErrorString = "The Qt parser encountered an error";
    QVERIFY2(parseError.error == QJsonParseError::NoError, parErrorString.toUtf8());

    QString testError;
    StyleSheet sheet;
    sheet.parseSheet(doc);

    QString expectedId = "basic-v2";
    QString expectedName = "Basic";
    int expectedVersion = 8;
    int expectedNumberOfLayers = 5;

    testError = QString("The style Sheet object id does not match, expected %1 but got %2")
                    .arg(expectedId)
                    .arg(sheet.m_id);
    QVERIFY2(sheet.m_id == expectedId, testError.toUtf8());

    testError = QString("The style Sheet object version does not match, expected %1 but got %2")
                    .arg(expectedVersion)
                    .arg(sheet.m_version);
    QVERIFY2(sheet.m_version == expectedVersion, testError.toUtf8());


    testError = QString("The style Sheet object name does not match, expected %1 but got %2")
                    .arg(expectedName)
                    .arg(sheet.m_name);
    QVERIFY2(sheet.m_name == expectedName, testError.toUtf8());


    testError = QString("The style Sheet object does not contain the correct amount of layers, expected %1 but got %2")
                    .arg(expectedNumberOfLayers)
                    .arg(sheet.m_layerStyles.length());
    QVERIFY2(sheet.m_layerStyles.length() == expectedNumberOfLayers, testError.toUtf8());

    for(int i = 0; i < sheet.m_layerStyles.length(); i++){
        testError = QString("The style layer pointer at index %1 is invalid").arg(i);
        QVERIFY2(sheet.m_layerStyles.at(i) != nullptr, testError.toUtf8());
    }

    testBackgroundLayerStyle(sheet.m_layerStyles.at(0));
    testFillLyerStyle(sheet.m_layerStyles.at(1));
    testLineLayerStyle(sheet.m_layerStyles.at(2));
    testPointLayerStyle(sheet.m_layerStyles.at(3));
    testNotImplementedLayerStyle(sheet.m_layerStyles.at(4));

}
