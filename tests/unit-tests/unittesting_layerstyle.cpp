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
