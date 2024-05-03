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

// Helper function to let us do early shutdown.
[[noreturn]] void shutdown(const QString &msg = "")
{
    if (msg != "") {
        qCritical() << msg;
    }
    std::exit(EXIT_FAILURE);
}

static QMap<TileCoord, QString> loadTilePaths() {
    QMap<TileCoord, QString> out;

    QDir dir{":"};
    QList<QString> fileList = dir.entryList(QDir::Filter::Files);
    if (fileList.isEmpty()) {
        shutdown("No files found");
    }

    static const QRegularExpression re("z(\\d+)x(\\d+)y(\\d+)\\.mvt");

    // Iterate over each file name in the list
    for (const QString &filePath : fileList) {
        QRegularExpressionMatch match = re.match(filePath);
        if (!match.hasMatch()) {
            shutdown("No match found for file");
        }

        TileCoord coord;
        coord.zoom = match.captured(1).toInt();
        coord.x = match.captured(2).toInt();
        coord.y = match.captured(3).toInt();

        out.insert(coord, ":" + filePath);
    }

    return out;
}

static QVector<TileCoord> loadFullTileCoordList_Sorted() {
    QMap<TileCoord, QString> tileFilePaths = loadTilePaths();

    std::vector<std::pair<TileCoord, qint64>> tileCoordPairs;

    // Iterate over each file name in the list
    for (const auto &[coord, filePath] : tileFilePaths.asKeyValueRange()) {
        QFile file{ filePath };
        if (!file.open(QFile::ReadOnly)) {
            shutdown("No match found for file");
        }

        qint64 fileByteCount = file.size();

        tileCoordPairs.push_back({ coord, fileByteCount });
    }
    std::sort(
        tileCoordPairs.begin(),
        tileCoordPairs.end(),
        [](const std::pair<TileCoord, qint64> &a, const std::pair<TileCoord, qint64> &b) {
            return a.second > b.second;
        });
    QVector<TileCoord> sortedTileCoords;
    for (const std::pair<TileCoord, qint64> &item : tileCoordPairs) {
        sortedTileCoords.push_back(item.first);
    }
    return sortedTileCoords;
}

static QMap<TileCoord, QByteArray> loadTileFiles()
{
    QMap<TileCoord, QByteArray> out;

    QMap<TileCoord, QString> filePaths = loadTilePaths();
    for (const auto &[coord, path] : filePaths.asKeyValueRange()) {
        QFile file{ path };
        if (!file.open(QFile::ReadOnly)) {
            shutdown("Unable to open vector file");
        }

        QByteArray fileBytes = file.readAll();

        if (fileBytes.isEmpty()) {
            shutdown("Vector file was empty");
        }

        out.insert(coord, fileBytes);
    }

    return out;
}

static QMap<TileCoord, QByteArray> loadTileFiles_Dummy()
{
    QMap<TileCoord, QByteArray> out;

    QMap<TileCoord, QString> filePaths = loadTilePaths();
    for (const auto &[coord, path] : filePaths.asKeyValueRange()) {
        QFile file{ path };
        if (!file.open(QFile::ReadOnly)) {
            shutdown("Unable to open vector file");
        }

        QByteArray fileBytes = file.readAll();

        if (fileBytes.isEmpty()) {
            shutdown("Vector file was empty");
        }

        out.insert(coord, fileBytes);
    }

    return out;
}

void writeTestFilesToCacheDir(const QString &outPath)
{
    QMap<TileCoord, QByteArray> tileFiles = loadTileFiles();

    // Write all our files to the temp directory so that the TileLoader will be able to load from it.
    for (const auto &[coord, fileBytes] : tileFiles.asKeyValueRange()) {
        bool writeSuccess = Bach::writeTileToDiskCache_Vector(
            outPath,
            coord,
            fileBytes);
        if (!writeSuccess) {
            shutdown("Unable to write file");
        }
    }
}

// Write our vector tile files to temp directory.
static void writeTestFilesToCacheDir_Dummy(
    const QString &path)
{
    // Load the biggest file.
    QVector<TileCoord> allCoords = loadFullTileCoordList_Sorted();

    QMap<TileCoord, QByteArray> tileFiles = loadTileFiles();

    TileCoord biggestTileCoord = allCoords[0];
    QByteArray biggestTileFile = tileFiles[biggestTileCoord];

    for (TileCoord coord : allCoords) {
        bool writeSuccess = Bach::writeTileToDiskCache_Vector(
            path,
            coord,
            biggestTileFile);
        if (!writeSuccess) {
            shutdown("Unable to write file");
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
    QVector<TileCoord> coordsSortedBySize = loadFullTileCoordList_Sorted();

    auto grabFirst = [&](int n) {
        std::set<TileCoord> out;
        for (TileCoord item : coordsSortedBySize.first(n)) {
            out.insert(item);
        }
        return out;
    };

    QVector<TestItem> out;

    // Single thread, 1
    out.push_back({
        1,
        grabFirst(1) });

    // Single thread, 4 tiles
    out.push_back({
        1,
        grabFirst(4) });

    // Single thread, 8 tiles
    out.push_back({
        1,
        grabFirst(8) });

    // Single thread, 16 tiles
    out.push_back({
        1,
        grabFirst(16) });

    // Single thread, 32 tiles
    out.push_back({
        1,
        grabFirst(32) });

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
    if (QThread::idealThreadCount() > 8) {
        // Now duplicate the test cases but make them use 16 threads.
        for (int i = 0; i < oldItemCount; i++) {
            TestItem temp = out[i];
            temp.threadCount = QThread::idealThreadCount();
            out.push_back(temp);
        }
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

    QMap<TileCoord, QString> tileFilePaths = loadTilePaths();
    // Sorted by size of the corresponding file.
    QVector<TileCoord> tileCoordsSorted = loadFullTileCoordList_Sorted();
    std::pair<TileCoord, QString> biggestTile = {
        tileCoordsSorted.first(),
        tileFilePaths[tileCoordsSorted.first()]
    };

    QVector<TestItem> testItems = setupTestItems();

    {
        // Create the temp-dir that we want to store
        // our files into.
        Bach::TestUtils::TempDir tempDir;
        writeTestFilesToCacheDir(tempDir.path());

        QMap<TileCoord, QByteArray> memoryFiles = loadTileFiles();

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
    std::cout << "Same-tile test" << std::endl;
    {
        // Create the temp-dir that we want to store
        // our files into.
        Bach::TestUtils::TempDir tempDir;

        QMap<TileCoord, QByteArray> memoryFiles = loadTileFiles_Dummy();

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
