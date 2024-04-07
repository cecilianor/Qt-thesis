#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <QChar>
#include <QJsonArray>
#include <QMap>
#include <QString>
#include <QVariant>

#include "VectorTiles.h"

class Evaluator
{
public:
    Evaluator(){};
    static QVariant resolveExpression(const QJsonArray& expression, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
private:
    static void setupExpressionMap();
    static QMap<QString, QVariant(*)(const QJsonArray&, const AbstractLayerFeature*, int mapZoomLevel, float vpZoomeLevel)> m_expressionMap;
    static QVariant get(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant has(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant in(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant compare(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant notEqual(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant equal(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant greater(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant all(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant case_(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant coalesce(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant match(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
    static QVariant interpolate(const QJsonArray& array, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel);
};

#endif // EVALUATOR_H


