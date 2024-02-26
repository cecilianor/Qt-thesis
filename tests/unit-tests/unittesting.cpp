#include <QTest>

#include "TileURL.h"
#include "Rendering.h"

#include <QObject>

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:
    void readKey_returns_success_when_valid_key();
    void readKey_returns_failure_when_invalid_key();
    void getStyleSheet_returns_success_on_supported_stylesheet();
    void getStyleSheet_returns_failure_on_unsupported_stylesheet();
    void longLatToWorldNormCoordDegrees_returns_expected_basic_values();
    //void renderingTests();
};

QTEST_MAIN(UnitTesting)
// This include needs to match the name of this .cpp file.
// Nils: I think it might not be needed when the class is defined in a .h file?
#include "unittesting.moc"

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

void UnitTesting::longLatToWorldNormCoordDegrees_returns_expected_basic_values()
{
    constexpr double epsilon = 0.001;
    auto comparePair = [](const QPair<double, double> &a, const QPair<double, double> &b) {
        if (std::abs(a.first - b.first) > epsilon)
            return false;
        else if (std::abs(a.second - b.second) > epsilon)
            return false;
        return true;
    };

    struct TestItem {
        QPair<double, double> input;
        QPair<double, double> expectedOut;
    };
    TestItem items[] = {
        { {0, 0}, {0.5, 0.5} },
        { {-180, 0}, {0, 0.5} },
        { {-90, 0}, {0.25, 0.5} },
        { {90, 0}, {0.75, 0.5} },
        { {180, 0}, {1, 0.5} },
    };

    for (const auto &item : items) {
        auto out = Bach::lonLatToWorldNormCoordDegrees(item.input.first, item.input.second);

        auto descr = QString("Input (%1, %2) did not match expected output (%3, %4). Instead got (%5, %6).")
            .arg(item.input.first)
            .arg(item.input.second)
            .arg(item.expectedOut.first)
            .arg(item.expectedOut.second)
            .arg(out.first)
            .arg(out.second)
            .toUtf8();

        QVERIFY2(comparePair(out, item.expectedOut), descr.constData());
    }
}

/// Rendering tests
//  Nils: ?
/*
void TestGetTile::renderingTests() {
    auto result = Bach::CalcVisibleTiles(0.5, 0.5, 0.5, 0, 0);
    auto expected = QVector<TileCoord>{{0, 0, 0}};

    QVERIFY(result == expected);
}
*/
