#include "unittesting.h"

#include "Layerstyle.h"


void UnitTesting::getStopOutput_returns_basic_values(){
    QList<QPair<int, float>> stops({{4,0.8},{9, 1.1}, {11, 1.75}, {18, 2.5},{22, 2.72}});
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

    testError = QString("The layer style is expected to be of type BackgroundLayerStyle");
    QVERIFY2(layerStyle->type() == AbstractLayereStyle::LayerType::background, testError.toUtf8());
    auto const& backgroundStyle = *static_cast<BackgroundStyle const*>(layerStyle);

    testError =  QString("The layerStyle id does not match, expected %1 but got %2")
                    .arg("Background")
                    .arg(backgroundStyle.m_id);
    QVERIFY2(backgroundStyle.m_id == "Background", testError.toUtf8());

    testError =  QString("The layerStyle visibility does not match, expected %1 but got %2")
                    .arg("visible")
                    .arg(backgroundStyle.m_visibility);
    QVERIFY2(backgroundStyle.m_visibility == "visible", testError.toUtf8());

    testError =  QString("The layerStyle minZoom does not match, expected %1 but got %2")
                    .arg(0)
                    .arg(backgroundStyle.m_minZoom);
    QVERIFY2(backgroundStyle.m_minZoom == 0, testError.toUtf8());

    testError =  QString("The layerStyle maxZoom does not match, expected %1 but got %2")
                    .arg(24)
                    .arg(backgroundStyle.m_maxZoom);
    QVERIFY2(backgroundStyle.m_maxZoom == 24, testError.toUtf8());


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

void UnitTesting::parseSheet_returns_basic_values()
{
    QString path = ":/unitTestResources/styleTest.json";
    bool fileExist = QFile::exists(path);
    QString fileExistsError = "File \"" + path + "\" does not exist";
    QVERIFY2(fileExist == true, fileExistsError.toUtf8());

    QFile styleFile(path);
    bool fileOpened = styleFile.open(QIODevice::ReadOnly);
    QString fileOpenError = "Could not open file";
    QVERIFY2(fileOpened == true, fileOpenError.toUtf8());

    QJsonDocument doc;
    QJsonParseError parseError;
    doc = QJsonDocument::fromJson(styleFile.readAll(), &parseError);
    QString parErrorString = "The Qt parser encountered an error";
    QVERIFY2(parseError.error == QJsonParseError::NoError, parErrorString.toUtf8());

    QString testError;
    StyleSheet sheet;
    sheet.parseSheet(doc);

    QString expectedId = "basic-v2";
    QString expectedName = "Basic";
    int expectedVersion = 8;
    int expectedNumberOfLayers = 4;

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

    testBackgroundLayerStyle(sheet.m_layerStyles.at(0));

}
