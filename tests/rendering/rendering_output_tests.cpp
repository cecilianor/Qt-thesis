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
}

void RenderingTest::compare_generated_images_to_baseline() {
    QDir dir { "renderoutput_failures" };
    if (dir.exists()) {
        bool removeSuccess = dir.removeRecursively();
        QVERIFY(removeSuccess);
    }

    Bach::TestUtils::TempDir tempDir;

    bool success = Bach::OutputTester::test([&](int testId, const QImage &generatedImg) {
        QString baselinePath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);

        QString generatedPath = QString(tempDir.path() + QDir::separator() + "%1.png")
            .arg(testId);
        bool writeSuccess = Bach::writeImageToNewFileHelper(generatedPath, generatedImg);
        QVERIFY2(writeSuccess, "Unable to write generated image to temporary file.");

        QString diffPath = tempDir.path() + QDir::separator() + "different.png";

        QProcess process;
        QStringList arguments;
        // Compare the image to the baseline.
        // Construct the command arguments
        arguments << "compare";
        arguments << "-metric";
        arguments << "AE";
        //arguments << "-fuzz";
        //arguments << "5%"; // Adjust the fuzz factor as needed
        arguments << baselinePath;
        arguments << generatedPath;
        arguments << diffPath; // Discard output image

        // Set the program to 'magick' (or just 'compare' depending on your ImageMagick installation)
        process.start("magick", arguments);

        // Wait for the process to finish
        bool finished = process.waitForFinished();
        QVERIFY2(finished, "Unable to run ImageMagick to compare images...");
        QVERIFY2(process.exitStatus() == QProcess::NormalExit, "ImageMagick did not finish normally during comparison.");
        int exitCode = process.exitCode();
        if (exitCode != 0)
        {
            QDir dir { "renderoutput_failures" };
            // QFile won't create our directories for us.
            // We gotta make them ourselves.
            if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
                qCritical() << "tried writing to file. Creating parent directory failed.\n";
            }
            // If it failed we want to store the failed tests.

            QString expectedOutputPath = QString("renderoutput_failures/") + QString("%1_expected.png").arg(QString::number(testId));
            bool expectedCopySuccess = QFile::copy(
                baselinePath,
                expectedOutputPath);
            QVERIFY2(expectedCopySuccess, "Failed to copy failed baseline file into failure report directory.");

            bool generatedImgSaveSuccess = generatedImg.save(QString("renderoutput_failures/%1_generated.png").arg(testId));
            QVERIFY2(generatedImgSaveSuccess, "Failed to copy failed baseline file into failure report directory.");

            bool differCopySuccess = QFile::copy(
                diffPath,
                QString("renderoutput_failures/%1_diff.png").arg(testId));
            QVERIFY2(expectedCopySuccess, "Failed to copy failed baseline file into failure report directory.");
        }

        QString errorMsg = QString("ImageMagic said difference was too high at %1.").arg(exitCode);
        QVERIFY2(
            exitCode == 0,
            errorMsg.toUtf8());
    });

    QVERIFY2(success, "");
}
