#ifndef RENDERING_OUTPUT_TESTER_H
#define RENDERING_OUTPUT_TESTER_H

#include <QFont>
#include <QImage>
#include <QVector>

#include "TileCoord.h"
#include "LayerStyle.h"

#include <functional>

#ifndef BACH_RENDEROUTPUT_BASELINE_DIR
#error "C++ define 'BACH_RENDEROUTPUT_BASELINE_DIR' was not defined. This likely means a build error."
#endif

namespace Bach::OutputTester {
    const int baseImageSize = 1024;

    struct TestItem {
        static QString nameJsonKey() { return "name"; }
        QString name;

        static QString coordsJsonKey() { return "coords"; }
        double vpX = {};
        double vpY = {};

        static QString vpZoomJsonKey() { return "vp-zoom"; }
        double vpZoom = {};

        static QString mapZoomJsonKey() { return "map-zoom"; }
        int mapZoom = {};

        static QString drawFillJsonKey() { return "draw-fill"; }
        bool drawFill = {};

        static QString drawLinesJsonKey() { return "draw-lines"; }
        bool drawLines = {};

        static QString tileListJsonKey() { return "tiles"; }
        QVector<TileCoord> coords = {};
        bool autoCalcVisibleTiles = false;

        int imageWidth = baseImageSize;
        int imageHeight = baseImageSize;

        double imageAspect() const { return (double)imageWidth / (double)imageHeight; }
    };

    struct SimpleError {
        QString msg;
    };

    template<typename T>
    struct SimpleResult {
        bool success = {};
        T value;
        QString errorMsg;

        SimpleResult(const SimpleError& err) :
            success{ false },
            errorMsg{ err.msg } {}
        SimpleResult(const T& val) :
            success{ true },
            value{ val } {}
        SimpleResult(T&& val) :
            success{ true },
            value{ std::move(val) } {}
    };

    template<>
    struct SimpleResult<void> {
        bool success;
        QString errorMsg;

        SimpleResult() :
            success { true } {}
        SimpleResult(const SimpleError& err) :
            success{ false },
            errorMsg{ err.msg } {}
    };

    std::optional<QFont> loadFont();
    SimpleResult<StyleSheet> loadStylesheet();
    SimpleResult<QVector<TestItem>> loadTestItems();
    SimpleResult<QImage> render(
        const TestItem &item,
        const StyleSheet &stylesheet,
        const QFont &font);

    using ProcessTestCaseFnT = std::function<void(
        int testId,
        const TestItem &testItem,
        const QImage &image)>;
    SimpleResult<void> iterateOverTestCases(
        const QFont &font,
        const ProcessTestCaseFnT &processFn);

    QString buildBaselinePath();
    QString buildBaselineExpectedOutputPath();
    QString buildBaselineExpectedOutputPath(int testId);
    QString getStyleSheetPath();
}

#endif // RENDERING_OUTPUT_TESTER_H
