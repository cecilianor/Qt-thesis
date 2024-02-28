#include "unittesting.h"

#include "Rendering.h"

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

/* We just want to see if the vector contains the same elements, order doesn't matter.
 *
 * Returns bool for success, and also an optional error message.
 */
static QPair<bool, QString> compareTileCoordLists(
    const QVector<TileCoord> &expected,
    const QVector<TileCoord> &result)
{
    if (expected.size() != result.size())
        return { false, "Lists don't have same length." };

    for (const auto &itemA : expected) {
        auto itemFoundInB = false;
        for (const auto &itemB : result) {
            if (itemA == itemB) {
                itemFoundInB = true;
                break;
            }
        }
        if (!itemFoundInB)
            return {
                false,
                QString("Item %1 not found in list of results.").arg(itemA.toString())
            };
    }

    return { true, "" };
}

void UnitTesting::calcVisibleTiles_returns_expected_basic_cases()
{
    auto generateRangeOfTiles = [](int zoom, int xMin, int xMax, int yMin, int yMax) {
        QVector<TileCoord> out;
        for (int x = xMin; x < xMax; x++) {
            for (int y = yMin; y < yMax; y++) {
                out.push_back({ zoom, x, y });
            }
        }
        return out;
    };

    struct TestItem {
        struct Input {
            double vpX;
            double vpY;
            double vpAspect;
            double vpZoomLevel;
            int mapZoomLevel;
        };
        Input input;
        QVector<TileCoord> expectedOut;
    };

    const QVector<TestItem> testItems = {
        {
            TestItem::Input{ 0.5, 0.5, 1.0, 0, 0 },
            {
                { 0, 0, 0 },
            }
        },
        {
            TestItem::Input{ 0.5, 0.5, 1.0, 1, 1 },
            generateRangeOfTiles(1, 0, 2, 0, 2)
        },
        {
            TestItem::Input{ 0.5, 0.5, 1.0, 0.25, 2 },
            generateRangeOfTiles(2, 0, 4, 0, 4)
        },
        {
            TestItem::Input{ 0.5, 0.5, 1.0, 2, 2 },
            generateRangeOfTiles(2, 1, 3, 1, 3)
        }
    };

    for (int i = 0; i < testItems.size(); i++) {
        const auto &item = testItems[i];
        const auto &input = item.input;
        auto result = Bach::calcVisibleTiles(
            input.vpX,
            input.vpY,
            input.vpAspect,
            input.vpZoomLevel,
            input.mapZoomLevel);

        auto [success, errorMsg] = compareTileCoordLists(item.expectedOut, result);
        errorMsg = QString("Test item %1: ").arg(i) + errorMsg;
        QVERIFY2(success, errorMsg.toUtf8());
    }
}
