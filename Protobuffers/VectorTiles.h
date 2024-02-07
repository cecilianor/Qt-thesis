#ifndef VECTORTILES_H
#define VECTORTILES_H

#include <QByteArray>
#include <QRect>

class VectorTile
{
public:
    VectorTile();
    ~VectorTile();

    void DeserializeMessage(QByteArray data);
    QRect boundingRect() const;

};
#endif // VECTORTILES_H
