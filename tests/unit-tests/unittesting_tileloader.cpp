#include <QTest>

#include "unittesting.h"
#include "TileLoader.h"
#include "Utilities.h"

#include <QObject>

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

    QVERIFY(styleSheetURL.resultType == ResultType::unknownError);
}

