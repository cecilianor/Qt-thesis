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

private:
    const QString expressionTestPath = ":/unitTestResources/expressionTest.json";

    // Opens and parses the test file.
    bool openAndParseTestFile(QFile &testFile, QJsonDocument &doc) {
        // Verify that the test file can be opened.
        testFile.setFileName(expressionTestPath);
        if(!testFile.open(QIODevice::ReadOnly))
            return false;

        // Verify that the test file got parsed correctly.
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(testFile.readAll(), &parseError);
        if(parseError.error != QJsonParseError::NoError)
            return false;

        return true; // Everything went okay, return true.
    }

    // Cleanup at the end of each test.
    void closeTestFile(QFile &testFile){
        testFile.close();
    }

    // Checks if expected and gotten double floating point values are within 0.0001 of each other.
    bool validDoubleRange(double double1, double double2)
    {
        auto epsilon = 0.0001;
        if (std::abs(double1 - double2) > epsilon)
            return false;
        if (std::abs(double2 - double1) > epsilon)
            return false;
        return true;
    }

private slots:
    void resolveExpression_returns_basic_values();
    void resolveExpression_with_coalesce_value();
    void resolveExpression_with_get_value();
    void resolveExpression_with_has_value();
    void resolveExpression_with_in_value();
    void resolveExpression_with_equals_value();
    void resolveExpression_with_inequality_value();
    void resolveExpression_with_greater_than_value();
    void resolveExpression_with_all_value();
    /*void resolveExpression_with_case_value();         // Test ON HOLD DUE TO FLOAT COMPARISON! */
    /*void resolveExpression_with_match_value();        // Test ON HOLD DUE TO FLOAT COMPARISON! */
    /*void resolveExpression_with_interpolate_value();  // Test ON HOLD DUE TO FLOAT COMPARISON! */
    /*void resolveExpression_with_compound_value();     // Test ON HOLD DUE TO FLOAT COMPARISON! */
};

QTEST_MAIN(UnitTesting)
#include "unittest_evaluator.moc"

// Test resolve expression function when the get value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_get_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("get").toObject();
    feature.fetureMetaData.insert("class", "grass");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"get\" function returns empty result when a string is expected");


    QVERIFY2(result.typeId() == QMetaType::Type::QString, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"get\" function, expected %1 but got %2")
                       .arg("grass",result.toString());
    QVERIFY2(result.toString() == "grass", errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"get\" function returns a valid variant when the result is expected to be non-valid");
    QVERIFY2(result.isValid() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `has` value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_has_value() {
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("has").toObject();
    feature.fetureMetaData.insert("subclass", "farm");
    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"has\" function returns empty result when a bool is expected");

    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"has\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"has\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"has\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `in` value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_in_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("in").toObject();
    feature.fetureMetaData.insert("class", "residential");
    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"in\" function returns empty result when a bool is expected");

    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"in\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"in\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"in\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `==` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_equals_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("==").toObject();
    feature.fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");

    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());

    expression = expressionObject.value("specialCase1").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("specialCase2").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `!=` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_inequality_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("!=").toObject();
    feature.fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"not equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"not equal\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"not equal\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"not equal\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `>` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_greater_than_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value(">").toObject();
    feature.fetureMetaData.insert("intermittent", 1);

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"greater\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"greater\" function, expected %1 but got %2")
                       .arg(true)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"greater\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"greater\" function, expected %1 but got %2")
                       .arg(false)
                       .arg(result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `all` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_all_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for file: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("all").toObject();
    feature.fetureMetaData.insert("class", "neighbourhood");
    feature.fetureMetaData.insert("intermittent", 1);
    feature.fetureMetaData.insert("subclass", "farm");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"all\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"all\" function, expected %1 but got %2")
                       .arg(true, result.toBool());
    QVERIFY2(result.toBool() == true, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"all\" function returns empty result when a bool is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Bool, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"all\" function, expected %1 but got %2")
                       .arg(false,result.toBool());
    QVERIFY2(result.toBool() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// Test resolve expression function when the `case` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
/* TEST ON HOLD DUE TO FLOAT AND INT COMPARISON OF VALUES
void UnitTesting::resolveExpression_with_case_value()
{
    //Parse the json file into a QJsonDocument for further processing.
    QJsonObject expressionObject = doc.object().value("case").toObject();
    feature.fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"case\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"case\" function, expected %1 but got %2")
                       .arg(15)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 15, errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"case\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::Double || result.typeId() == QMetaType::Type::LongLong, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"case\" function, expected %1 but got %2")
                       .arg(20)
                       .arg(result.toDouble());
    QVERIFY2(result.toDouble() == 20, errorMessage.toUtf8());
    closeTestFile(file);
}
*/

// Test resolve expression function when the `coalesce` expression object value is passed in.
// This function checks both for a valid (positive) and invalid (negative case).
void UnitTesting::resolveExpression_with_coalesce_value()
{
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());

    PolygonFeature feature;
    QString errorMessage;
    QJsonArray expression;
    QVariant result;

    QJsonObject expressionObject = doc.object().value("coalesce").toObject();
    feature.fetureMetaData.insert("class", "neighbourhood");

    expression = expressionObject.value("positive").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"coalesce\" function returns empty result when an int is expected");
    QVERIFY2(result.typeId() == QMetaType::Type::QString, errorMessage.toUtf8());
    errorMessage = QString("Wrong result from \"coalesce\" function, expected %1 but got %2")
                       .arg("neighbourhood", result.toString());
    QVERIFY2(result.toString() == "neighbourhood", errorMessage.toUtf8());

    expression = expressionObject.value("negative").toArray();
    result = Evaluator::resolveExpression(expression, &feature, 0, 0);
    errorMessage = QString("\"coalesce\" function returns a valid result when an empty variant is expected");
    QVERIFY2(result.isValid() == false, errorMessage.toUtf8());
    closeTestFile(file);
}

// ON HOLD DUE TO FLOAT COMPARISONS
void testMatchExpression(const QJsonObject &expressionObject, PolygonFeature *feature)
{
    /*
    QFile file;
    QJsonDocument doc;
    QVERIFY2(openAndParseTestFile(file, doc),
             " Opening and parsing failed for: " + expressionTestPath.toUtf8());
    */

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
    //closeTestFile(file);
}

// ON HOLD DUE TO FLOAT COMPARISONS
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

// ON HOLD DUE TO FLOAT COMPARISONS
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

    //testGetExpression(expressionsObject.value("get").toObject(), &testFeature);
    //testHasExpression(expressionsObject.value("has").toObject(), &testFeature);
    //testinExpression(expressionsObject.value("in").toObject(), &testFeature);
    //testEqualityExpression(expressionsObject.value("==").toObject(), &testFeature);
    //testInequalityExpression(expressionsObject.value("!=").toObject(), &testFeature);
    //testGreaterExpression(expressionsObject.value(">").toObject(), &testFeature);
    //testAllExpression(expressionsObject.value("all").toObject(), &testFeature);
    //testCaseExpression(expressionsObject.value("case").toObject(), &testFeature);
    //testCoalesceExpression(expressionsObject.value("coalesce").toObject(), &testFeature);

    /* The following three tests need updating before use due to float comparisons.
        They are currently ON-HOLD! */
    //testMatchExpression(expressionsObject.value("match").toObject(), &feature);
    //testInterpolateExpression(expressionsObject.value("interpolate").toArray(), &feature);
    //testCompoundExpression(expressionsObject.value("compound").toObject(), &feature);
}
