#include <QtLogging>
#include <QDebug>

#include <VectorTiles.h>

#include <chrono>
#include <vector>

// Helper function to let us do early shutdown.
[[noreturn]] void shutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

static std::vector<QByteArray> loadTestFilesIntoMemory() {
    std::vector<QByteArray> out;
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            QString path = QString(":z2x%1y%2.mvt").arg(x).arg(y);
            QFile file{ path };
            bool fileOpenSuccess = file.open(QFile::ReadOnly);
            if (!fileOpenSuccess) {
                shutdown("Unable to open file");
            }

            QByteArray bytes = file.readAll();
            if (bytes.isEmpty()) {
                shutdown("Expected all files to not be empty.");
            }
            out.push_back(bytes);
        }
    }
    return out;
}

/*!
 * \brief
 * Number of iterations per test.
 */
static constexpr int iterations = 5;

int main() {
    std::vector<QByteArray> testFiles = loadTestFilesIntoMemory();

    // Basic info about the test.
    qDebug() << "Parsing number of files: " << testFiles.size();
    qDebug() << "Number of test iterations: " << iterations;

    auto timeStart = std::chrono::high_resolution_clock::now();

    // Iterate over the entire N times.
    for (int i = 0; i < iterations; i++) {
        // Iterate over every file we have preloaded into memory.
        for (const QByteArray& bytes : testFiles) {
            std::optional<VectorTile> newTileOpt = VectorTile::fromByteArray(bytes);
            if (!newTileOpt.has_value()) {
                shutdown("Benchmark expects all files to be parsed successfully.");
            }
        }
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();


    // Calculate the total time it took to load.
    double totalTimeMilli = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();


    qDebug() << "Total time: " << totalTimeMilli << " millisec";

    // Total amount of tiles we parsed.ns
    int tilesParsedTotal = testFiles.size() * iterations;

    qDebug() << "Average time per file: " << (totalTimeMilli / tilesParsedTotal) << " millisec";
}
