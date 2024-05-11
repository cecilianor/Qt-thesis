// Non-QT and non-STL header files.
#include "Evaluator.h"

/*
 * All thesis functions follow the MapTiler specification, which can be found here:
 * https://docs.maptiler.com/gl-style-specification/expressions/
 */

// This maps expression keywords to their corresponding functions.
QMap<QString, QVariant(*)(const QJsonArray&, const AbstractLayerFeature*, int mapZoomLevel, float vpZoomLevel)> Evaluator::m_expressionMap;

/*!
 * \brief Evaluator::resolveExpression
 * This function forwards the expression array to the appropriate function from the function map
 * depending on the expression kyeword wich is always the first element of the array.
 *
 * \param expression The QJson array containing the expression to be resolved.
 * \param feature The feature on which expression operation will be performed (if aplicable)
 * \param mapZoomLevel The zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomLevel The viewport zoom level.
 * Currently not used for any expression (Can be used for smoother interpolation).
 *
 * \return a QVariant containing the result of the evaluation, or an invalid QVariant if the expression was invalid.
 */
QVariant Evaluator::resolveExpression(
    const QJsonArray &expression,
    const AbstractLayerFeature* feature,
    int mapZoomLevel,
    float vpZoomLevel)
{
    // This will check only the first time the function is called after the program starts.
    if (m_expressionMap.isEmpty())
        setupExpressionMap();

    // Check for valid expression.
    if (expression.empty())
        return {};

    // Extract the operation keyword from the expression.
    QString operation = expression.begin()->toString();

    if (operation == "!=") {
        // This check is made since all operations can have an OPTIONAL "!" sign for negation except "!=" operation.
        return m_expressionMap["!="](expression, feature, mapZoomLevel, vpZoomLevel);
    } else {
        if (operation.startsWith("!")) {
            // Check if the operation contains a negation sign.
            if (m_expressionMap.contains(operation.sliced(1)))
                // In case the expression is negated, remove the "!" sign to get the operation keyword.
                return m_expressionMap[operation.sliced(1)](expression, feature, mapZoomLevel, vpZoomLevel);
        } else {
            if (m_expressionMap.contains(operation))
                return m_expressionMap[operation](expression, feature, mapZoomLevel, vpZoomLevel);
        }
    }
    // Return an invalid QVariant in case the expression was invalid or not supported.
    return {};
}

/*!
 * \brief Evaluator::setupExpressionMap
 * Maps each expression keyword to the function that resolves it.
 */
void Evaluator::setupExpressionMap()
{
    m_expressionMap.insert("get", get);
    m_expressionMap.insert("has", has);
    m_expressionMap.insert("in", in);
    m_expressionMap.insert("!=", compare);
    m_expressionMap.insert("==", compare);
    m_expressionMap.insert(">", greater);
    m_expressionMap.insert("all", all);
    m_expressionMap.insert("case", case_);
    m_expressionMap.insert("coalesce", coalesce);
    m_expressionMap.insert("match", match);
    m_expressionMap.insert("interpolate", interpolate);
}

/*!
 * \brief Evaluator::get
 * Resolves a "get" expression, which gets a property from the metadata of the feature.
 *
 * \param array The QJson array containing the expression to be resolved.
 * \param feature The feature on which expression operation will be performed.
 * \param mapZoomLevel The zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomLevel The viewport zoom level.
 *
 * \return a QVariant conatining the value of the feature's property or an invalid(NULL)
 * QVariant if the feature does not contain the specified property.
 */
QVariant Evaluator::get(const QJsonArray & array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomLevel)
{
    QString property = array.at(1).toString();
    if(feature->featureMetaData.contains(property)){
        return feature->featureMetaData[property];
    }else{
        return {};
    }
}


/*!
 * \brief Evaluator::has
 * Resolve the "has" expression which checks if a property exists in the metadata of the feature.
 * \param expression the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing True if the feature's metadata include the property, or False otherwise.
 */
QVariant Evaluator::has(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QString property = array.at(1).toString();
    return feature->featureMetaData.contains(property);
}

/*!
 * \brief Evaluator::in
 * Resolve the "in" expression which checks if a feature's property is in a range of values.
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return  a QVariant containing true if the property is in the range of values or false otherwise
 */
QVariant Evaluator::in(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QString keyword = array.at(1).toString();
    if(feature->featureMetaData.contains(keyword)){
        QVariant value = feature->featureMetaData[keyword];

        //the range of values to be checked is in the array from elemet 2 to n.
        auto temp = array.toVariantList().sliced(2).contains(value);
        //Check for negation.
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

/*!
 * \brief getType
 * A helper function to check the type of the feature and return it as a string.
 * \param feature the feature Whose type is being checked.
 * \return a string containing the type of the feature.
 */
static QString getType(const AbstractLayerFeature *feature)
{
    switch(feature->type()){
        case AbstractLayerFeature::featureType::polygon:
            return "Polygon";
            break;
        case AbstractLayerFeature::featureType::line:
            return "LineString";
            break;
        case AbstractLayerFeature::featureType::point:
            return "Point";
            break;
        default:
            return "Unknown";
            break;
    }
}
/*!
 * \brief Evaluator::compare
 * Resolve the "==" and "!=" expression which checks if two values are equal or not
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return return a QVariant containing true if the two compared elements are true or false otherwise.
 */
QVariant Evaluator::compare(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QVariant operand1;
    QVariant operand2;
    if(array.at(1).isArray()){ //We check if the operation opperand is a simple value or an expression in itself that will need resolving to extract the value.
        static QJsonArray operand1Arr = array.at(1).toArray();
        QString temp = resolveExpression(operand1Arr, feature, mapZoomLevel, vpZoomeLevel).toString();
        if(temp == "$type"){ //"type" is not a part of the feature's metadata so it is a special case
            operand1 = getType(feature);
        }else{
            operand1 = feature->featureMetaData.contains(temp) ? feature->featureMetaData[temp] : QVariant();
        }
    }else{
        QString temp = array.at(1).toString();
        if(temp == "$type"){ //type is not a part of the feature's metadata so it is a special case
            operand1 = getType(feature);
        }else{
            operand1 = feature->featureMetaData.contains(temp) ? feature->featureMetaData[temp] : QVariant();
        }
    }

    operand2 = array.at(2).toVariant();
    //Check wich operation this expression contains and return the result of the comparison
    if(array.at(0).toString() == "!="){
        return operand1 != operand2;
    }else{
        return operand1 == operand2;
    }
}

/*!
 * \brief Evaluator::greater
 * Resolve the ">" expression which checks if a value is greater than the other.
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing the true if the first value is greated than the other one or false otherwise
 */
QVariant Evaluator::greater(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QVariant operand1;
    QVariant operand2;
    if(array.at(1).isArray()){//If the operand is an expression, we need to resolve it to get the operand value.
        static QJsonArray operand1Arr = array.at(1).toArray();
        operand1 = resolveExpression(operand1Arr, feature, mapZoomLevel, vpZoomeLevel);
    }else{
        operand1 = array.at(1).toVariant();
    }

    if(array.at(2).isArray()){//If the operand is an expression, we need to resolve it to get the operand value.
        static QJsonArray operand2Arr = array.at(2).toArray();
        operand2 = resolveExpression(operand2Arr, feature, mapZoomLevel, vpZoomeLevel);
    }else{
        operand2 = array.at(2).toVariant();
    }

    //The operand types that we can compare are numeric values or strings.
    if(operand1.typeId() == QMetaType::QString){
        return operand1.toString() > operand2.toString();
    }else{
        return operand1.toDouble() > operand2.toDouble();
    }
}

/*!
 * \brief Evaluator::all
 * Resolve the "all" expression which checks if all the inner expressions in the array evaluate to true
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing true if all the inner expressions are true or returns false otehrwise
 */
QVariant Evaluator::all(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    //loop over all the expressions and check that they evaluate to true.
    for(int i = 1; i <= array.size() - 1; i++){
        QJsonArray expressionArray = array.at(i).toArray();
        if(!resolveExpression(expressionArray, feature, mapZoomLevel, vpZoomeLevel).toBool()){
            return false;
        }
    }
    return true;
}

/*!
 * \brief Evaluator::case_
 * Resolve the "case" expression which return the first output whose corresponding input evaluates to true
 * or the fallback value if all the inputs are false
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing the output for the input that evalueated to true, or the fallback value
 */
QVariant Evaluator::case_(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    //loop over the array elements from 1 to n - 1 (element 0 contains the operation keyword and element n contains the fallback value)
    for(int i = 1; i < array.size() - 2; i += 2){
        if(array.at(i).isArray()){
            QJsonArray expression = array.at(i).toArray();
            //if the current expression being resolved evaluated to true return its corresponding output (the values right after it).
            if(resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel).toBool()){
                return array.at(i + 1).toVariant();
            }
        }
    }
    //if the loop ends without returning, we return the fallback value at index n.
    return array.last().toVariant();
}

/*!
 * \brief Evaluator::coalesce
 * Resolve the "coalesce" expression which return the first non null output.
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing the value of the first non-null expression, or an invalid QVariant if non exist.
 */
QVariant Evaluator::coalesce(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    //loop over the expression array returning the first valid QVariant.
    for(int i = 1; i <= array.size() - 1; i++){
        QJsonArray expression = array.at(i).toArray();
        auto returnVariant = resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel);
        if(returnVariant.isValid()) return returnVariant;
    }
    return {};
}

/*!
 * \brief Evaluator::match
 * Resolve the "match" expression which mimics a switch case statement

 * \return a QVariant containing the value of the output whos label matches the input, or the fallback value if not labels match.
 */
QVariant Evaluator::match(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    //Extract the label to be used for the checks.
    QJsonArray expression = array.at(1).toArray();
    QVariant input = resolveExpression(expression, feature, mapZoomLevel, vpZoomeLevel);

    //loop over the array checking which value matches the input and we return its corresponding output.
    //we loop over the elemtns from 2 to n-2 because the first two elements contain the expression keyword and
    //the operation label, and the last element contains the fallback value.
    for(int i = 2; i < array.size() - 2; i += 2){
        if(array.at(i).isArray()){
            if(array.at(i).toArray().toVariantList().contains(input)){
                if(array.at(i + 1).isArray()){
                    return resolveExpression(array.at(i + 1).toArray(), feature, mapZoomLevel, vpZoomeLevel);
                }else{
                    return array.at(i + 1).toVariant();
                }
            }

        }else if(input == array.at(i).toVariant()){
            if(array.at(i + 1).isArray()){
                return resolveExpression(array.at(i + 1).toArray(), feature, mapZoomLevel, vpZoomeLevel);
            }else{
                return array.at(i + 1).toVariant();
            }
        }
    }
    //If the loop ends without returning, we return the fallback value at index n.
    return array.last().toVariant();
}

/*!
 * \brief lerp
 * A helper function to Perform a linear interpolation.
 * \param stop1 a QPair containing the x and y for the first stop point
 * \param stop2 a QPair containing the x and y for the second stop point
 * \param currentZoom an int containing the value to be used in the interpolation
 * \return a float containing the result of the interpolation.
 */
static float lerp(QPair<float, float> stop1, QPair<float, float> stop2, int currentZoom)
{
    float lerpedValue = stop1.second + (currentZoom - stop1.first)*(stop2.second - stop1.second)/(stop2.first - stop1.first);
    return lerpedValue;
}

/*!
 * \brief Evaluator::interpolate
 * Resolve the "interpolate" expression which performs an interpolation fiven a zoom level (limited only to linear interpolation)
 * \param array the QJsong array containing the expression to be resolved.
 * \param feature the feature on which expression operation will be performed.
 * \param mapZoomLevel the zoom level that will be used to resolve expressions that require a zoom parameter.
 * \param vpZoomeLevel
 * \return a QVariant containing the result of the interpolation.
 */
QVariant Evaluator::interpolate(const QJsonArray &array, const AbstractLayerFeature *feature, int mapZoomLevel, float vpZoomeLevel)
{
    QVariant returnVariant;
    //Loop over the values array starting at index 3 and find the two pairs that the value falls between.
    //We start at index 3 because element 0 contains the operation keyword, the element 1 contains the
    //element 1 contains the type of the interpolation, and element 2 contains the name of the value for the interpolation.
    if(mapZoomLevel <= array.at(3).toDouble()){//In case the value is less that the smallest element
        if(array.at(4).isArray()){
            return resolveExpression(array.at(4).toArray(), feature, mapZoomLevel, vpZoomeLevel);
        }else{
            return array.at(4).toDouble();
        }
    }else if(mapZoomLevel >= array.at(array.size()-2).toDouble()){//In case the value is greated than the largest element
        if(array.last().isArray()){
            return resolveExpression(array.last().toArray(), feature, mapZoomLevel, vpZoomeLevel);
        }else{
            return array.last().toDouble();
        }
    }else{ //In case the value falls between two elements
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

        return lerp(QPair<float, float>(stopInput1,stopOutput1), QPair<float, float>(stopInput2,stopOutput2), mapZoomLevel);
    }
}









