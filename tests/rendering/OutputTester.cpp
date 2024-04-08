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

/*!
 * \brief buildBaselinePath
 * \return The path to the baseline folder. The path can be expected
 * to be inside the repository folder.
 */
QString OutputTester::buildBaselinePath()
{
    return BACH_RENDEROUTPUT_BASELINE_DIR;
}

/*!
 * \brief buildBaselineExpectedOutputPath
 * \return The path to the baseline folder where expected output will land.
 * The path can be expected to be inside the repository folder.
 */
QString OutputTester::buildBaselineExpectedOutputPath()
{
    return buildBaselinePath() + QDir::separator() + "expected_output";
}

/*!
 * \brief buildBaselineExpectedOutputPath
 * \param testId
 * \return The path to the baseline folder where the expected output file will land.
 * The path can be expected to be inside the repository folder.
 */
QString OutputTester::buildBaselineExpectedOutputPath(int testId)
{
    return buildBaselineExpectedOutputPath() + QDir::separator() + QString::number(testId) + QString(".png");
}

/*!
 * \brief getStyleSheetPath
 * \return Returns the path to the predetermined stylesheet
 */
QString OutputTester::getStyleSheetPath()
{
    return buildBaselinePath() + QDir::separator() + "/styleSheet.json";
}

/*!
 * \brief OutputTester::loadFont
 * Loads the predetermined font from file.
 * \return The QFont if success. Nullopt if something fails.
 */
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

/*!
 * \brief OutputTester::loadFont
 * Loads the predetermined stylesheet from file.
 * \return The StyleSheet if success. Nullopt if something fails.
 */
SimpleResult<StyleSheet> OutputTester::loadStylesheet()
{
    std::optional<StyleSheet> styleSheetResult =
        StyleSheet::fromJsonFile(OutputTester::getStyleSheetPath());

    if (!styleSheetResult.has_value()) {
        return SimpleError{ "Failed to parse JSON into StyleSheet object." };
    }

    return std::move(styleSheetResult.value());
}

// Container that maps TileCoord to a uniquely owned and uniquely allocated VectorTile.
using TileMapT = std::map<TileCoord, std::unique_ptr<VectorTile>>;
/*!
 * \internal
 * \brief loadTiles
 * Loads all the tiles passed by the parameter.
 * \param tileCoords
 * \return The map of tiles if successful. An error message if not.
 */
static SimpleResult<TileMapT> loadTiles(QVector<TileCoord> tileCoords) {
    TileMapT out;

    // Iterate over the TileCoords we want and try to load their respective
    // vector tile files.
    for (TileCoord tileCoord : tileCoords) {

        QString path = OutputTester::buildBaselinePath() + QDir::separator() + QString("z%1x%2y%3.mvt")
            .arg(tileCoord.zoom)
            .arg(tileCoord.x)
            .arg(tileCoord.y);

        std::optional<VectorTile> tileResult = VectorTile::fromFile(path);
        if (!tileResult.has_value()) {
            return SimpleError{ QString("Failed to load file '%1' into VectorTile object.").arg(path) };
        }

        out.insert({
            tileCoord,
            std::make_unique<VectorTile>(std::move(tileResult.value())) });
    }
    return out;
}

/*!
 * \internal
 * \brief stringFromJson
 * Helper function to generate string from a JSON value,
 * or return an error value if not possible.
 */
static SimpleResult<QString> stringFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a string" };
    if (jsonVal.type() != QJsonValue::Type::String) { return err; }
    return jsonVal.toString();
}

/*!
 * \internal
 * \brief boolFromJson
 * Helper function to generate bool from a JSON value,
 * or return an error value if not possible.
 */
static SimpleResult<bool> boolFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a boolean" };
    if (jsonVal.type() != QJsonValue::Type::Bool) { return err; }
    return jsonVal.toBool();
}

/*!
 * \internal
 * \brief posIntFromJson
 * Helper function to generate positive integer from a JSON value,
 * or return an error value if not possible.
 *
 * Note: Will fail if the JSON source is a double and not an integer.
 */
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

/*!
 * \internal
 * \brief doubleFromJson
 * Helper function to generate a double from a JSON value,
 * or return an error value if not possible.
 */
static SimpleResult<double> doubleFromJson(QJsonValueConstRef jsonVal)
{
    SimpleError err = { "JSON value is not a boolean" };
    if (jsonVal.type() != QJsonValue::Type::Double) { return err; }
    return jsonVal.toDouble();
}

/*!
 * \brief loadCoordsFromJson
 * Attempts to loads the viewport coordinates from given QJsonOject into the testItem argument.
 *
 * \param testItemJson The test-item in QJson form.
 * \param testItem
 * \return Returns success if no problem was encountered.
 * Otherwise returns with error message.
 */
SimpleResult<void> TestItem::loadCoordsFromJson(const QJsonObject &testItemJson)
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

    this->vpX = normCoords.first;
    this->vpY = normCoords.second;

    return {};
}

/*!
 * \internal
 * \brief tileCoordListFromJson
 * Tries to generate a list of TileCoord objects from given JSON value.
 *
 * \param jsonVal
 * \return Returns the list if successful. Returns an error message if not.
 */
static SimpleResult<QVector<TileCoord>> tileCoordListFromJson(const QJsonValueConstRef &jsonVal)
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

        // Load zoom member.
        SimpleResult<int> result = posIntFromJson(tileCoordJsonArr[0]);
        if (!result.success) { return err; }
        outTile.zoom = result.value;

        // Load X member.
        result = posIntFromJson(tileCoordJsonArr[1]);
        if (!result.success) { return err; }
        outTile.x = result.value;

        // Load Y member.
        result = posIntFromJson(tileCoordJsonArr[2]);
        if (!result.success) { return err; }
        outTile.y = result.value;

        out.push_back(outTile);
    }

    return out;
}

/*!
 * \brief testItemSaneDefaults
 * Returns a TestItem with members set to what it considered
 * sane defaults for this system.
 * \return The TestItem object.
 */
static TestItem testItemSaneDefaults()
{
    TestItem out{};
    out.vpX = 0.5;
    out.vpY = 0.5;
    out.mapZoom = 0;
    out.drawFill = true;
    out.drawLines = true;
    return out;
}

/*!
 * \brief validateTestItemJsonKeys
 * Checks that all keys of the QJsonObject are recognized as valid keys
 * for a TestItem.
 * \param jsonObj
 * \return Success if no issues. Otherwise returns error message
 * that includes unrecognized key.
 */
static SimpleResult<void> validateTestItemJsonKeys(const QJsonObject &jsonObj) {
    for (const QString &foundKey : jsonObj.keys()) {
        bool found = false;
        for (const QString &knownKey : TestItem::knownJsonKeys()) {
            if (foundKey == knownKey) {
                found = true;
                break;
            }
        }
        if (!found) {
            return SimpleError{ QString("Unrecognized key '%1'").arg(foundKey) };
        }
    }
    return {};
}

/*!
 * \brief loadTestItemsFromJson
 * Tries to load a list of TestItems from a JSON document.
 *
 * \param json
 * \return Returns the list of TestItems if succesful. Returns an error message if not.
 */
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

        // Check that there are no unrecognized keys.
        {
            SimpleResult<void> result = validateTestItemJsonKeys(testItemJson);
            if (!result.success) {
                return SimpleError { result.errorMsg };
            }
        }

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
        SimpleResult<void> coordGetResult = outItem.loadCoordsFromJson(testItemJson);
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
                // Calculate all visible tiles based on the test-item configuration
                outItem.tileCoords = Bach::calcVisibleTiles(
                    outItem.vpX,
                    outItem.vpY,
                    outItem.imageAspect(),
                    outItem.vpZoom,
                    outItem.mapZoom);
            } else {
                SimpleResult<QVector<TileCoord>> tileCoordsResult = tileCoordListFromJson(*it);
                if (!tileCoordsResult.success) {
                    return SimpleError{ tileCoordsResult.errorMsg };
                }
                outItem.tileCoords = tileCoordsResult.value;
            }
        }

        out.push_back(outItem);
    }
    return out;
}

/*!
 * \brief loadTestItems
 * Tries to load all the TestItems from predetermined JSON file.
 * \return Returns the list of TestItems if successful. Returns an error message
 * if not successful.
 */
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
    SimpleResult<QVector<TestItem>> result = loadTestItemsFromJson(jsonDoc);
    if (!result.success) {
        return SimpleError{ QString("When parsing test-items JSON: ") + result.errorMsg };
    }
    return result;
}

/*!
 * \brief render
 * Renders a specific TestItem and outputs the resulting QImage if successful.
 *
 * \param item
 * \param stylesheet The stylesheet to use for rendering.
 * \param font The font to use for rendering.
 * \return Returns the final QImage if successful. Returns an error message
 * if not successful.
 */
SimpleResult<QImage> OutputTester::render(
    const TestItem &item,
    const StyleSheet &stylesheet,
    const QFont &font)
{
    SimpleResult<TileMapT> tileMapResult = loadTiles(item.tileCoords);
    if (!tileMapResult.success) { return SimpleError{ tileMapResult.errorMsg }; }
    const TileMapT &tileMap = tileMapResult.value;

    // Build a map that the painting function can accept.
    QMap<TileCoord, const VectorTile*> tempTiles;
    for (auto& [coord, tile] : tileMap) {
        tempTiles.insert(coord, tile.get());
    }

    // Create the QImage we want to render into.
    QImage generatedImg { item.imageWidth, item.imageHeight, QImage::Format_ARGB32};
    QPainter painter{ &generatedImg };
    painter.setFont(font);

    // Set up the render settings.
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
