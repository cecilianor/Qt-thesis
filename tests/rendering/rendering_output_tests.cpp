#include <QObject>
#include <QTest>
#include <QProcess>

#include "OutputTester.h"
#include "Utilities.h"

class RenderingTest : public QObject
{
    Q_OBJECT

private slots:
    void compare_generated_images_to_baseline();
};

QTEST_MAIN(RenderingTest)
#include "rendering_output_tests.moc"

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
};

struct RunImageComparison_Result {
    bool success;
    QString errorString;
};

RunImageComparison_Result runImageMagickV7(
    const QString &baselinePath,
    const QString &generatedPath,
    const QString &diffPath)
{
    QProcess process;
    QStringList arguments;

    arguments << "compare";
    arguments << "-metric";
    arguments << "AE";
    //arguments << "-fuzz";
    //arguments << "5%"; // Adjust the fuzz factor as needed
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
    const QString &diffPath)
{
    QProcess process;
    QStringList arguments;

    arguments << "-metric";
    arguments << "AE";
    //arguments << "-fuzz";
    //arguments << "5%"; // Adjust the fuzz factor as needed
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
    const QString &diffPath)
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
        return runImageMagickV7(baselinePath, generatedPath, diffPath);
    } else {
        return runImageMagickV6(baselinePath, generatedPath, diffPath);
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

void RenderingTest::compare_generated_images_to_baseline() {
    QDir dir { "renderoutput_failures" };
    if (dir.exists()) {
        bool removeSuccess = dir.removeRecursively();
        QVERIFY(removeSuccess);
    }

    Bach::TestUtils::TempDir tempDir;

    bool testSuccess = true;

    bool iterateCasesSuccess = Bach::OutputTester::test([&](int testId, const QImage &generatedImg) {
        QString baselinePath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);

        QString generatedPath = QString(tempDir.path() + QDir::separator() + "%1.png")
            .arg(testId);
        bool writeSuccess = Bach::writeImageToNewFileHelper(generatedPath, generatedImg);
        if (!writeSuccess) {
            qCritical() << "Unable to write generated image to temporary file." ;
        }

        QString diffPath = tempDir.path() + QDir::separator() + "different.png";

        RunImageComparison_Result imgCompareResult = runImageComparison(
            baselinePath,
            generatedPath,
            diffPath);

        if (!imgCompareResult.success) {
            testSuccess = false;
            qCritical() <<
                QString("Error during comparison on test-case #%1: ").arg(testId) +
                imgCompareResult.errorString;

            // Start writing the files to the failure report.
            writeIntoFailureReport(
                testId,
                baselinePath,
                generatedImg,
                diffPath);
        }
    });

    if (!iterateCasesSuccess) {
        testSuccess = false;
    }

    if (!testSuccess) {
        qCritical() << "Failure report can be found in folder 'renderoutput_failures'";
    }
    QVERIFY(testSuccess);
}
