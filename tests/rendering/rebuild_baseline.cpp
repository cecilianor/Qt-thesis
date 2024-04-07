#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>

#include "Utilities.h"

#include "OutputTester.h"

// Helper function to let us do early shutdown during startup.
[[noreturn]] void shutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

void removeOutputDirectoryIfPresent() {
    QDir expectedOutputFolder = Bach::OutputTester::buildBaselineExpectedOutputPath();
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

    std::optional<QFont> fontOpt = Bach::OutputTester::loadFont();
    if (!fontOpt.has_value()) {
        shutdown("Unable to load font file. Shutting down.");
    }
    const QFont &font = fontOpt.value();

    bool success = Bach::OutputTester::test(
        font,
        [](int testId, const QImage &generatedImg) {

        QString expectedOutputPath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);

        bool writeToFileSuccess = Bach::writeImageToNewFileHelper(expectedOutputPath, generatedImg);
        if (!writeToFileSuccess) {
            shutdown("Failed to write image to file.");
        }
    });

    if (!success) {
        shutdown("Failed to process all test cases. Unknown error.");
    }
}
