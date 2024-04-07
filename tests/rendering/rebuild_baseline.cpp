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

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QFontDatabase::removeAllApplicationFonts();
    int fontId = QFontDatabase::addApplicationFont(
        Bach::OutputTester::buildBaselinePath() +
        QDir::separator() +
        "Roboto-Regular.ttf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
        QFont appFont(fontFamilies.first());
        QGuiApplication::setFont(appFont);
    }

    QDir expectedOutputFolder = Bach::OutputTester::buildBaselineExpectedOutputPath();
    if (expectedOutputFolder.exists()) {
        bool removeSuccess = expectedOutputFolder.removeRecursively();
        if (!removeSuccess) {
            shutdown("baseline folder already exists but was unable to delete it. Shutting down.");
        }
    }

    bool success = Bach::OutputTester::test([](int testId, const QImage &generatedImg) {

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
