#include <QTest>

#include "unittesting.h"

#include "TileURL.h"

#include <QObject>

QTEST_MAIN(UnitTesting)
// This include needs to match the name of this .cpp file.
// Nils: I think it might not be needed when the class is defined in a .h file?
//#include "unittesting.moc"

// Try to get a key that's correct
void UnitTesting::readKey_returns_success_when_valid_key() {
    TileURL tileURL;

    QString keyFromFile = tileURL.readKey("testkey.txt");
    QString keyString ="123*+abcDEF<>";

    QVERIFY(keyFromFile==keyString);
}

// Try to get a key that's wrong
void UnitTesting::readKey_returns_failure_when_invalid_key() {
    TileURL tileURL;

    QString keyFromFile = tileURL.readKey("testkey.txt");
    QString wrongKey ="IAmWrong";       //correct key = 123*+abcDEF<>

    QVERIFY(keyFromFile!=wrongKey);
}

/// Tests of getting styleshehets
// Get a supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_success_on_supported_stylesheet() {
    TileURL tileURL;
    QString key = tileURL.readKey("key.txt");

    std::pair<QByteArray, TileURL::ErrorCode> styleSheetURL =
        tileURL.getStylesheet(TileURL::styleSheetType::basic_v2, key);

    QVERIFY(styleSheetURL.second == TileURL::ErrorCode::success);
}

// Get a non-supported stylesheet
// Note that this specific test will fail if an illegal key is provided
void UnitTesting::getStyleSheet_returns_failure_on_unsupported_stylesheet() {
    TileURL tileURL;
    QString key = tileURL.readKey("key.txt");

    std::pair<QByteArray, TileURL::ErrorCode> styleSheetURL =
        tileURL.getStylesheet(TileURL::styleSheetType::bright_v2, key);

    QVERIFY(styleSheetURL.second == TileURL::ErrorCode::unknownError);
}
