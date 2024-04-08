#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>

#include "Utilities.h"

#include "OutputTester.h"

namespace OutputTester = Bach::OutputTester;
using OutputTester::SimpleResult;
using OutputTester::SimpleError;

// Helper function to let us do early shutdown during startup.
[[noreturn]] void shutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

void removeOutputDirectoryIfPresent() {
    QDir expectedOutputFolder = OutputTester::buildBaselineExpectedOutputPath();
    if (expectedOutputFolder.exists()) {
        bool removeSuccess = expectedOutputFolder.removeRecursively();
        if (!removeSuccess) {
            shutdown("baseline folder already exists but was unable to delete it. Shutting down.");
        }
    }
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    removeOutputDirectoryIfPresent();

    std::optional<QFont> fontOpt = OutputTester::loadFont();
    if (!fontOpt.has_value()) {
        shutdown("Unable to load font file. Shutting down.");
    }
    const QFont &font = fontOpt.value();

    SimpleResult<void> success = OutputTester::iterateOverTestCases(
        font,
        [](
            int testId,
            const Bach::OutputTester::TestItem &testItem,
            const QImage &generatedImg)
        {

        QString expectedOutputPath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);

        bool writeToFileSuccess = Bach::writeImageToNewFileHelper(expectedOutputPath, generatedImg);
        if (!writeToFileSuccess) {
            shutdown("Failed to write image to file.");
        }
    });

    if (!success.success) {
        shutdown(success.errorMsg);
    }
}
