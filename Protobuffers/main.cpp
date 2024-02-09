#include <QCoreApplication>
#include "VectorTiles.h"
#include "networkcontroller.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NetworkController netwrokCotroller;
    QByteArray responseData = netwrokCotroller.sendRequest("https://api.maptiler.com/tiles/v3/12/2170/1190.pbf?key=bWo4cKyYIs8K3SkrLiTk");
    VectorTile tile;
    tile.DeserializeMessage(responseData);
    return a.exec();
}
