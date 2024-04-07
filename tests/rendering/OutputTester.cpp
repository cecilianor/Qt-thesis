#include "OutputTester.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QJsonDocument>

#include "Layerstyle.h"
#include "Rendering.h"
#include "VectorTiles.h"

QVector<TileCoord> Bach::OutputTester::genTileCoordList(int zoom, int minX, int maxX, int minY, int maxY)
{
    QVector<TileCoord> out;
    for (int x = minX; x < maxX; x++) {
        for (int y = minY; y < maxY; y++) {
            out.push_back({zoom, x, y});
        }
    }
    return out;
}

QString Bach::OutputTester::buildBaselinePath()
{
    return BACH_RENDEROUTPUT_BASELINE_DIR;
}

QString Bach::OutputTester::buildBaselineExpectedOutputPath()
{
    return buildBaselinePath() + QDir::separator() + "expected_output";
}

QString Bach::OutputTester::buildBaselineExpectedOutputPath(int testId)
{
    return buildBaselineExpectedOutputPath() + QDir::separator() + QString::number(testId) + QString(".png");
}

QString Bach::OutputTester::getStyleSheetPath() {
    return buildBaselinePath() + QDir::separator() + "/styleSheet.json";
}

std::optional<QFont> Bach::OutputTester::loadFont()
{
    QFontDatabase::removeAllApplicationFonts();
    int fontId = QFontDatabase::addApplicationFont(
        Bach::OutputTester::buildBaselinePath() +
        QDir::separator() +
        "RobotoMono-Regular.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont font = fontFamilies.first();
        QGuiApplication::setFont(font);
        return font;
    } else {
        return std::nullopt;
    }
}

using TileMapT = std::map<TileCoord, std::unique_ptr<VectorTile>>;
static std::optional<TileMapT> loadTiles(QVector<TileCoord> tileCoords) {
    TileMapT tileStorage;
    for (TileCoord tileCoord : tileCoords) {
        // Load each tile from file.
        QString path = Bach::OutputTester::buildBaselinePath() + QDir::separator() + QString("z%1x%2y%3.mvt")
            .arg(tileCoord.zoom)
            .arg(tileCoord.x)
            .arg(tileCoord.y);

        QFile vectorFile { path };
        bool fileOpenSuccess = vectorFile.open(QFile::ReadOnly);
        if (!fileOpenSuccess) {
            //shutdown(QString("Unable to open file: %1").arg(path));
            return std::nullopt;
        }

        std::optional<VectorTile> tileResult = VectorTile::fromByteArray(vectorFile.readAll());
        if (!tileResult.has_value()) {
            //shutdown("Failed to parse file into VectorTile object.");
            return std::nullopt;
        }
        auto allocatedTile = std::make_unique<VectorTile>(std::move(tileResult.value()));
        tileStorage.insert({tileCoord, std::move(allocatedTile)});
    }
    return tileStorage;
}

bool Bach::OutputTester::iterateOverTestCases(
    const QFont &font,
    const std::function<void(int, const QImage&)> &processFn)
{
    QFile styleSheetFile { Bach::OutputTester::getStyleSheetPath() };
    styleSheetFile.open(QFile::ReadOnly);

    QJsonParseError parseError;
    QJsonDocument styleSheetJson = QJsonDocument::fromJson(styleSheetFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        //shutdown("Failed to parse vector stylesheet file into JSON");
        return false;
    }

    std::optional<StyleSheet> styleSheetResult = StyleSheet::fromJson(styleSheetJson);
    if (!styleSheetResult.has_value()) {
        //shutdown("Failed to parse JSON into stylesheet object.");
        return false;
    }
    const StyleSheet &styleSheet = styleSheetResult.value();

    using TestItem = Bach::OutputTester::TestItem;
    const QVector<TestItem> &testItems = Bach::OutputTester::testItems;
    for (int i = 0; i < testItems.size(); i++) {
        const TestItem &testItem = testItems[i];

        QVector<TileCoord> tileCoords = testItem.coords;
        if (testItem.autoCalcVisibleTiles) {
            tileCoords = Bach::calcVisibleTiles(
                testItem.vpX,
                testItem.vpY,
                testItem.imageAspect(),
                testItem.vpZoom,
                testItem.mapZoom);
        }

        std::optional<TileMapT> tileMapOpt = loadTiles(tileCoords);
        if (!tileMapOpt.has_value()) {
            return false;
        }
        const TileMapT &tileMap = tileMapOpt.value();
        QMap<TileCoord, const VectorTile*> tempTiles;
        for (auto& [coord, tile] : tileMap) {
            tempTiles.insert(coord, tile.get());
        }

        QImage generatedImg { testItem.imageWidth, testItem.imageHeight, QImage::Format_ARGB32};
        QPainter painter{ &generatedImg };
        painter.setFont(font);

        Bach::PaintVectorTileSettings paintSettings = {};
        paintSettings.drawFill = true;
        paintSettings.drawLines = true;
        paintSettings.drawText = testItem.drawText;
        paintSettings.forceNoChangeFontType = true;
        Bach::paintVectorTiles(
            painter,
            testItem.vpX,
            testItem.vpY,
            testItem.vpZoom,
            testItem.mapZoom,
            tempTiles,
            styleSheet,
            paintSettings,
            false);

        painter.end();

        processFn(i, generatedImg);
    }

    return true;
}
