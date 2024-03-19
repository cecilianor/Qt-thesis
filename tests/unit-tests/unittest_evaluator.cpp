#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QObject>
#include <QTest>

#include "Evaluator.h"

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:
    void resolveExpression_returns_basic_values();

};

QTEST_MAIN(UnitTesting)
#include "unittest_evaluator.moc"

void testGetExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{
    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "grass");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"get\" function returns empty result when a string is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::QString, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"get\" function, expected %1 but got %2")
                       .arg("grass")
                       .arg(result.toString());
    QVERIFY2(result.toString() == "grass", errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"get\" function returns a valid variant when the result is expected to be non-valid");
    QVERIFY2(result.isValid() == false, errorMessage.toUtf8());

}

void testHasExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("subclass", "farm");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"has\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"has\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"has\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"has\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());

}

void testinExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{


    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "residential");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"in\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"in\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"in\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"in\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());

}
void testEqualityExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{
    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());

    expression = expressionObject.value("specialCase1").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("specialCase2").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
}

void testInequalityExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"not equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"not equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"not equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"not equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
}

void testGreaterExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("intermittent", 1);


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"greater\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"greater\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"greater\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"greater\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
}

void testAllExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");
    feature->fetureMetaData.insert("intermittent", 1);
    feature->fetureMetaData.insert("subclass", "farm");


    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"all\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"all\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"all\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"all\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
}

void testCaseExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{
    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"case\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"case\" function, expected %1 but got %2")
                       .arg(15)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 15, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"case\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"case\" function, expected %1 but got %2")
                       .arg(20)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 20, errorMessage.toUtf8());
}

void testCoalesceExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"coalesce\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::QString, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"coalesce\" function, expected %1 but got %2")
                       .arg("neighbourhood")
                       .arg(result.toString());
    QVERIFY2(result.toString() == "neighbourhood", errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"coalesce\" function returns a valid result when an empty variant is expected");
    QVERIFY2(result.isValid() == false, errorMessage.toUtf8());

}

void testMatchExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{

    QString errorMessage;
    QJsonArray expression;
    QVariant result;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"match\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"match\" function, expected %1 but got %2")
                       .arg(2)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 2, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("\"match\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"match\" function, expected %1 but got %2")
                       .arg(4)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 4, errorMessage.toUtf8());

}

void testInterpolateExpression(const QJsonArray &expression, PolygonFeature *feature)
{
    QString errorMessage;
    QVariant result;
    float expectedInterpolationResult;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "neighbourhood");


    expectedInterpolationResult = 11;
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 0, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    result = Evaluator::resolveExpression(expression, feature, 3, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 3, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 11 + (2.*2/5);
    result = Evaluator::resolveExpression(expression, feature, 5, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 5, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 11 + 4.*2/5;
    result = Evaluator::resolveExpression(expression, feature, 7, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 7, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 13 + 2.*3/3;
    result = Evaluator::resolveExpression(expression, feature, 10, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 10, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 16;
    result = Evaluator::resolveExpression(expression, feature, 11, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 11, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 16 + (2.*5/5);
    result = Evaluator::resolveExpression(expression, feature, 13, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 13, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 16 + (4.*5/5);
    result = Evaluator::resolveExpression(expression, feature, 15, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 15, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult = 21;
    result = Evaluator::resolveExpression(expression, feature, 18, 0);
    errorMessage = QString("Wrong result from \"interpolate\" functionfor zoom level 18, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());


}

void testCompoundExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{
    QString errorMessage;
    QJsonArray expression = expressionObject.value("expression1").toArray();
    QVariant result;
    float expectedInterpolationResult;
    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "motorway");

    expectedInterpolationResult = 0.5;
    result = Evaluator::resolveExpression(expression, feature, 0, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 0, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());
    expectedInterpolationResult =1 + (1*1.5/4);
    result = Evaluator::resolveExpression(expression, feature, 7, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 7, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    feature->fetureMetaData.insert("brunnel", "bridge");
    expectedInterpolationResult =1 + (1*(-1.)/4);
    result = Evaluator::resolveExpression(expression, feature, 7, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 7, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult =2;
    result = Evaluator::resolveExpression(expression, feature, 11, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 11, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    feature->fetureMetaData.insert("ramp", 1);
    expectedInterpolationResult =0.5;
    result = Evaluator::resolveExpression(expression, feature, 11, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 11, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    feature->fetureMetaData.clear();
    feature->fetureMetaData.insert("class", "service");
    expectedInterpolationResult =0.75;
    result = Evaluator::resolveExpression(expression, feature, 11, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 11, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult =2+(2*14./4);
    result = Evaluator::resolveExpression(expression, feature, 18, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 18, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());

    expectedInterpolationResult =16;
    result = Evaluator::resolveExpression(expression, feature, 30, 0);
    errorMessage = QString("Wrong result from \"interpolate\" function for zoom level 30, expected %1 but got %2")
                       .arg(expectedInterpolationResult)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == expectedInterpolationResult, errorMessage.toUtf8());
}

void UnitTesting::resolveExpression_returns_basic_values()
{
    //Check that the file exists.
    QString path = ":/unitTestResources/expressionTest.json";
    bool fileExist = QFile::exists(path);
    QString fileExistsError = "File \"" + path + "\" does not exist";
    QVERIFY2(fileExist == true, fileExistsError.toUtf8());

    //Open the json file and check that the operation was succesful.
    QFile expressionFile(path);
    bool fileOpened = expressionFile.open(QIODevice::ReadOnly);
    QString fileOpenError = "Could not open file";
    QVERIFY2(fileOpened == true, fileOpenError.toUtf8());

    //Parse the json file into a QJsonDocument for further processing.
    QJsonDocument doc;
    QJsonParseError parseError;
    doc = QJsonDocument::fromJson(expressionFile.readAll(), &parseError);

    //Check for parsing errors.
    QString parErrorString = "The Qt parser encountered an error";
    QVERIFY2(parseError.error == QJsonParseError::NoError, parErrorString.toUtf8());

    QJsonObject expressionsObject = doc.object();
    PolygonFeature testFeature;
    testGetExpression(expressionsObject.value("get").toObject(), &testFeature);
    testHasExpression(expressionsObject.value("has").toObject(), &testFeature);
    testinExpression(expressionsObject.value("in").toObject(), &testFeature);
    testEqualityExpression(expressionsObject.value("==").toObject(), &testFeature);
    testInequalityExpression(expressionsObject.value("!=").toObject(), &testFeature);
    testGreaterExpression(expressionsObject.value(">").toObject(), &testFeature);
    testAllExpression(expressionsObject.value("all").toObject(), &testFeature);
    testCaseExpression(expressionsObject.value("case").toObject(), &testFeature);
    testCoalesceExpression(expressionsObject.value("coalesce").toObject(), &testFeature);
    testMatchExpression(expressionsObject.value("match").toObject(), &testFeature);
    testInterpolateExpression(expressionsObject.value("interpolate").toArray(), &testFeature);
    testCompoundExpression(expressionsObject.value("compound").toObject(), &testFeature);
}
