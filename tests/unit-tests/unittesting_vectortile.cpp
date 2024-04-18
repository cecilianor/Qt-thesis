// Qt header files
#include <QObject>
#include <QTest>

// Other header files
#include "VectorTiles.h"

class UnitTesting : public QObject
{
    Q_OBJECT

private slots:
    void tileFromByteArray_returns_basic_values();
};

QTEST_MAIN(UnitTesting)
#include "unittesting_vectortile.moc"

void testLayer(const TileLayer &layer, QString expectedName, int expectedExtent,
               int expectedNumberOfFeatures, int expectedVersion){
    QString errorMessage;
    errorMessage = QString("The layer name does not match, expected %1 but got %2")
                       .arg(expectedName)
                       .arg(layer.name());
    QVERIFY2(layer.name() == expectedName, errorMessage.toUtf8());

    errorMessage = QString("the layer's extent does not match, expected %1 but got %2")
                       .arg(expectedExtent)
                       .arg(layer.extent());
    QVERIFY2(layer.extent() == expectedExtent, errorMessage.toUtf8());

    errorMessage = QString("the number of features in the layer does not match, expected %1 but got %2")
                       .arg(expectedNumberOfFeatures)
                       .arg(layer.m_features.size());
    QVERIFY2(layer.m_features.size() == expectedNumberOfFeatures, errorMessage.toUtf8());

    errorMessage = QString("the layer's version does not match, expected %1 but got %2")
                       .arg(expectedVersion)
                       .arg(layer.version());
    QVERIFY2(layer.version() == expectedVersion, errorMessage.toUtf8());
}

void testTileLayers(const VectorTile &tile){
    int expectedNumberOfLayers = 6;
    QString errorMessage = QString("The number of layers in the tile does not match, expected %1 but got %2")
        .arg(expectedNumberOfLayers)
        .arg(tile.m_layers.size());
    QVERIFY2(tile.m_layers.size() == expectedNumberOfLayers, errorMessage.toUtf8());

    errorMessage = QString("the tile is missing layer: boundary");
    QVERIFY2(tile.m_layers.find("boundary") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("boundary")->second, "boundary", 4096, 717, 2);

    errorMessage = QString("the tile is missing layer: globallandcover");
    QVERIFY2(tile.m_layers.find("globallandcover") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("globallandcover")->second, "globallandcover", 4096, 6, 2);

    errorMessage = QString("the tile is missing layer: landcover");
    QVERIFY2(tile.m_layers.find("landcover") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("landcover")->second, "landcover", 4096, 11, 2);

    errorMessage = QString("the tile is missing layer: place");
    QVERIFY2(tile.m_layers.find("place") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("place")->second, "place", 4096, 43, 2);

    errorMessage = QString("the tile is missing layer: water");
    QVERIFY2(tile.m_layers.find("water") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("water")->second, "water", 4096, 2, 2);

    errorMessage = QString("the tile is missing layer: water_name");
    QVERIFY2(tile.m_layers.find("water_name") != tile.m_layers.end(), errorMessage.toUtf8());
    testLayer(*tile.m_layers.find("water_name")->second, "water_name", 4096, 7, 2);
}

void UnitTesting::tileFromByteArray_returns_basic_values()
{
    //Check that the file exists.
    QString path = ":/unitTestResources/000testTile.pbf";
    bool fileExist = QFile::exists(path);
    QString fileExistsError = "File \"" + path + "\" does not exist";
    QVERIFY2(fileExist == true, fileExistsError.toUtf8());

    //Open the json file and check that the operation was succesful.
    QFile tileFile(path);
    bool fileOpened = tileFile.open(QIODevice::ReadOnly);
    QString fileOpenError = "Could not open file";
    QVERIFY2(fileOpened == true, fileOpenError.toUtf8());

    std::optional<VectorTile> tile = Bach::tileFromByteArray(tileFile.readAll());
    QString readError = "Could not read file data";
    QVERIFY2(tile != std::nullopt, readError.toUtf8());
    testTileLayers(tile.value());
}
