#include <QFontDatabase>
#include <QTest>
#include <QProcess>
#include <QGuiApplication>

#include "Bach/Merlin/Merlin.h"
#include "Utilities.h"

namespace Merlin = Bach::Merlin;
using TestItem = Merlin::TestItem;
using Merlin::SimpleResult;
using Merlin::SimpleError;

namespace Bach::TestUtils {
    /*!
     * \brief The TempDir class is
     * a helper class that creates a temporary folder
     * inside the system's 'temp' folder. The folder
     * is automatically destroyed when this class is destroyed.
     */
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
    /*!
     * \brief tempDir
     * \return Returns the path to the temporary directory
     * of this test run.
     */
    QString tempDir() const { return _tempDir.path(); }
    Bach::TestUtils::TempDir _tempDir;

    /*!
     * \brief font
     * \return Returns the predetermined QFont to
     * use for rendering.
     */
    const QFont &font() { return _font; }
    QFont _font;

    const StyleSheet &stylesheet() { return _stylesheet; }
    StyleSheet _stylesheet;

    /*!
     * \brief testItems
     * \return Returns the list of predetermined test cases
     * for this test run.
     */
    const QVector<TestItem> &testItems() { return _testItems; }
    QVector<TestItem> _testItems;

private slots:
    void initTestCases();
    void compare_to_baseline_data();
    void compare_to_baseline();
};

QTEST_MAIN(RenderingTest)
#include "rendering_output_tests.moc"

void RenderingTest::initTestCases()
{
    // Check if a QGuiApplication already exists,
    // if not, create one.
    // A QGuiApplication is required to do QPainter commands.
    if (QGuiApplication::instance() == nullptr) {
        int argc = 0; // QGuiApplication expects an int&, so we make a temporary one.
        QGuiApplication(argc, nullptr); // Create a new instance if it does not exist
    }

    // Load font
    {
        std::optional<QFont> fontOpt = Merlin::loadFont();
        QVERIFY2(fontOpt.has_value(), "Failed to load predetermined font file.");
        this->_font = fontOpt.value();
    }

    // Load stylesheet
    {
        SimpleResult<StyleSheet> result = Merlin::loadStylesheet();
        QVERIFY2(result.success, result.errorMsg.toUtf8());
        this->_stylesheet = std::move(result.value);
    }

    // Load test items
    {
        SimpleResult<QVector<TestItem>> result = Merlin::loadTestItems();
        QVERIFY2(result.success, result.errorMsg.toUtf8());
        this->_testItems = result.value;
    }

    // Destroy the failure report directory if it already exists.
    QDir dir { "renderoutput_failures" };
    if (dir.exists()) {
        bool removeSuccess = dir.removeRecursively();
        QVERIFY2(
            removeSuccess,
            "Failure report directory already exists, but failed to destroy it.");
    }
}

void RenderingTest::compare_to_baseline_data()
{
    QTest::addColumn<int>("testId");
    QTest::addColumn<TestItem>("testItem");

    // Populate the data for the test.
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

/*!
 * \internal
 * \brief runImageMagickV7
 * Runs the ImageMagick process on our test case.
 *
 * Uses the ImageMagick v7 command line interface specifically.
 * Will not work if v6 is installed.
 *
 * \param baselinePath File-path to the baseline file.
 * \param generatedPath File-path to the newly generated file.
 * \param diffPath File-path for where to store the diffed file.
 * \param diffThresholdPercentage The threshold to allow to pass differences.
 * Measured in percentage compared to Absolute Error. Read more about it
 * in ImageMagick documentation.
 *
 * \return If the operation was successful, returns a boolean of whether the
 * test was acceptable.
 *
 * If the operation was unsuccessful, will return an error message.
 */
static SimpleResult<bool> runImageMagickV7(
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
        return SimpleError{ "Unable to run ImageMagick to compare images." };
    }
    if (process.exitStatus() != QProcess::NormalExit) {
        return SimpleError { "ImageMagick did not finish normally during comparison." };
    }

    return process.exitCode() == 0;
}


/*!
 * \internal
 * \brief runImageMagickV6
 * Runs the ImageMagick process on our test case.
 *
 * Uses the ImageMagick v6 command line interface specifically.
 * Will not work if v7 is installed.
 *
 * \param baselinePath File-path to the baseline file.
 * \param generatedPath File-path to the newly generated file.
 * \param diffPath File-path for where to store the diffed file.
 * \param diffThresholdPercentage The threshold to allow to pass differences.
 * Measured in percentage compared to Absolute Error. Read more about it
 * in ImageMagick documentation.
 *
 * \return If the operation was successful, returns a boolean of whether the
 * test was acceptable.
 *
 * If the operation was unsuccessful, will return an error message.
 */
static SimpleResult<bool> runImageMagickV6(
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
        return SimpleError{ "Unable to run ImageMagick to compare images." };
    }
    if (process.exitStatus() != QProcess::NormalExit) {
        return SimpleError { "ImageMagick did not finish normally during comparison." };
    }

    return process.exitCode() == 0;
}

/*!
 * \internal
 * \brief runImageComparison
 * Performs the main image comparison acceptability test.
 *
 * \param baselinePath File-path to the baseline file.
 * \param generatedPath File-path to the newly generated file.
 * \param diffPath File-path for where to store the diffed file.
 * \param diffThresholdPercentage The threshold to allow to pass differences.
 * Measured in percentage compared to Absolute Error. Read more about it
 * in ImageMagick documentation.
 *
 * \return If the operation was successful, returns a boolean of whether the
 * test was acceptable.
 *
 * If the operation was unsuccessful, will return an error message.
 */
static SimpleResult<bool> runImageComparison(
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

/*!
 * \brief writeIntoFailureReport
 * Writes the test case files into the failure report folder.
 *
 * \param testId
 * \param baselinePath
 * \param generatedImg
 * \param diffPath
 *
 * \return Returns success if no errors was encountered.
 * Otherwise returns an error message.
 */
static SimpleResult<void> writeIntoFailureReport(
    int testId,
    const QString &baselinePath,
    const QImage &generatedImg,
    const QString &diffPath)
{
    QDir dir { "renderoutput_failures" };
    // QFile won't create our directories for us.
    // We gotta make them ourselves.
    if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
        return SimpleError{
            "Tried writing file to failure report directory. Creating parent directory failed." };
    }

    QString failureReportDir = QString("renderoutput_failures");

    QString expectedOutputPath =
        failureReportDir +
        QDir::separator() +
        QString("%1_expected.png").arg(testId);
    bool expectedCopySuccess = QFile::copy(baselinePath, expectedOutputPath);
    if (!expectedCopySuccess) {
        return SimpleError{
            "Failed to copy baseline file into failure report directory." };
    }

    bool generatedImgSaveSuccess = generatedImg.save(
        QString("renderoutput_failures/%1_generated.png").arg(testId));
    if (!generatedImgSaveSuccess) {
        return SimpleError{
            "Failed to copy generated image into failure report directory." };
    }

    bool differCopySuccess = QFile::copy(
        diffPath,
        QString("renderoutput_failures/%1_diff.png").arg(testId));
    if (!differCopySuccess) {
        return SimpleError{
            "Failed to copy diff image into failure report directory." };
    }

    return {};
}

void RenderingTest::compare_to_baseline() {

    QFETCH(int, testId);
    QFETCH(TestItem, testItem);

    QString tempDir = this->tempDir();

    // Render our image.
    SimpleResult<QImage> renderResult = Merlin::render(
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

    QString baselinePath = Bach::Merlin::buildBaselineExpectedOutputPath(testId);
    QString diffPath = tempDir + "/different.png";
    int diffThreshold = 5;

    SimpleResult<bool> imgCompareResult = runImageComparison(
        baselinePath,
        generatedPath,
        diffPath,
        diffThreshold);
    // If the operation itself failed, or the image comparison was not acceptable,
    // then we write the failed test case to file.
    if (!imgCompareResult.success || !imgCompareResult.value) {
        // Start writing the files to the failure report.
        writeIntoFailureReport(
            testId,
            baselinePath,
            generatedImg,
            diffPath);
    }

    QVERIFY2(
        imgCompareResult.success,
        imgCompareResult.errorMsg.toUtf8());
    QVERIFY2(
        imgCompareResult.value,
        "Generated image is not equal to baseline.");
}
