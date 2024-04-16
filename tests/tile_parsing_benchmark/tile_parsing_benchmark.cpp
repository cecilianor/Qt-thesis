#include <QTest>

#include <VectorTiles.h>

#include <chrono>

class TileParsingBenchmark : public QObject {
    Q_OBJECT

public:
    // Load all the tiles into QByteArrays.
    QVector<QByteArray> _files;
    const QVector<QByteArray> &getFiles() const { return _files; }

    /*!
     * \brief
     * Number of iterations per test.
     */
    static constexpr int iterations = 3;

    double totalTime;

private slots:
    void initTestCases();
    void run();
};

QTEST_APPLESS_MAIN(TileParsingBenchmark)
#include "tile_parsing_benchmark.moc"

void TileParsingBenchmark::initTestCases()
{
    /* Load all our test files into
     * memory
     */
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            QString path = QString(":z1x%1y%2.mvt").arg(x).arg(y);
            QFile file{ path };
            bool fileOpenSuccess = file.open(QFile::ReadOnly);
            QVERIFY(fileOpenSuccess);
            QByteArray bytes = file.readAll();
            QVERIFY2(!bytes.isEmpty(), "Expected all files to not be empty.");
            _files.push_back(bytes);
        }
    }

    // Basic info about the test.
    qDebug() << "Parsing number of files: " << _files.size();
    qDebug() << "Number of test iterations: " << iterations;
}

void TileParsingBenchmark::run()
{
    // The QBENCHMARK macro has no way for us to report the average time of each case.
    // QBENCHMARK will also only do 1 iteration of the benchmark, which gives us poor
    // sample size.
    // So we do it manually using std::chrono instead.
    auto timeStart = std::chrono::high_resolution_clock::now();

    // Iterate over the entire N times.
    for (int i = 0; i < iterations; i++) {

        // Iterate over every file we have preloaded into memory.
        for (const QByteArray& bytes : getFiles()) {
            std::optional<VectorTile> newTileOpt = VectorTile::fromByteArray(bytes);
            QVERIFY2(newTileOpt.has_value(), "Benchmark expects all files to be parsed successfully.");
        }
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    // Calculate the total time it took to load.
    double totalTimeMilli = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();


    // This will output text that says we only ran one iteration.
    // So it's better to just print it manually during cleanup for this scenario.
    //QTest::setBenchmarkResult(totalTime, QTest::QBenchmarkMetric::WalltimeMilliseconds);

    qDebug() << "Total time: " << totalTime << " millisec";

    // Total amount of tiles we parsed.ns
    int tilesParsedTotal = getFiles().size() * iterations;

    qDebug() << "Average time per file: " << (totalTime / tilesParsedTotal) << " millisec";

}
