#include "unittesting.h"

#include "Layerstyle.h"


void UnitTesting::interpolate_returns_basic_expected_values() {
    /*
    struct TestItem {
        QPair<int, int> inputA;
        QPair<int, int> inputB;
        int inputViewportZoom;
        int expectedOut;
    };

    QVector<TestItem> testItems = {
    {
       {1, 1},
       {9, 2},
       15,
       2
    },
    {
        {1, 1},
        {9, 2},
        1,
        1
    },
    };

    for (int i = 0; i < testItems.size(); i++) {
        const auto &item = testItems[i];

        auto result = Bach::interpolate<int>(item.inputA, item.inputB, item.inputViewportZoom);
        auto errorMsg = QString("At item #%1. Expected %2, but got %3")
            .arg(i)
            .arg(item.expectedOut)
            .arg(result);

        QVERIFY2(result == item.expectedOut, errorMsg.toUtf8());
    }
*/
}

void UnitTesting::getLerpedValue_returns_basic_expceted_value()
{
    /*
    struct TestItem {
        QVector<QPair<int, int>> inputStops;
        int inputViewportZoom;
        int expectedOut;
    };

    QVector<TestItem> testItems = {
        {
            { {1, 1}, {9, 2} },
            10,
            2
        },
        {
            { {0, 1}, {9, 2} },
            0,
            1
        },
    };

    for (int i = 0; i < testItems.size(); i++) {
        const auto &item = testItems[i];

        auto result = Bach::getLerpedValue<int>(item.inputStops, item.inputViewportZoom);

        auto errorMsg = QString("At item #%1. Expected %2, but got %3")
            .arg(i)
            .arg(item.expectedOut)
            .arg(result);

        QVERIFY2(result == item.expectedOut, errorMsg.toUtf8());
    }
*/
}
