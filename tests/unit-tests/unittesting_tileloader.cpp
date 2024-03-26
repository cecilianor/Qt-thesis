#include <QJsonDocument>
#include <QObject>
#include <QTest>
#include <QTimer>

#include "TileLoader.h"
#include "Utilities.h"

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:
    void readKey_returns_success_when_valid_key();
    void readKey_returns_failure_when_invalid_key();
    void getStyleSheet_returns_success_on_supported_stylesheet();
    void getStyleSheet_returns_failure_on_unsupported_stylesheet();
    void getTilesLink_valid_style_sheet_returns_success();
    void getTilesLink_unknown_source_type_returns_unknown_source_type_error();
    void getTilesLink_missing_url_returns_tile_sheet_not_found_error();
    void loadTileFromCache_fails_on_broken_file();
    void loadTileFromCache_parses_cached_file_successfully();
    void check_new_tileLoader_has_no_tiles();
};

QTEST_MAIN(UnitTesting)
#include "unittesting_tileloader.moc"
// This include needs to match the name of this .cpp file.

// Try to get a key that's correct
void UnitTesting::readKey_returns_success_when_valid_key()
{
    std::optional<QString> keyFromFileResult = Bach::readMapTilerKey("testkey.txt");
    QVERIFY2(keyFromFileResult.has_value(), "Unable to load MapTiler key from file.");

    QString keyFromFile = keyFromFileResult.value();
    QString keyString ="123*+abcDEF<>";

    QVERIFY(keyFromFile == keyString);
}

// Try to get a key that's wrong
void UnitTesting::readKey_returns_failure_when_invalid_key()
{
    std::optional<QString> keyFromFileResult = Bach::readMapTilerKey("testkey.txt");
    QVERIFY2(keyFromFileResult.has_value(), "Unable to load MapTiler key from file.");

    QString keyFromFile = keyFromFileResult.value();
    QString wrongKey ="IAmWrong";       //correct key = 123*+abcDEF<>

    QVERIFY(keyFromFile != wrongKey);
}

/// Tests of getting styleshehets
// Get a supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_success_on_supported_stylesheet()
{
    std::optional<QString> keyFromFileResult = Bach::readMapTilerKey("testkey.txt");
    QVERIFY2(keyFromFileResult.has_value(), "Unable to load MapTiler key from file.");

    HttpResponse styleSheetURL = Bach::requestStyleSheetFromWeb(
        MapType::BasicV2,
        keyFromFileResult.value());

    QVERIFY(styleSheetURL.resultType == ResultType::Success);
}

// Get a non-supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_failure_on_unsupported_stylesheet()
{
    std::optional<QString> keyFromFileResult = Bach::readMapTilerKey("testkey.txt");
    QVERIFY2(keyFromFileResult.has_value(), "Unable to load MapTiler key from file.");

    HttpResponse styleSheetURL = Bach::requestStyleSheetFromWeb(
        MapType::BrightV2,
        keyFromFileResult.value());

    QVERIFY(styleSheetURL.resultType == ResultType::NoImplementation);
}

// Test the getTilesLink function with a valid style sheet containing the specified source type
void UnitTesting::getTilesLink_valid_style_sheet_returns_success()
{
    // Create a valid JSON style sheet with the specified source type
    QJsonObject sourcesObject;
    QJsonObject sourceTypeObject;
    sourceTypeObject["url"] = "https://example.com/tiles";
    sourcesObject["maptiler_planet"] = sourceTypeObject;
    QJsonObject jsonObject;
    jsonObject["sources"] = sourcesObject;
    QJsonDocument styleSheet(jsonObject);

    // Call the function with the valid style sheet and source type
    ParsedLink parsedLink = Bach::getTilesLinkFromStyleSheet(styleSheet, "maptiler_planet");

    // Verify that the parsed link and result type are as expected
    QCOMPARE(parsedLink.link, QString("https://example.com/tiles"));
    QCOMPARE(parsedLink.resultType, ResultType::Success);
}

// Test the getTilesLink function with an unknown source type
void UnitTesting::getTilesLink_unknown_source_type_returns_unknown_source_type_error()
{

    QString unknownType = ("random_string");
    // Create a valid JSON style sheet with a different source type
    QJsonObject sourcesObject;
    QJsonObject sourceTypeObject;
    sourceTypeObject["url"] = "https://example.com/tiles";
    sourcesObject["another_source_type"] = sourceTypeObject;
    QJsonObject jsonObject;
    jsonObject["sources"] = sourcesObject;
    QJsonDocument styleSheet(jsonObject);

    // Call the function with the style sheet and an unknown source type
    ParsedLink parsedLink = Bach::getTilesLinkFromStyleSheet(styleSheet, unknownType);

    // Verify that the result type is unknown source type
    QCOMPARE(parsedLink.resultType, ResultType::UnknownSourceType);
}

// Test the getTilesLink function with a style sheet missing the URL for the specified source type
void UnitTesting::getTilesLink_missing_url_returns_tile_sheet_not_found_error()
{
    // Create a valid JSON style sheet missing the URL for the specified source type
    QJsonObject sourcesObject;
    QJsonObject sourceTypeObject;
    sourcesObject["maptiler_planet"] = sourceTypeObject;
    QJsonObject jsonObject;
    jsonObject["sources"] = sourcesObject;
    QJsonDocument styleSheet(jsonObject);

    // Call the function with the style sheet
    ParsedLink parsedLink = Bach::getTilesLinkFromStyleSheet(styleSheet, "maptiler_planet");

    // Verify that the result type is tile sheet not found
    QCOMPARE(parsedLink.resultType, ResultType::TileSheetNotFound);
}

namespace Bach::UnitTesting {
    class TempDir {
    public:
        TempDir()
        {
            auto sep = QDir::separator();

            QString uniqueDirName = QUuid::createUuid().toString();
            QString tempDirPath =
                QDir::tempPath() + sep +
                "Qt_thesis_unit_test_files" + sep +
                uniqueDirName;

            _dir = tempDirPath;
        }
        TempDir(const TempDir&) = delete;
        TempDir(TempDir&&) = delete;

        ~TempDir() {
            QDir temp{ _dir };
            temp.removeRecursively();
        }

        const QString& path() const { return _dir; }

    private:
        QString _dir;
    };
}

// This test uses a predetermined cached file that is known to be corrupt.
// This test should make sure the TileLoader is able to catch this as a parsing error.
void UnitTesting::loadTileFromCache_fails_on_broken_file()
{
    const TileCoord expectedCoord = {0, 0, 0};

    // Create a unique temporary directory for this test.
    Bach::UnitTesting::TempDir tempDir;

    // Write our input file to the tile cache.
    QFile inputFile(":unitTestResources/loadTileFromCache_fails_on_broken_file.mvt");
    bool inputFileOpenResult = inputFile.open(QFile::ReadOnly);
    QVERIFY2(inputFileOpenResult == true, "Unable to open input file.");

    QByteArray inputFileBytes = inputFile.readAll();
    bool writeToCacheResult = Bach::writeTileToDiskCache(
        tempDir.path(),
        expectedCoord,
        inputFileBytes);
    QVERIFY2(writeToCacheResult == true, "Unable to write input file into tile cache.");

    auto tileLoaderPtr = TileLoader::newDummy(tempDir.path());
    TileLoader &tileLoader = *tileLoaderPtr;

    QEventLoop loop;
    QObject::connect(
        &tileLoader,
        &TileLoader::tileFinished,
        &loop,
        &QEventLoop::quit);

    // If loading has a bug somewhere, it might never get finished.
    // For now we just have a timeout in case something went wrong.
    QTimer::singleShot(
        5000,
        &loop,
        [&]() {
            QVERIFY2(false, "Test hit the timeout. This should never happen.");
            loop.quit();
        });

    tileLoader.requestTiles({ expectedCoord }, true);

    loop.exec();

    std::optional<Bach::LoadedTileState> tileStateResult = tileLoader.getTileState(expectedCoord);
    QVERIFY2(
        tileStateResult.has_value(),
        "TileLoader::getTileState returned nullopt when it was just reported to have finished loading.");

    Bach::LoadedTileState tileState = tileStateResult.value();
    QVERIFY2(
        tileState == Bach::LoadedTileState::ParsingFailed,
        "Expected loaded to be marked as parsing failed, but result was different.");
}

// This test uses a predetermined cached file that is known to be correct.
// This test should make sure the TileLoader is able to parse this.
void UnitTesting::loadTileFromCache_parses_cached_file_successfully()
{
    // Write our input file to the tile cache.
    QFile inputFile(":unitTestResources/loadTileFromCache_parses_cached_file_successfully.mvt");
    bool inputFileOpenResult = inputFile.open(QFile::ReadOnly);
    QVERIFY2(inputFileOpenResult == true, "Unable to open input file.");

    const TileCoord expectedCoord = {0, 0, 0};

    Bach::UnitTesting::TempDir tempDir;

    QByteArray inputFileBytes = inputFile.readAll();
    bool writeToCacheResult = Bach::writeTileToDiskCache(
        tempDir.path(),
        expectedCoord,
        inputFileBytes);
    QVERIFY2(writeToCacheResult == true, "Unable to write input file into tile cache.");

    auto tileLoaderPtr = TileLoader::newDummy(tempDir.path());
    TileLoader &tileLoader = *tileLoaderPtr;

    QEventLoop loop;

    // If loading has a bug somewhere, it might never get finished.
    // For now we just have a timeout in case something went wrong.
    QTimer::singleShot(
        3000,
        &loop,
        [&]() {
            QVERIFY2(false, "Timed out when loading tile.");
            loop.quit();
        });

    tileLoader.requestTiles(
        { expectedCoord },
        [&](TileCoord loadedCoord) {
            QVERIFY2(
                loadedCoord == expectedCoord,
                "Tile signal function was not signaled with correct tile coordinate.");

            // Stop the loop, so we can check the result of the tile.
            loop.quit();
        });

    loop.exec();

    auto tileStateResult = tileLoader.getTileState(expectedCoord);
    QVERIFY2(
        tileStateResult.has_value(),
        "TileLoader::getTileState returned nullopt when it was just reported to have finished loading.");
    auto tileState = tileStateResult.value();
    QVERIFY2(
        tileState == Bach::LoadedTileState::Ok,
        "Expected loaded to be marked as parsing OK, but result was different.");
}

void UnitTesting::check_new_tileLoader_has_no_tiles()
{
    auto tileLoaderPtr = TileLoader::newDummy("");
    TileLoader &tileLoader = *tileLoaderPtr;
    auto result = tileLoader.requestTiles({});
    auto &map = result->vectorMap();
    QVERIFY(map.size() == 0);
}
