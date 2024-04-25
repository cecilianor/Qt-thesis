#include <QCoreApplication>
#include <QDir>

#include <TileLoader.h>

#include <chrono>
#include <iostream>


/*!
 * \brief iterations
 * Controls how many iterations we should do on each test.
 */
constexpr int iterations = 5;

constexpr bool loadFromMemory = false;

const TileCoord sameTileCoord = { 3, 1, 2 };

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
 * \param width Extracts N amount of tiles from the top-left square of the map.
 * This determines the width of that squared, measured in tiles.
 * \return A set of TileCoords
 */
static std::set<TileCoord> generateTileCoordSet()
{
    std::set<TileCoord> out;
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            out.insert(TileCoord{ 3, x, y });
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
    // Grab the first N
    std::set<TileCoord> out;
    for (int i = 0; i < count; i++) {
        out.insert(tempList[i]);
    }
    return out;
}

// Helper function to let us do early shutdown.
[[noreturn]] void shutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

void writeTestFilesToCacheDir(QString path)
{
    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            QString filename = QString("z3x%1y%2").arg(x).arg(y);

            QFile vectorFile { ":" + filename + ".mvt" };
            if (!vectorFile.open(QFile::ReadOnly)) {
                shutdown("Unable to open vector file");
            }

            QByteArray fileBytes = vectorFile.readAll();
            if (fileBytes.isEmpty()) {
                shutdown("Vector file was empty");
            }

            bool writeSuccess = Bach::writeTileToDiskCache_Vector(
                path,
                TileCoord{ 3, x, y },
                fileBytes);
            if (!writeSuccess) {
                shutdown("Unable to write file");
            }
        }
    }
}

QMap<TileCoord, QByteArray> loadTestFiles()
{
    QMap<TileCoord, QByteArray> out;

    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            QString filename = QString("z3x%1y%2").arg(x).arg(y);

            QFile vectorFile { ":" + filename + ".mvt" };
            if (!vectorFile.open(QFile::ReadOnly)) {
                shutdown("Unable to open vector file");
            }

            QByteArray fileBytes = vectorFile.readAll();
            if (fileBytes.isEmpty()) {
                shutdown("Vector file was empty");
            }

            out.insert(TileCoord{ 3, x, y }, fileBytes);
        }
    }
    return out;
}

// Write our vector tile files to temp directory.
void writeTestFilesToCacheDir_Dummy(QString path)
{
    QString filename = QString("z3x1y2");

    QFile file { ":" + filename + ".mvt" };
    if (!file.open(QFile::ReadOnly))
        shutdown("Unable to open vector file");
    QByteArray fileBytes = file.readAll();
    if (fileBytes.isEmpty()) {
        shutdown("Vector file was empty");
    }

    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            bool writeSuccess = Bach::writeTileToDiskCache_Vector(
                path,
                TileCoord{ 3, x, y },
                fileBytes);
            if (!writeSuccess) {
                shutdown("Unable to write file");
            }
        }
    }

}

QMap<TileCoord, QByteArray> loadTestFiles_Dummy(TileCoord coord)
{
    QMap<TileCoord, QByteArray> out;

    QString filename = QString("z%1x%2y%3")
        .arg(coord.zoom)
        .arg(coord.x)
        .arg(coord.y);

    QFile file { ":" + filename + ".mvt" };
    if (!file.open(QFile::ReadOnly))
        shutdown("Unable to open vector file");
    QByteArray fileBytes = file.readAll();

    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++) {
            out.insert(TileCoord{ 3, x, y }, fileBytes);
        }
    }

    return out;
}


/*!
 * \class
 * \brief The TestItem class is a helper class to
 * define a single test-configuration. It holds values as to what
 * tile-coords we should load and whether we should do so single or
 * multi-threadedly.
 */
struct TestItem {
    int threadCount;
    std::set<TileCoord> tileCoords;
};

/*!
 * \brief setupTestItems
 * Helper function to set up our list of
 * test items.
 * \return
 */
static QVector<TestItem> setupTestItems()
{
    std::set<TileCoord> set = generateTileCoordSet();

    QVector<TestItem> out;

    // Single thread, 1
    out.push_back({
        1,
        grabFirst(set, 1) });

    // Single thread, 4 tiles
    out.push_back({
        1,
        grabFirst(set, 4) });

    // Single thread, 8 tiles
    out.push_back({
        1,
        grabFirst(set, 8) });

    // Single thread, 16 tiles
    out.push_back({
        1,
        grabFirst(set, 16) });

    // Single thread, 32 tiles
    out.push_back({
        1,
        grabFirst(set, 32) });

    // Now duplicate the test cases but make them use 4 threads.
    int oldItemCount = out.size();
    for (int i = 0; i < oldItemCount; i++) {
        TestItem temp = out[i];
        temp.threadCount = 4;
        out.push_back(temp);
    }

    // Now duplicate the test cases but make them use 8 threads.
    for (int i = 0; i < oldItemCount; i++) {
        TestItem temp = out[i];
        temp.threadCount = 8;
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
    const QMap<TileCoord, QByteArray> *fileBytes,
    QString cacheDir)
{
    auto grabFileBytesFn = [&](TileCoord coord, TileType type) -> const QByteArray* {
        if (type == TileType::Raster) {
            return nullptr;
        }
        auto it = fileBytes->find(coord);
        if (it == fileBytes->end()) {
            return nullptr;
        }
        return &*it;
    };

    // The TileLoader always loads the tiles in the background,
    // in a non-blocking manner.
    // So we're gonna use an event-loop to block this function
    // until all tiles we requested are loaded.
    QEventLoop eventLoop;

    // Instantiate the TileLoader.
    std::unique_ptr<TileLoader> tileLoaderPtr = TileLoader::newDummy(
        cacheDir,
        fileBytes == nullptr ? nullptr : std::function(grabFileBytesFn),
        false, // Don't load raster tiles.
        testItem.threadCount);
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
                //static int counter = 0;
                //std::cout << "tile loaded " << counter++ << std::endl;
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

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "Iterations per test case: " << iterations << std::endl;
    if (loadFromMemory) {
        std::cout << "Loading tiles from memory" << std::endl;
    } else {
        std::cout << "Loading tiles from files" << std::endl;
    }

    std::cout << std::endl;

    {
        // Create the temp-dir that we want to store
        // our files into.
        Bach::TestUtils::TempDir tempDir;
        writeTestFilesToCacheDir(tempDir.path());

        QMap<TileCoord, QByteArray> memoryFiles = loadTestFiles();

        QVector<TestItem> testItems = setupTestItems();

        // Iterate over all test-items and run the benchmark on each of them.
        for (const TestItem &testItem : testItems) {
            double totalTestItemTime = 0;

            // Perform each test case N amount of times and calc the average.
            for (int iter = 0; iter < iterations; iter++) {
                double timeMilli = runSingleCase(
                    testItem,
                    loadFromMemory ? &memoryFiles : nullptr,
                    tempDir.path());
                totalTestItemTime += timeMilli;
            }

            // Print out the average time it took for this test case.
            // We print it out in seconds
            QString lineOut = QString("%1 thread(s), %2 tiles: avg. %3 millisec")
                .arg(testItem.threadCount)
                .arg(testItem.tileCoords.size())
                .arg(totalTestItemTime / iterations);
            std::cout << lineOut.toStdString() << std::endl;;
        }
    }

    std::cout << std::endl;
    QString temp = QString("Same-tile-test (z%1 x%2 y%3)")
        .arg(sameTileCoord.zoom)
        .arg(sameTileCoord.x)
        .arg(sameTileCoord.y);
    std::cout << temp.toStdString() << std::endl;
    {
        // Create the temp-dir that we want to store
        // our files into.
        Bach::TestUtils::TempDir tempDir;

        QMap<TileCoord, QByteArray> memoryFiles = loadTestFiles_Dummy(sameTileCoord);

        // Write our vector tile files to temp directory.
        writeTestFilesToCacheDir_Dummy(tempDir.path());

        QVector<TestItem> testItems = setupTestItems();

        // Iterate over all test-items and run the benchmark on each of them.
        for (const TestItem &testItem : testItems) {
            double totalTestItemTime = 0;

            // Perform each test case N amount of times and calc the average.
            for (int iter = 0; iter < iterations; iter++) {
                double timeMilli = runSingleCase(
                    testItem,
                    loadFromMemory ? &memoryFiles : nullptr,
                    tempDir.path());
                totalTestItemTime += timeMilli;
            }

            // Print out the average time it took for this test case.
            // We print it out in seconds
            QString lineOut = QString("%1 thread(s), %2 tiles: avg. %3 millisec")
                .arg(testItem.threadCount)
                .arg(testItem.tileCoords.size())
                .arg(totalTestItemTime / iterations);
            std::cout << lineOut.toStdString() << std::endl;;
        }
    }
}
