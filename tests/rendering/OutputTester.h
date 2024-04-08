#ifndef RENDERING_OUTPUT_TESTER_H
#define RENDERING_OUTPUT_TESTER_H

#include <QFont>
#include <QImage>
#include <QVector>

#include "TileCoord.h"

#include <functional>

#ifndef BACH_RENDEROUTPUT_BASELINE_DIR
#error "C++ define 'BACH_RENDEROUTPUT_BASELINE_DIR' was not defined. This likely means a build error."
#endif

namespace Bach::OutputTester {
    const int baseImageSize = 1024;

    struct TestItem {
        double vpX;
        double vpY;
        double vpZoom;
        int mapZoom;
        bool drawFill;
        bool drawLines;
        bool drawText;
        QVector<TileCoord> coords;
        bool autoCalcVisibleTiles = false;
        int imageWidth = baseImageSize;
        int imageHeight = baseImageSize;

        double imageAspect() const { return (double)imageWidth / (double)imageHeight; }
    };

    QVector<TileCoord> genTileCoordList(int zoom, int minX, int maxX, int minY, int maxY);

    std::optional<QFont> loadFont();

    using ProcessTestCaseFnT = std::function<void(
        int testId,
        const TestItem &testItem,
        const QImage &image)>;
    bool iterateOverTestCases(
        const QFont &font,
        const ProcessTestCaseFnT &processFn);

    QString buildBaselinePath();
    QString buildBaselineExpectedOutputPath();
    QString buildBaselineExpectedOutputPath(int testId);
    QString getStyleSheetPath();

    // We have issues with consistency between Linux and Windows
    // when rendering text.
    // Temporary solution is to render everything but text with low diff threshold.
    // We render text alone with higher diff threshold.
    inline const QVector<TestItem> testItems = {
        // First one is an example that should draw only background.
        TestItem {
            0, // vpX
            0, // vpY
            0, // vpZoom
            1, // mapZoom
            true, // drawFill
            true, // drawLines
            false, // drawText
            {}, // coords
            false, // autoCalcVisibleTiles
        },
        TestItem {
            0.5,
            0.5,
            0,
            0,
            true,
            true,
            false,
            {},
            true,
        },
        TestItem {
            0.5,
            0.5,
            0,
            1,
            true,
            true,
            false,
            {},
            true,
        },
        TestItem {
            0.5,
            0.5,
            0,
            1,
            true,
            true,
            false,
            {
                { 1, 0, 0 }
            }
        },
        TestItem {
            0.5,
            0.5,
            1,
            1,
            true,
            true,
            false,
            {},
            true
        },
        TestItem {
            0.25,
            0.25,
            0.5,
            1,
            true,
            true,
            false,
            {},
            true,
        },
        TestItem {
            0.25,
            0.25,
            0.5,
            1,
            true,
            true,
            false,
            {},
            true,
            (int)(baseImageSize / 0.5),
        },
        TestItem {
            0.25,
            0.25,
            0,
            1,
            true,
            true,
            false,
            {},
            true,
        },
        // Render only text
        TestItem {
            0.5,
            0.5,
            0,
            1,
            false,
            false,
            true,
            {},
            true,
        },
    };
}

#endif // RENDERING_OUTPUT_TESTER_H
