#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>

#include "Utilities.h"

#include "OutputTester.h"

namespace OutputTester = Bach::OutputTester;
using Bach::OutputTester::TestItem;
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

// Helper function to destroy the output folder if it already exists.
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

    // Load the font.
    std::optional<QFont> fontOpt = OutputTester::loadFont();
    if (!fontOpt.has_value()) {
        shutdown("Unable to load font file. Shutting down.");
    }
    const QFont &font = fontOpt.value();

    // Load the stylesheet.
    std::optional<StyleSheet> styleSheetResult =
        StyleSheet::fromJsonFile(OutputTester::getStyleSheetPath());
    if (!styleSheetResult.has_value()) {
        shutdown("Failed to load stylesheet file.");
    }
    const StyleSheet &styleSheet = styleSheetResult.value();

    // Load the test items.
    SimpleResult<QVector<TestItem>> testItemsResult = OutputTester::loadTestItems();
    if (!testItemsResult.success) {
        shutdown(testItemsResult.errorMsg);
    }
    const QVector<TestItem> &testItems = testItemsResult.value;

    // Iterate over the test cases.
    // Try to render them and output to disk.
    for (int i = 0; i < testItems.size(); i++) {
        const TestItem &testItem = testItems[i];
        int testId = i;

        SimpleResult<QImage> renderResult = render(
            testItem,
            styleSheet,
            font);
        if (!renderResult.success) {

            shutdown(
                QString("Error in test case #%1: ").arg(testId) +
                renderResult.errorMsg);
        }
        const QImage& generatedImg = renderResult.value;

        QString expectedOutputPath = Bach::OutputTester::buildBaselineExpectedOutputPath(testId);

        bool writeToFileSuccess = Bach::writeImageToNewFileHelper(expectedOutputPath, generatedImg);
        if (!writeToFileSuccess) {
            shutdown("Failed to write image to file.");
        }
    }
}
