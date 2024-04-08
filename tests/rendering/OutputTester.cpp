#include "OutputTester.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QJsonDocument>

#include "Rendering.h"
#include "VectorTiles.h"

namespace OutputTester = Bach::OutputTester;
using TestItem = OutputTester::TestItem;
using OutputTester::SimpleResult;
using OutputTester::SimpleError;

QString OutputTester::buildBaselinePath()
{
    return BACH_RENDEROUTPUT_BASELINE_DIR;
}

QString OutputTester::buildBaselineExpectedOutputPath()
{
    return buildBaselinePath() + QDir::separator() + "expected_output";
}

QString OutputTester::buildBaselineExpectedOutputPath(int testId)
{
    return buildBaselineExpectedOutputPath() + QDir::separator() + QString::number(testId) + QString(".png");
}

QString OutputTester::getStyleSheetPath()
{
    return buildBaselinePath() + QDir::separator() + "/styleSheet.json";
}

std::optional<QFont> OutputTester::loadFont()
{
    QFontDatabase::removeAllApplicationFonts();
    int fontId = QFontDatabase::addApplicationFont(
        OutputTester::buildBaselinePath() +
        QDir::separator() +
        "RobotoMono-Regular.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont font = fontFamilies.first();
        QGuiApplication::setFont(font);
        return font;
    } else {
        return std::nullopt;
    }
}

SimpleResult<StyleSheet> OutputTester::loadStylesheet()
{
    QFile styleSheetFile { OutputTester::getStyleSheetPath() };
    styleSheetFile.open(QFile::ReadOnly);

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return SimpleError{
            QString("Failed to parse vector stylesheet file into JSON: ") +
            parseError.errorString() };
    }

    std::optional<StyleSheet> styleSheetResult = StyleSheet::fromJson(styleSheetJson);
    if (!styleSheetResult.has_value()) {
        return SimpleError{ "Failed to parse JSON into StyleSheet object." };
    }
    return std::move(styleSheetResult.value());
}

using TileMapT = std::map<TileCoord, std::unique_ptr<VectorTile>>;
static SimpleResult<TileMapT> loadTiles(QVector<TileCoord> tileCoords) {
    TileMapT tileStorage;
    for (TileCoord tileCoord : tileCoords) {
        // Load each tile from file.
        QString path = OutputTester::buildBaselinePath() + QDir::separator() + QString("z%1x%2y%3.mvt")
            .arg(tileCoord.zoom)
            .arg(tileCoord.x)
            .arg(tileCoord.y);

        QFile vectorFile { path };
        bool fileOpenSuccess = vectorFile.open(QFile::ReadOnly);
        if (!fileOpenSuccess) {
            return SimpleError{ QString("Unable to open file: %1").arg(path) };
        }

        std::optional<VectorTile> tileResult = VectorTile::fromByteArray(vectorFile.readAll());
        if (!tileResult.has_value()) {
            return SimpleError{ QString("Failed to parse file '%1' into VectorTile object.").arg(path) };
        }
        tileStorage.insert({
            tileCoord,
            std::make_unique<VectorTile>(std::move(tileResult.value())) });
    }
    return tileStorage;
}

static SimpleResult<QString> stringFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a string" };
    if (jsonVal.type() != QJsonValue::Type::String) { return err; }
    return jsonVal.toString();
}

static SimpleResult<bool> boolFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a boolean" };
    if (jsonVal.type() != QJsonValue::Type::Bool) { return err; }
    return jsonVal.toBool();
}

static SimpleResult<int> posIntFromJson(QJsonValueConstRef jsonVal)
{
    QString errMsg = "JSON value is not a positive integer";
    if (jsonVal.type() != QJsonValue::Type::Double) {
        return SimpleError{ errMsg };
    }

    double tempFloat = jsonVal.toDouble();
    int tempInt = jsonVal.toInteger();
    // Check that our value has no decimal component.
    if ((double)tempInt != tempFloat) {
        return SimpleError{ errMsg };
    }

    if (tempInt < 0) {
        return SimpleError{ errMsg };
    }

    return tempInt;
}

static SimpleResult<double> doubleFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a boolean" };
    if (jsonVal.type() != QJsonValue::Type::Double) { return err; }
    return jsonVal.toDouble();
}

using TestItem = OutputTester::TestItem;

static SimpleResult<void> loadTestItemCoords(const QJsonObject &testItemJson, TestItem& testItem)
{
    SimpleError err = { "JSON value is not valid viewpo coordinate" };

    auto it = testItemJson.find(TestItem::coordsJsonKey());
    if (it == testItemJson.end()) {
        return {};
    }
    QJsonValueConstRef coordJson = *it;
    if (!coordJson.isArray()) { return err; }

    QJsonArray coordJsonArr = coordJson.toArray();
    // Check length
    if (coordJsonArr.size() != 2) { return err; };

    SimpleResult<double> result = doubleFromJson(coordJsonArr[0]);
    if (!result.success || result.value > 180 || result.value < -180) { return err; }
    double longitude = result.value;

    result = doubleFromJson(coordJsonArr[1]);
    if (!result.success || result.value > 90 || result.value < -90) { return err; }
    double latitude = result.value;

    QPair<double, double> normCoords = Bach::lonLatToWorldNormCoordDegrees(longitude, latitude);

    testItem.vpX = normCoords.first;
    testItem.vpY = normCoords.second;

    return {};
}

SimpleResult<QVector<TileCoord>> tileCoordListFromJson(const QJsonValueConstRef &jsonVal)
{
    SimpleError err = { "JSON value is not a valid TileCoord list" };

    QVector<TileCoord> out;
    if (!jsonVal.isArray()) { return err; }
    QJsonArray jsonArr = jsonVal.toArray();

    // Check that each element is another array with size 3, and check that each only contains integers.
    for (QJsonValueConstRef jsonArrItem : jsonArr) {
        if (!jsonArrItem.isArray()) { return err; }
        TileCoord outTile;

        QJsonArray tileCoordJsonArr = jsonArrItem.toArray();
        if (tileCoordJsonArr.size() != 3) { return err; }

        SimpleResult<int> result = posIntFromJson(tileCoordJsonArr[0]);
        if (!result.success) { return err; }
        outTile.zoom = result.value;

         result = posIntFromJson(tileCoordJsonArr[1]);
        if (!result.success) { return err; }
        outTile.x = result.value;

        result = posIntFromJson(tileCoordJsonArr[2]);
        if (!result.success) { return err; }
        outTile.y = result.value;

        out.push_back(outTile);
    }

    return out;
}

TestItem testItemSaneDefaults()
{
    TestItem out{};
    out.vpX = 0.5;
    out.vpY = 0.5;
    out.mapZoom = 0;
    out.drawFill = true;
    out.drawLines = true;
    return out;
}

SimpleResult<QVector<TestItem>> loadTestItemsFromJson(const QJsonDocument &json)
{
    QVector<TestItem> out;

    if (!json.isArray()) {
        return SimpleError{ "Top level JSON object must be array." };
    }

    const QJsonArray &jsonArr = json.array();
    for (QJsonValueConstRef arrItem : jsonArr) {
        TestItem outItem = testItemSaneDefaults();

        if (!arrItem.isObject()) {
            return SimpleError{ "JSON array element must be a JSON object." };
        }
        const QJsonObject &testItemJson = arrItem.toObject();

        // Load name if any
        {
            auto it = testItemJson.find(TestItem::nameJsonKey());
            if (it != testItemJson.end()) {
                SimpleResult<QString> result = stringFromJson(*it);
                if (!result.success) {
                    return SimpleError{ result.errorMsg };
                }
                outItem.name = result.value;
            }
        }

        // Load coords if any
        SimpleResult<void> coordGetResult = loadTestItemCoords(testItemJson, outItem);
        if (!coordGetResult.success) {
            return SimpleError{ coordGetResult.errorMsg };
        }

        // Load "draw-fill"
        {
            auto it = testItemJson.find(TestItem::drawFillJsonKey());
            if (it != testItemJson.end()) {
                SimpleResult<bool> result = boolFromJson(*it);
                if (!result.success) {
                    return SimpleError{ result.errorMsg };
                }
                outItem.drawFill = result.value;
            }
        }

        // Load "draw-lines"
        {
            auto it = testItemJson.find(TestItem::drawLinesJsonKey());
            if (it != testItemJson.end()) {
                SimpleResult<bool> result = boolFromJson(*it);
                if (!result.success) {
                    return SimpleError{ result.errorMsg };
                }
                outItem.drawLines = result.value;
            }
        }

        // Load viewport zoom if any
        {
            auto it = testItemJson.find(TestItem::vpZoomJsonKey());
            if (it != testItemJson.end()) {
                // Load it as integer
                SimpleResult<int> result = posIntFromJson(*it);
                if (!result.success) {
                    return SimpleError{ result.errorMsg };
                }
                outItem.vpZoom = result.value;
            }
        }

        // Load map zoom if any
        {
            auto it = testItemJson.find(TestItem::mapZoomJsonKey());
            if (it != testItemJson.end()) {
                // Load it as integer
                SimpleResult<int> mapZoomResult = posIntFromJson(*it);
                if (!mapZoomResult.success) {
                    return SimpleError{ mapZoomResult.errorMsg };
                }
                outItem.mapZoom = mapZoomResult.value;
            }
        }
        // Load tiles if any
        {
            auto it = testItemJson.find(TestItem::tileListJsonKey());
            if (it == testItemJson.end()) {
                outItem.autoCalcVisibleTiles = true;
            } else {
                SimpleResult<QVector<TileCoord>> tileCoordsResult = tileCoordListFromJson(*it);
                if (!tileCoordsResult.success) {
                    return SimpleError{ tileCoordsResult.errorMsg };
                }
                outItem.coords = tileCoordsResult.value;
            }
        }

        out.push_back(outItem);
    }
    return out;
}

SimpleResult<QVector<TestItem>> OutputTester::loadTestItems() {
    QFile file { OutputTester::buildBaselinePath() + QDir::separator() + "testcases.json" };
    file.open(QFile::ReadOnly);

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return SimpleError{
            QString("Failed to parse testcases.json file into JSON with error: ") +
            parseError.errorString() };
    }
    return loadTestItemsFromJson(jsonDoc);
}

SimpleResult<void> OutputTester::iterateOverTestCases(
    const QFont &font,
    const OutputTester::ProcessTestCaseFnT &processFn)
{
    QFile styleSheetFile { OutputTester::getStyleSheetPath() };
    styleSheetFile.open(QFile::ReadOnly);

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return SimpleError {
            QString("Failed to parse vector stylesheet file into JSON") +
            parseError.errorString() };
    }

    std::optional<StyleSheet> styleSheetResult = StyleSheet::fromJson(styleSheetJson);
    if (!styleSheetResult.has_value()) {
        return SimpleError { "Failed to parse JSON into stylesheet object." };
    }
    const StyleSheet &styleSheet = styleSheetResult.value();

    SimpleResult<QVector<TestItem>> testItemsResult = loadTestItems();
    if (!testItemsResult.success) {
        return SimpleError { testItemsResult.errorMsg };
    }
    const QVector<TestItem> &testItems = testItemsResult.value;

    for (int i = 0; i < testItems.size(); i++) {
        const TestItem &testItem = testItems[i];

        SimpleResult<QImage> renderResult = render(
            testItem,
            styleSheet,
            font);
        if (!renderResult.success) {
            return SimpleError { renderResult.errorMsg };
        }
        const QImage& generatedImg = renderResult.value;

        processFn(i, testItem, generatedImg);
    }

    return {};
}

SimpleResult<QImage> OutputTester::render(
    const TestItem &item,
    const StyleSheet &stylesheet,
    const QFont &font)
{
    QVector<TileCoord> tileCoords = item.coords;
    if (item.autoCalcVisibleTiles) {
        tileCoords = Bach::calcVisibleTiles(
            item.vpX,
            item.vpY,
            item.imageAspect(),
            item.vpZoom,
            item.mapZoom);
    }

    SimpleResult<TileMapT> tileMapResult = loadTiles(tileCoords);
    if (!tileMapResult.success) { return SimpleError{ tileMapResult.errorMsg }; }
    const TileMapT &tileMap = tileMapResult.value;

    // Build a map that the painting function can accept.
    QMap<TileCoord, const VectorTile*> tempTiles;
    for (auto& [coord, tile] : tileMap) {
        tempTiles.insert(coord, tile.get());
    }

    QImage generatedImg { item.imageWidth, item.imageHeight, QImage::Format_ARGB32};
    QPainter painter{ &generatedImg };
    painter.setFont(font);

    Bach::PaintVectorTileSettings paintSettings = {};
    paintSettings.drawFill = item.drawFill;
    paintSettings.drawLines = item.drawLines;
    paintSettings.drawText = false;
    paintSettings.forceNoChangeFontType = true;
    Bach::paintVectorTiles(
        painter,
        item.vpX,
        item.vpY,
        item.vpZoom,
        item.mapZoom,
        tempTiles,
        stylesheet,
        paintSettings,
        false);

    return generatedImg;
}
