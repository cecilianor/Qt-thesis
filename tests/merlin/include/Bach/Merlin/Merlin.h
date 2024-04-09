#ifndef BACH_MERLIN_H
#define BACH_MERLIN_H

#include <QFont>
#include <QJsonObject>
#include <QImage>
#include <QVector>

#include "TileCoord.h"
#include "Layerstyle.h"

#ifndef BACH_RENDEROUTPUT_BASELINE_DIR
#error "C++ define 'BACH_RENDEROUTPUT_BASELINE_DIR' was not defined. This likely means a build error."
#endif

namespace Bach::Merlin {
    /*!
     * \brief The SimpleError class,
     * a helper struct to generate an error SimpleResult without having
     * to supply a type template parameter.
     */
    struct SimpleError {
        QString msg;
    };

    /*!
     * \brief The SimpleResult class is a helper container
     * for returning error messages or a value from a function.
     *
     * If the struct is set to success, then the class holds a valid value.
     *
     * If the struct is NOT set to success, it may or may not hold an
     * error message, but it will definitely not hold a value.
     */
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
        SimpleError error() const { return SimpleError { errorMsg }; }
    };

    /*!
     *  \brief
     *  A simple type overload for when we want to return a SimpleResult
     *  with no type associated.
     */
    template<>
    struct SimpleResult<void> {
        bool success;
        QString errorMsg;

        SimpleResult() :
            success { true } {}
        SimpleResult(const SimpleError& err) :
            success{ false },
            errorMsg{ err.msg } {}
        SimpleError error() const { return SimpleError { errorMsg }; }
    };


    // The base size of an image outputted by the tester.
    const int baseImageSize = 1024;

    /*!
     * \brief The TestItem class
     * is a class that describes a single test item when
     * running render-tests. It contains configurations
     * on how to run a single vector-render.
     */
    struct TestItem {
        static QString nameJsonKey() { return "name"; }
        /*!
         * \brief name
         * Contains the name of this test case. Will be displayed
         * where relevant to help identify a particular test. May be empty.
         */
        QString name;

        static QString coordsJsonKey() { return "coords"; }
        SimpleResult<void> loadCoordsFromJson(const QJsonObject &testItemJson);

        /*!
         * \brief vpX
         * Center X coordinate of the viewport.
         */
        double vpX = {};
        /*!
         * \brief vpY
         * Center Y coordinate of the viewport.
         */
        double vpY = {};

        static QString vpZoomJsonKey() { return "vp-zoom"; }
        /*!
         * \brief vpZoom
         * Zoom level of the viewport.
         */
        double vpZoom = {};

        static QString mapZoomJsonKey() { return "map-zoom"; }
        /*!
         * \brief mapZoom
         * Zoom level of the map.
         */
        int mapZoom = {};

        static QString drawFillJsonKey() { return "draw-fill"; }
        /*!
         * \brief drawFill
         * Controls whether fill-elements should be rendered.
         */
        bool drawFill = {};

        static QString drawLinesJsonKey() { return "draw-lines"; }
        /*!
         * \brief drawLines
         * Controls whether line-elements should be rendered.
         */
        bool drawLines = {};

        static QString tileListJsonKey() { return "tiles"; }
        /*!
         * \brief tileCoords
         * Contains the list of tiles that should be used during rendering.
         * May be a subset of visible tiles.
         */
        QVector<TileCoord> tileCoords;

        /*!
         * \brief imageWidth
         * Width of the output image, in pixels.
         */
        int imageWidth = baseImageSize;
        /*!
         * \brief imageHeight
         * Height of the output image, in pixels.
         */
        int imageHeight = baseImageSize;

        /*!
         * \brief imageAspect
         * \return The aspect ratio of the image being outputted by this test case.
         */
        double imageAspect() const { return (double)imageWidth / (double)imageHeight; }

        static QVector<QString> knownJsonKeys() {
            return {
                nameJsonKey(),
                coordsJsonKey(),
                vpZoomJsonKey(),
                mapZoomJsonKey(),
                drawFillJsonKey(),
                drawLinesJsonKey(),
                tileListJsonKey()
            };
        }
    };


    std::optional<QFont> loadFont();
    SimpleResult<StyleSheet> loadStylesheet();
    SimpleResult<QVector<TestItem>> loadTestItems();

    SimpleResult<QImage> render(
        const TestItem &item,
        const StyleSheet &stylesheet,
        const QFont &font);

    QString buildBaselinePath();
    QString buildBaselineInputPath();
    QString buildBaselineExpectedOutputPath();
    QString buildBaselineExpectedOutputPath(int testId);
    QString getStyleSheetPath();
}

#endif // BACH_MERLIN_H
