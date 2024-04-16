#include <QCoreApplication>
#include <QDir>

#include <TileLoader.h>

#include <chrono>
#include <iostream>

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

using Bach::TileLoader;

/*!
 * \internal
 * \brief generateTileCoordSet
 * Helper function to generate a group of tiles.
 * It's hardcoded for zoom level 2.
 *
 *
 *
 * \param width Extracts N amount of tiles from the top-left square of the map.
 * This determines the width of that squared, measured in tiles.
 * \return A set of TileCoords
 */
static std::set<TileCoord> generateTileCoordSet(int width)
{
    std::set<TileCoord> out;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < width; y++) {
            out.insert(TileCoord{ 2, x, y });
        }
    }
    return out;
}

/*!
 * \brief grabFirst
 * Helper function. Grabs the first N elements of the set.
 * \param in Source set
 * \param count Number of items to grab.
 * \return
 */
static std::set<TileCoord> grabFirst(std::set<TileCoord> in, int count)
{
    // Turn it into a list.
    std::vector<TileCoord> tempList = { in.begin(), in.end() };
    // Grab the first 8
    std::set<TileCoord> out;
    for (int i = 0; i < count; i++) {
        out.insert(tempList[i]);
    }
    return out;
}

// Helper function to let us do early shutdown.
[[noreturn]] void earlyShutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

/*!
 * \internal
 * \brief writeTestFilesToCacheDir
 * Helper function for the setup of this benchmark.
 *
 * Installs all our test files into the disk cache for TileLoader to read.
 * \param path
 */
void writeTestFilesToCacheDir(QString path)
{
    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            QString filename = QString("z2x%1y%2").arg(x).arg(y);

            QFile vectorFile { ":" + filename + ".mvt" };
            if (!vectorFile.open(QFile::ReadOnly))
                earlyShutdown("Unable to open vector file.");

            QFile rasterFile { ":" + filename + ".png" };
            if (!rasterFile.open(QFile::ReadOnly))
                earlyShutdown("Unable to open raster file.");

            bool writeSuccess = Bach::writeTileToDiskCache(
                path,
                TileCoord{ 2, x, y },
                vectorFile.readAll(),
                rasterFile.readAll());
            if (!writeSuccess) {
                earlyShutdown("Unable to write file(s) to TileLoader disk cache folder.");
            }
        }
    }
}

/*!
 * \class
 * \brief The TestItem class is a helper class to
 * define a single test-configuration. It holds values as to what
 * tile-coords we should load and whether we should do so single or
 * multi-threadedly.
 */
struct TestItem {
    bool singleThread;
    std::set<TileCoord> tileCoords;

    QString name() const {
        QString out;
        if (singleThread) {
            out += "singlethread";
        } else {
            out += "multithread";
        }
        out += ", " + QString::number(tileCoords.size()) + " tile(s)";
        return out;
    }
};

/*!
 * \brief setupTestItems
 * Helper function to set up our list of
 * test items.
 * \return
 */
static QVector<TestItem> setupTestItems()
{
    std::set<TileCoord> smallSet = generateTileCoordSet(2);
    std::set<TileCoord> largeSet = generateTileCoordSet(4);

    QVector<TestItem> out;

    // Single thread, 1
    out.push_back({
        true,
        generateTileCoordSet(1) });

    // Single thread, 4 tiles
    out.push_back({
        true,
        smallSet });

    // Single thread, 8 tiles
    out.push_back({
        true,
        grabFirst(largeSet, 8) });

    // Single thread, 16 tiles
    out.push_back({
        true,
        largeSet });

    // Now duplicate the test cases except make them multithreaded.
    int oldItemCount = out.size();
    for (int i = 0; i < oldItemCount; i++) {
        TestItem temp = out[i];
        temp.singleThread = false;
        out.push_back(temp);
    }

    return out;
}

/*!
 * \brief runSingleCase
 * Runs the benchmark for a single test case and returns
 * time spent during the critical portion.
 *
 * \return Time spent in milliseconds.
 */
static double runSingleCase(
    const TestItem &testItem,
    QString cacheDir)
{
    // If we're gonna do a multi-threaded test, we pass in nullopt
    // to the TileLoader and it will select how many threads it wants.
    std::optional<int> threadWorkerCount = std::nullopt;
    if (testItem.singleThread) {
        threadWorkerCount = 1;
    }

    // The TileLoader always loads the tiles in the background,
    // in a non-blocking manner.
    // So we're gonna use an event-loop to block this function
    // until all tiles we requested are loaded.
    QEventLoop eventLoop;

    // Instantiate the TileLoader.
    std::unique_ptr<TileLoader> tileLoaderPtr = TileLoader::newDummy(
        cacheDir,
        threadWorkerCount);
    TileLoader& tileLoader = *tileLoaderPtr;

    // Time critical portion

    // The QBENCHMARK macro has no way for us to report the average time of each case.
    // QBENCHMARK will also only do 1 iteration of the benchmark, which gives us poor
    // sample size.
    // So we do it manually using std::chrono instead.
    auto timeStart = std::chrono::high_resolution_clock::now();

    // Count how many tiles we have loaded, when all are loaded we want
    // to break out.
    int tileLoadedCounter = 0;

    tileLoader.requestTiles(
        testItem.tileCoords,
        [&](TileCoord) {
            // This lambda is called on the TileLoader worker thread.
            // We don't have atomic access to the integer counter here.
            // So we dispatch it to the event-loop instead.

            QMetaObject::invokeMethod(&eventLoop, [&]() {

                // Increment counter, if all tiles are loaded, exit loop.
                tileLoadedCounter++;
                if (tileLoadedCounter >= testItem.tileCoords.size()) {
                    eventLoop.exit();
                }
            });
        });

    // This will block until all our tiles are done loading.
    eventLoop.exec();

    auto timeEnd = std::chrono::high_resolution_clock::now();

    // Calulate duration
    double timeDurMilli = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();
    return timeDurMilli;
}

/*!
 * \brief iterations
 * Controls how many iterations we should do on each test.
 */
constexpr int iterations = 3;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Create the temp-dir that we want to store
    // our files into.
    Bach::TestUtils::TempDir tempDir;

    // Write our vector tile files to temp directory.
    writeTestFilesToCacheDir(tempDir.path());

    QVector<TestItem> testItems = setupTestItems();

    std::cout << "Iterations per test case: " << iterations << std::endl;

    // Iterate over all test-items and run the benchmark on each of them.
    for (const TestItem &testItem : testItems) {
        double totalTestItemTime = 0;

        // Perform each test case N amount of times and calc the average.
        for (int iter = 0; iter < iterations; iter++) {

            double timeMilli = runSingleCase(testItem, tempDir.path());
            totalTestItemTime += timeMilli;
        }

        // Print out the average time it took for this test case.
        // We print it out in seconds
        QString lineOut = QString("%1: avg. %2 sec")
            .arg(testItem.name())
            .arg(totalTestItemTime / iterations / 1000);
        std::cout << lineOut.toStdString() << std::endl;;
    }
}
