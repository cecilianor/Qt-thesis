#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <QPainter>
#include <QMap>

#include "TileCoord.h"
#include "VectorTiles.h"

namespace Bach {
    constexpr int maxZoomLevel = 16;
    const int defaultDesiredTileWidth = 256;

    int CalcMapZoomLevelForTileSizePixels(
        int vpWidth,
        int vpHeight,
        double vpZoom,
        int desiredTileWidth = defaultDesiredTileWidth);

    QVector<TileCoord> CalcVisibleTiles(
        double vpX,
        double vpY,
        double vpAspect,
        double vpZoomLevel,
        int mapZoomLevel);

    inline void paintSingleTile(
        VectorTile const& tileData,
        QPainter& painter,
        QMap<QString, QColor> const& layerColors,
        QTransform const& transformIn)
    {
        for (auto const& [layerName, layer]: tileData.m_layers.asKeyValueRange()) {
            auto layerColorIt = layerColors.find(layerName);
            if (layerColorIt != layerColors.end()) {
                painter.setBrush(*layerColorIt);
            } else {
                painter.setBrush(Qt::NoBrush);
            }

            for (auto const& featureIn : layer->m_features) {
                switch (featureIn->type()) {
                case AbstractLayerFeature::featureType::polygon: {
                    auto* feature = dynamic_cast<PolygonFeature*>(featureIn);
                    auto const& path = feature->polygon();

                    QTransform transform = transformIn;
                    transform.scale(1 / 4096.0, 1 / 4096.0);
                    auto newPath = transform.map(path);

                    painter.save();
                    //painter.setBrush(Qt::NoBrush);
                    painter.setPen(Qt::NoPen);
                    painter.drawPath(newPath);
                    painter.restore();
                }
                break;
                case AbstractLayerFeature::featureType::line: {
                    auto* feature = dynamic_cast<LineFeature*>(featureIn);
                    auto const& path = feature->line();
                    QTransform transform = transformIn;
                    transform.scale(1 / 4096.0, 1 / 4096.0);
                    auto newPath = transform.map(path);

                    painter.save();
                    painter.setPen(Qt::red);
                    painter.setBrush(Qt::NoBrush);
                    painter.drawPath(newPath);
                    painter.restore();
                }
                break;
                default:
                    break;
                }
            }
        }
    }

    void paintSingleTileDebug(
        QPainter& painter,
        TileCoord const& tileCoord,
        QPoint pixelPos,
        QTransform const& transform);

    inline void paintTiles(
        QPainter& painter,
        double vpX,
        double vpY,
        double viewportZoomLevel,
        int mapZoomLevel,
        QMap<TileCoord, VectorTile const*> const& tileContainer,
        QMap<QString, QColor> const& layerColors)
    {
        auto viewportWidth = painter.window().width();
        auto viewportHeight = painter.window().height();
        double vpAspectRatio = (double)viewportWidth / (double)viewportHeight;
        auto visibleTiles = CalcVisibleTiles(
            vpX,
            vpY,
            vpAspectRatio,
            viewportZoomLevel,
            mapZoomLevel);

        auto largestDimension = qMax(viewportWidth, viewportHeight);

        double scale = pow(2, viewportZoomLevel - mapZoomLevel);
        double tileWidthNorm = scale;
        double tileHeightNorm = scale;

        auto font = painter.font();
        font.setPointSize(18);
        painter.setFont(font);

        // Calculate total number of tiles at the current zoom level
        int totalTilesAtZoom = 1 << mapZoomLevel;

        // Calculate the offset of the viewport center in pixel coordinates
        double centerNormX = vpX * totalTilesAtZoom * tileWidthNorm - 1.0 / 2;
        double centerNormY = vpY * totalTilesAtZoom * tileHeightNorm - 1.0 / 2;

        if (viewportHeight >= viewportWidth) {
            centerNormX += -0.5 * vpAspectRatio + 0.5;
        } else if (viewportWidth >= viewportHeight) {
            centerNormY += -0.5 * (1 / vpAspectRatio) + 0.5;
        }

        for (const auto& tileCoord : visibleTiles) {
            double posNormX = (tileCoord.x * tileWidthNorm) - centerNormX;
            double posNormY = (tileCoord.y * tileHeightNorm) - centerNormY;

            auto tilePixelPos = QPoint(
                round(posNormX * largestDimension),
                round(posNormY * largestDimension));
            int tileWidthPixels = round(tileWidthNorm * largestDimension);
            int tileHeightPixels = round(tileHeightNorm * largestDimension);

            painter.save();

            QTransform transform;
            transform.translate(tilePixelPos.x(), tilePixelPos.y());
            //transform.scale(largestDimension, largestDimension);
            painter.setTransform(transform);

            auto pen = painter.pen();
            pen.setColor(Qt::white);
            pen.setWidth(1);
            painter.setPen(pen);

            QTransform test;
            test.scale(largestDimension * scale, largestDimension * scale);

            auto tileIt = tileContainer.find(tileCoord);
            if (tileIt != tileContainer.end()) {
                auto const& tileData = **tileIt;

                painter.save();
                painter.setClipRect(
                    0,
                    0,
                    tileWidthPixels,
                    tileHeightPixels);

                paintSingleTile(tileData, painter, layerColors, test);

                painter.restore();
            }

            paintSingleTileDebug(painter, tileCoord, tilePixelPos, test);

            painter.restore();
        }
    }
}

#endif // RENDERING_HPP
