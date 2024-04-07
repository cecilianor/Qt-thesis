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

    QVector<TileCoord> genTileCoordList(int zoom, int minX, int maxX, int minY, int maxY);

    std::optional<QFont> loadFont();
    bool test(
        const QFont &font,
        const std::function<void(int, const QImage &image)> &fn);

    QString buildBaselinePath();
    QString buildBaselineExpectedOutputPath();
    QString buildBaselineExpectedOutputPath(int testId);
    QString getStyleSheetPath();

    struct TestItem {
        double vpX;
        double vpY;
        double vpZoom;
        int mapZoom;
        QVector<TileCoord> coords;
        bool autoCalcVisibleTiles = false;
        int imageWidth = baseImageSize;
        int imageHeight = baseImageSize;

        double imageAspect() const { return (double)imageWidth / (double)imageHeight; }
    };

    const QVector<TestItem> testItems = {
        // First one is an example that should draw only background.
        TestItem {
            0, // vpX
            0, // vpY
            0, // vpZoom
            1, // mapZoom
            {}, // coords
            false, // autoCalcVisibleTiles
        },
        TestItem {
            0.5,
            0.5,
            0,
            1,
            {
                { 1, 0, 0 }
            }
        },
        TestItem {
            0.5,
            0.5,
            1,
            1,
            {},
            true
        },
        TestItem {
            0.25,
            0.25,
            0.5,
            1,
            {},
            true,
        },
        TestItem {
            0.25,
            0.25,
            0.5,
            1,
            {},
            true,
            (int)(baseImageSize / 0.5),
        },
    };
}

#endif // RENDERING_OUTPUT_TESTER_H
