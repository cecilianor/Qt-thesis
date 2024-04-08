#include <QFontDatabase>
#include <QTest>
#include <QProcess>
#include <QGuiApplication>

#include "OutputTester.h"
#include "Utilities.h"

namespace OutputTester = Bach::OutputTester;
using TestItem = OutputTester::TestItem;
using OutputTester::SimpleResult;
using Bach::OutputTester::SimpleError;

namespace Bach::TestUtils {
    class TempDir {
    public:
        TempDir()
        {
            auto sep = QDir::separator();

            QString uniqueDirName = QUuid::createUuid().toString();
            QString tempDirPath =
                QDir::tempPath() + sep +
                "Qt_thesis_unit_test_files" + sep +
                uniqueDirName;

            _dir = tempDirPath;
        }
        TempDir(const TempDir&) = delete;
        TempDir(TempDir&&) = delete;

        ~TempDir() {
            QDir temp{ _dir };
            bool removeSuccess = temp.removeRecursively();
            if (!removeSuccess) {
                qWarning() << "Unable to remove temp directory.";
            }
        }

        const QString& path() const { return _dir; }

    private:
        QString _dir;
    };
}

class RenderingTest : public QObject
{
    Q_OBJECT

public:
    Bach::TestUtils::TempDir _tempDir;
    QString tempDir() const { return _tempDir.path(); }

    const QFont &font() { return _font; }
    QFont _font;

    StyleSheet _stylesheet;
    const StyleSheet &stylesheet() { return _stylesheet; }

    QVector<TestItem> _testItems;
    const QVector<TestItem> &testItems() { return _testItems; }

private slots:
    void initTestCases();
    void compare_to_baseline_data();
    void compare_to_baseline();
};

QTEST_MAIN(RenderingTest)
#include "rendering_output_tests.moc"

void RenderingTest::initTestCases()
{
    // Check if a QGuiApplication already exists
    if (QGuiApplication::instance() == nullptr) {
        int argc = 0; // QGuiApplication expects an int&, so we make a temporary one.
        QGuiApplication(argc, nullptr); // Create a new instance if it does not exist
        if (QGuiApplication::instance() == nullptr) {
            qCritical() << "Error creating QGuiApplication.";
        }
    }

    // Load font
    {
        std::optional<QFont> fontOpt = OutputTester::loadFont();
        if (!fontOpt.has_value()) {
            QFAIL("Failed to load predetermined font file. Shutting down.");
        }
        this->_font = fontOpt.value();
    }

    // Load stylesheet
    {
        SimpleResult<StyleSheet> result = OutputTester::loadStylesheet();
        QVERIFY2(result.success, result.errorMsg.toUtf8());
        this->_stylesheet = std::move(result.value);
    }

    // Load test items
    {
        SimpleResult<QVector<TestItem>> result = OutputTester::loadTestItems();
        QVERIFY2(result.success, result.errorMsg.toUtf8());
        this->_testItems = result.value;
    }

    QDir dir { "renderoutput_failures" };
    if (dir.exists()) {
        bool removeSuccess = dir.removeRecursively();
        QVERIFY(removeSuccess);
    }
}

void RenderingTest::compare_to_baseline_data()
{
    QTest::addColumn<int>("testId");
    QTest::addColumn<TestItem>("testItem");

    for (int i = 0; i < testItems().size(); i++)
    {
        const TestItem &testItem = testItems()[i];
        QString rowName = QString("#%1").arg(i);
        if (testItem.name != "") {
            rowName += ": " + testItem.name;
        }

        QTest::newRow(rowName.toUtf8()) << i << testItem;
    }
}

struct RunImageComparison_Result {
    bool success;
    QString errorString;
};

RunImageComparison_Result runImageMagickV7(
    const QString &baselinePath,
    const QString &generatedPath,
    const QString &diffPath,
    int diffThresholdPercentage)
{
    QProcess process;
    QStringList arguments;

    arguments << "compare";
    arguments << "-metric";
    arguments << "AE";
    arguments << "-fuzz";
    arguments << QString("%1").arg(diffThresholdPercentage) + "%";
    arguments << baselinePath;
    arguments << generatedPath;
    arguments << diffPath;

    process.start("magick", arguments);

    bool finished = process.waitForFinished();
    if (!finished) {
        return {
            false,
            "Unable to run ImageMagick to compare images..."
        };
    }
    if (process.exitStatus() != QProcess::NormalExit) {
        return {
            false,
            "ImageMagick did not finish normally during comparison."
        };
    }

    if (process.exitCode() != 0) {
        return {
            false,
            "Reconstructed image is not equal to baseline."
        };
    }

    return {
        true,
        ""
    };
}

RunImageComparison_Result runImageMagickV6(
    const QString &baselinePath,
    const QString &generatedPath,
    const QString &diffPath,
    int diffThresholdPercentage)
{
    QProcess process;
    QStringList arguments;

    arguments << "-metric";
    arguments << "AE";
    arguments << "-fuzz";
    arguments << QString("%1").arg(diffThresholdPercentage) + "%";
    arguments << baselinePath;
    arguments << generatedPath;
    arguments << diffPath;

    process.start("compare", arguments);

    bool finished = process.waitForFinished();
    if (!finished) {
        return {
            false,
            "Unable to run ImageMagick to compare images..."
        };
    }
    if (process.exitStatus() != QProcess::NormalExit) {
        return {
            false,
            "ImageMagick did not finish normally during comparison."
        };
    }

    if (process.exitCode() != 0) {
        return {
            false,
            "Reconstructed image is not equal to baseline."
        };
    }

    return {
        true,
        ""
    };
}

RunImageComparison_Result runImageComparison(
    const QString &baselinePath,
    const QString &generatedPath,
    const QString &diffPath,
    int diffThresholdPercentage)
{
// Compile-check that the required defines are present.
#ifndef BACH_USE_IMAGEMAGICK_V7
#error "Program needs to know whether ImageMagick v6 or v7 should be used, \
    but couldn't find corresponding #define. \
    Likely a build error."
#endif
    // It's crucial that a macro defined as empty is not incorrectly processed as a 'false' value
    // This lambda is just a small fix to make sure this can't happen.
    auto makebool = [](bool in) { return in; };
    constexpr bool useImageMagickV7 = makebool(BACH_USE_IMAGEMAGICK_V7);

    if (useImageMagickV7) {
        return runImageMagickV7(
            baselinePath,
            generatedPath,
            diffPath,
            diffThresholdPercentage);
    } else {
        return runImageMagickV6(
            baselinePath,
            generatedPath,
            diffPath,
            diffThresholdPercentage);
    }
}

void writeIntoFailureReport(
    int testId,
    const QString &baselinePath,
    const QImage &generatedImg,
    const QString &diffPath)
{
    QDir dir { "renderoutput_failures" };
    // QFile won't create our directories for us.
    // We gotta make them ourselves.
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        qCritical() << "tried writing to file. Creating parent directory failed.\n";
    }

    QString failureReportDir = QString("renderoutput_failures");

    QString expectedOutputPath =
        failureReportDir +
        QDir::separator() +
        QString("%1_expected.png").arg(testId);
    bool expectedCopySuccess = QFile::copy(baselinePath, expectedOutputPath);
    if (!expectedCopySuccess) {
        qCritical() << "Failed to copy baseline file into failure report directory.";
    }

    bool generatedImgSaveSuccess = generatedImg.save(
        QString("renderoutput_failures/%1_generated.png").arg(testId));
    if (!generatedImgSaveSuccess) {
        qCritical() << "Failed to copy generated image into failure report directory.";
    }

    bool differCopySuccess = QFile::copy(
        diffPath,
        QString("renderoutput_failures/%1_diff.png").arg(testId));
    if (!differCopySuccess) {
        qCritical() << "Failed to copy failed baseline file into failure report directory.";
    }
}

void RenderingTest::compare_to_baseline() {

    QFETCH(int, testId);
    QFETCH(TestItem, testItem);

    QString tempDir = this->tempDir();

    SimpleResult<QImage> renderResult = OutputTester::render(
        testItem,
        stylesheet(),
        font());
    QVERIFY2(renderResult.success, renderResult.errorMsg.toUtf8());
    const QImage &generatedImg = renderResult.value;

    // Save to file
    QString generatedPath = QString(tempDir + QDir::separator() + "%1.png").arg(testId);
    bool writeSuccess = Bach::writeImageToNewFileHelper(
        generatedPath,
        generatedImg);
    QVERIFY2(writeSuccess, "Unable to write generated image to temporary file.");

    QString baselinePath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);
    QString diffPath = tempDir + QDir::separator() + "different.png";
    int diffThreshold = 5;

    RunImageComparison_Result imgCompareResult = runImageComparison(
        baselinePath,
        generatedPath,
        diffPath,
        diffThreshold);
    if (!imgCompareResult.success) {
        // Start writing the files to the failure report.
        writeIntoFailureReport(
            testId,
            baselinePath,
            generatedImg,
            diffPath);
    }
    QVERIFY2(imgCompareResult.success, imgCompareResult.errorString.toUtf8());
}
