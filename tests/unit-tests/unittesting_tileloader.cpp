#include <QTest>

#include "unittesting.h"
#include "TileLoader.h"
#include "Utilities.h"

#include <QObject>
#include <QJsonDocument>

QTEST_MAIN(UnitTesting)
// This include needs to match the name of this .cpp file.

// Try to get a key that's correct
void UnitTesting::readKey_returns_success_when_valid_key() {
    TileLoader tileLoader;

    QString keyFromFile = tileLoader.readKey("testkey.txt");
    QString keyString ="123*+abcDEF<>";

    QVERIFY(keyFromFile==keyString);
}

// Try to get a key that's wrong
void UnitTesting::readKey_returns_failure_when_invalid_key() {
    TileLoader tileLoader;

    QString keyFromFile = tileLoader.readKey("testkey.txt");
    QString wrongKey ="IAmWrong";       //correct key = 123*+abcDEF<>

    QVERIFY(keyFromFile!=wrongKey);
}

/// Tests of getting styleshehets
// Get a supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_success_on_supported_stylesheet() {
    NetworkController networkController;

    TileLoader tileLoader;
    QString key = tileLoader.readKey("key.txt");

    HttpResponse styleSheetURL =
        tileLoader.getStylesheet(StyleSheetType::basic_v2, key);

    QVERIFY(styleSheetURL.resultType == ResultType::success);
}

// Get a non-supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_failure_on_unsupported_stylesheet() {
    NetworkController networkController;
    TileLoader tileLoader;
    QString key = tileLoader.readKey("key.txt");

    HttpResponse styleSheetURL =
        tileLoader.getStylesheet(StyleSheetType::bright_v2, key);

    QVERIFY(styleSheetURL.resultType == ResultType::noImplementation);
}

// Test the getTilesLink function with a valid style sheet containing the specified source type
void UnitTesting::getTilesLink_valid_style_sheet_returns_success() {
    // Create a TileLoader instance
    TileLoader tileLoader;

    // Create a valid JSON style sheet with the specified source type
    QJsonObject sourcesObject;
    QJsonObject sourceTypeObject;
    sourceTypeObject["url"] = "https://example.com/tiles";
    sourcesObject["maptiler_planet"] = sourceTypeObject;
    QJsonObject jsonObject;
    jsonObject["sources"] = sourcesObject;
    QJsonDocument styleSheet(jsonObject);

    // Call the function with the valid style sheet and source type
    ParsedLink parsedLink = tileLoader.getTilesLink(styleSheet, "maptiler_planet");

    // Verify that the parsed link and result type are as expected
    QCOMPARE(parsedLink.link, QString("https://example.com/tiles"));
    QCOMPARE(parsedLink.resultType, ResultType::success);
}

// Test the getTilesLink function with an unknown source type
void UnitTesting::getTilesLink_unknown_source_type_returns_unknown_source_type_error() {
    // Create a TileLoader instance
    TileLoader tileLoader;
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
    ParsedLink parsedLink = tileLoader.getTilesLink(styleSheet, unknownType);

    // Verify that the result type is unknown source type
    QCOMPARE(parsedLink.resultType, ResultType::unknownSourceType);
}

// Test the getTilesLink function with a style sheet missing the URL for the specified source type
void UnitTesting::getTilesLink_missing_url_returns_tile_sheet_not_found_error() {
    // Create a TileLoader instance
    TileLoader tileLoader;

    // Create a valid JSON style sheet missing the URL for the specified source type
    QJsonObject sourcesObject;
    QJsonObject sourceTypeObject;
    sourcesObject["maptiler_planet"] = sourceTypeObject;
    QJsonObject jsonObject;
    jsonObject["sources"] = sourcesObject;
    QJsonDocument styleSheet(jsonObject);

    // Call the function with the style sheet
    ParsedLink parsedLink = tileLoader.getTilesLink(styleSheet, "maptiler_planet");

    // Verify that the result type is tile sheet not found
    QCOMPARE(parsedLink.resultType, ResultType::tileSheetNotFound);
}


