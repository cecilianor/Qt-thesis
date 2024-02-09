#ifndef RENDERING_HPP
#define RENDERING_HPP

#include <QPainter>
#include <QMap>

#include <optional>

#include "TileCoord.h"
#include "VectorTiles.h"

namespace Bach {
    QVector<TileCoord> CalcVisibleTiles(
        double vpX,
        double vpY,
        double vpZoomLevel,
        int mapZoomLevel);

    struct PaintSingleTile_Params {

    };

    inline void paintSingleTile(
        VectorTile const& tileData,
        QPainter& painter,
        QTransform const& transformIn)
    {
        for (auto const& layer : tileData.m_layers) {
            for (auto const& featureIn : layer->m_features)
            {
                if (featureIn->type() != AbstractLayerFeature::featureType::polygon) {
                    continue;
                }

                auto* feature = dynamic_cast<PolygonFeature*>(featureIn);
                auto const& path = feature->polygon();

                QTransform transform = transformIn;
                transform.scale(1 / 4096.0, 1 / 4096.0);
                auto newPath = transform.map(path);

                painter.drawPath(newPath);
            }
        }
    }

    inline void paintSingleTileDebug(
        QPainter& painter,
        TileCoord const& tileCoord,
        QPoint pixelPos,
        QTransform const& transform)
    {
        painter.drawLine(transform.map(QLineF{ QPointF(0.45, 0.45), QPointF(0.55, 0.55) }));
        painter.drawLine(transform.map(QLineF{ QPointF(0.55, 0.45), QPointF(0.45, 0.55) }));
        painter.drawRect(transform.mapRect(QRectF(0, 0, 1, 1)));

        {
            painter.save();
            QTransform transform;
            transform.translate(pixelPos.x(), pixelPos.y());
            painter.setTransform(transform);

            painter.drawText(10, 30, tileCoord.toString());
            painter.restore();
        }
    }

    inline void paintTiles(
        QPainter& painter,
        double vpX,
        double vpY,
        double viewportZoomLevel,
        int mapZoomLevel,
        QMap<TileCoord, VectorTile const*> const& tileContainer)
    {
        auto width = painter.window().width();
        auto height = painter.window().height();
        auto visibleTiles = CalcVisibleTiles(
            vpX,
            vpY,
            viewportZoomLevel,
            mapZoomLevel);

        auto largestDimension = qMax(width, height);
        double vpAspectRatio = (double)width / (double)height;

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

        if (height >= width) {
            centerNormX += -0.5 * vpAspectRatio + 0.5;
        } else if (width >= height) {
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
            pen.setColor(Qt::green);
            pen.setWidth(2);
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

                paintSingleTile(tileData, painter, test);

                painter.restore();
            }

            paintSingleTileDebug(painter, tileCoord, tilePixelPos, test);

            painter.restore();
        }
    }
}

#endif // RENDERING_HPP
