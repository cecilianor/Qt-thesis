#include "Evaluator.h"

QMap<QString, QVariant(*)(const QJsonArray&, const AbstractLayerFeature*, int mapZoomLevel, float vpZoomeLevel)> Evaluator::m_expressionMap;

QVariant Evaluator::resolveExpression(const QJsonArray &expression, const AbstractLayerFeature* feature, int mapZoomLevel, float vpZoomeLevel)
{
    if (m_expressionMap.isEmpty()) setupExpressionMap();
    if (expression.empty()) {
        return {};
    }
    QString operation = expression.begin()->toString();
    if(operation == "!=") {
        return m_expressionMap["!="](expression, feature, mapZoomLevel, vpZoomeLevel);
    }else{
        if(operation.startsWith("!")){
            if(m_expressionMap.contains(operation.sliced(1))){
                return m_expressionMap[operation.sliced(1)](expression, feature, mapZoomLevel, vpZoomeLevel);
            }
        } else {
            if(m_expressionMap.contains(operation)){
                return m_expressionMap[operation](expression, feature, mapZoomLevel, vpZoomeLevel);
            }
        }
    }
    return {};
}

void Evaluator::setupExpressionMap()
{
    m_expressionMap.insert("get", get);
    m_expressionMap.insert("has", has);
    m_expressionMap.insert("in", in);
    m_expressionMap.insert("!=", compare);
    m_expressionMap.insert("==", compare);
    m_expressionMap.insert("<", greater);
    m_expressionMap.insert("all", all);
    m_expressionMap.insert("case", case_);
    m_expressionMap.insert("coalesce", coalesce);
    m_expressionMap.insert("match", match);
    m_expressionMap.insert("interpolate", interpolate);
}

QVariant Evaluator::get(const QJsonArray & array, const AbstractLayerFeature * feature, int mapZoomLevel, float vpZoomeLevel)
{
    QString property = array.at(1).toString();
    if(feature->fetureMetaData.contains(property)){
        return feature->fetureMetaData[property];
    }else{
        return {};
    }
}

QVariant Evaluator::has(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QString property = array.at(1).toString();
    return feature->fetureMetaData.contains(property);
}

QVariant Evaluator::in(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QString keyword = array.at(1).toString();
    if(feature->fetureMetaData.contains(keyword)){
        QVariant value = feature->fetureMetaData[keyword];

        auto temp = array.toVariantList().sliced(2).contains(value);
        auto startsWithNot = array.first().toString().startsWith("!");
        if (startsWithNot) {
            temp = !temp;
        }

        bool result = temp;
        return result;
    }else{
        return false;
    }
}

QVariant Evaluator::compare(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{

    QVariant operand1;
    QVariant operand2;
    if(array.at(1).isArray()){
        static QJsonArray operand1Arr = array.at(1).toArray();
        QString temp = resolveExpression(operand1Arr, feature, mapZoomLevel, vpZoomeLevel).toString();
        if(temp == "$type"){
            switch(feature->type()){
            case AbstractLayerFeature::featureType::polygon:
                operand1 = QVariant(QString("Polygon"));
                break;
            case AbstractLayerFeature::featureType::line:
                operand1 = QVariant(QString("LineString"));
                break;
            case AbstractLayerFeature::featureType::point:
                operand1 = QVariant(QString("Point"));
                break;
            default:
                break;
            }
        }else{
            operand1 = feature->fetureMetaData.contains(temp) ? feature->fetureMetaData[temp] : QVariant();
        }
    }else{
        QString temp = array.at(1).toString();
        if(temp == "$type"){
            switch(feature->type()){
            case AbstractLayerFeature::featureType::polygon:
                operand1 = QVariant(QString("Polygon"));
                break;
            case AbstractLayerFeature::featureType::line:
                operand1 = QVariant(QString("LineString"));
                break;
            case AbstractLayerFeature::featureType::point:
                operand1 = QVariant(QString("Point"));
                break;
            default:
                break;
            }
        }else{
            operand1 = feature->fetureMetaData.contains(temp) ? feature->fetureMetaData[temp] : QVariant();
        }
    }


    operand2 = array.at(2).toVariant();
    if(array.at(0).toString() == "!="){
        return operand1 != operand2;
    }else{
        return operand1 == operand2;
    }

}

QVariant Evaluator::greater(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QVariant operand1;
    QVariant operand2;
    if(array.at(1).isArray()){
        static QJsonArray operand1Arr = array.at(1).toArray();
        operand1 = resolveExpression(operand1Arr, feature, mapZoomLevel, vpZoomeLevel);
    }else{
        operand1 = array.at(1).toVariant();
    }

    if(array.at(2).isArray()){
        static QJsonArray operand2Arr = array.at(2).toArray();
        operand1 = resolveExpression(operand2Arr, feature, mapZoomLevel, vpZoomeLevel);
    }else{
        operand1 = array.at(2).toVariant();
    }
    if(operand1.typeId() == QMetaType::QString){
        return operand1.toString() > operand2.toString();
    }else{
        return operand1.toDouble() > operand2.toDouble();
    }
}

QVariant Evaluator::all(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    for(int i = 1; i < array.size() - 1; i += 2){
        QJsonArray expressionArray = array.at(i).toArray();
        if(!resolveExpression(expressionArray, feature, mapZoomLevel, vpZoomeLevel).toBool()){
            return false;
        }
    }
    return true;
}

QVariant Evaluator::case_(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    for(int i = 1; i < array.size() - 2; i += 2){
        if(array.at(i).isArray()){
            QJsonArray expression = array.at(i).toArray();
            if(resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel).toBool()){
                return array.at(i + 1);
            }
        }
    }
    return array.last();
}

QVariant Evaluator::coalesce(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    for(int i = 1; i < array.size() - 1; i++){
        QJsonArray expression = array.at(i).toArray();
        auto returnVariant = resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel);
        if(returnVariant.isValid()) return returnVariant;
    }
    return {};
}

QVariant Evaluator::match(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QJsonArray expression = array.at(1).toArray();
    QVariant input = resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel);
    for(int i = 2; i < array.size() - 2; i += 2){
        if(input == array.at(i)){
            return array.at(i + 1).toVariant();
        }
    }
    return array.last().toVariant();

}

static float lerp(QPair<float, float> stop1, QPair<float, float> stop2, int currentZoom)
{
    float lerpedValue = stop1.second + (currentZoom - stop1.first)*(stop2.second - stop1.second)/(stop2.first - stop1.first);
    return lerpedValue;
}

QVariant Evaluator::interpolate(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QVariant returnVariant;
    if(mapZoomLevel <= array.at(3).toDouble()){
        if(array.at(4).isArray()){
            returnVariant = resolveExpression(array.at(4).toArray(), feature, mapZoomLevel, vpZoomeLevel);
        }else{
            returnVariant = array.at(4).toDouble();
        }
    }else if(mapZoomLevel >= array.at(array.size()-2).toDouble()){
        if(array.last().isArray()){
            returnVariant = resolveExpression(array.last().toArray(), feature, mapZoomLevel, vpZoomeLevel);
        }else{
            returnVariant = array.last().toDouble();
        }
    }else{
        int index = 3;
        while(mapZoomLevel > array.at(index).toDouble() && index < array.size()){
            index += 2;
        }
        float stopInput1 = array.at(index-2).toDouble();
        float stopInput2 = array.at(index).toDouble();
        float stopOutput1;
        float stopOutput2;

        if(array.at(index - 1).isArray()){
            stopOutput1 = resolveExpression(array.at(index - 1).toArray(), feature, mapZoomLevel, vpZoomeLevel).toFloat();
        }else{
            stopOutput1 = array.at(index - 1).toDouble();
        }

        if(array.at(index + 1).isArray()){
            stopOutput2 = resolveExpression(array.at(index + 1).toArray(), feature, mapZoomLevel, vpZoomeLevel).toFloat();
        }else{
            stopOutput2 = array.at(index + 1).toDouble();
        }


        returnVariant = lerp(QPair<float, float>(stopInput1,stopOutput1), QPair<float, float>(stopInput2,stopOutput2), mapZoomLevel);

    }
    return returnVariant;

}




/*bool Evaluator::isExpression(QJsonArray &array) const
{
    if(!array.at(0).isString()) return false;
    if(array.begin()->toString().startsWith("!")){
        return m_expressionMap.keys().contains(array.begin()->toString().sliced(1));
    }else{
        return m_expressionMap.keys().contains(array.begin()->toString());
    }
}*/









