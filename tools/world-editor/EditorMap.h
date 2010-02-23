#ifndef EDITORMAP_H
#define EDITORMAP_H

#include "Map.h"
#include "EditorObject.h"
#include "EditorEntity.h"

#include <QString>
#include <QList>

// An EditorMap has a list of EditorObjects and EditorEntities that are tile-aligned.
// The game editor can use this class to edit maps that have movable EditorObjects and EditorEntities
// Call the compile() method to turn it into a real Map which will break
// the EditorObjects into tiles
class EditorMap : public Map
{
public: //variables
    typedef struct {
        EditorObject * object;
        int tileX; // tile index
        int tileY;
        int layer;
    } MapObject;

    typedef struct {
        // layer name
        QString name;
        // list of objects per layer. Layer 0 of the object == the layer it is on.
        // this means the object's layerCount + index < this editormap's layerCount
        // if an object has multiple layers, there is a pointer to the same object
        // in each layer list in which the object's layer overlaps
        QList<MapObject *> objects;
        // list of entities per layer
        QList<EditorEntity *> entities;
    } MapLayer;

public: //methods
    // default Map. empty dimensions and layers.
    EditorMap();
    ~EditorMap();

    // load a Map from a text format file
    static EditorMap * load(QString file);

    void setLeft(double value);
    void setTop(double value);
    void setWidth(double value);
    void setHeight(double value);
    double width();
    double height();

    void addLayer(QString name = "");
    void deleteLayer(int index);
    void swapLayer(int i, int j);
    void renameLayer(int index, QString newName);
    QString layerName(int index);
    inline int layerCount();

    // add / remove(negative amount) tiles
    void addTilesLeft(int amount);
    void addTilesRight(int amount);
    void addTilesTop(int amount);
    void addTilesBottom(int amount);

    // return the number of tiles
    inline int tileCountX();
    inline int tileCountY();

    // expose the layers structure, read only.
    inline const MapLayer * layer(int index) const;

    // add/remove an object/entity to this map. makes sure the data structure is intact.
    void addObject(MapObject * object);
    void removeObject(MapObject * object);

    void addEntity(EditorEntity * entity);
    void removeEntity(EditorEntity * entity);

private: //variables
    // we simply remember the dimensions because we don't actually have a tile
    // grid until we compile into a Map
    int m_tileCountX;
    int m_tileCountY;

    QList<MapLayer *> m_layers;
};

inline int EditorMap::tileCountX()
{
    return m_tileCountX;
}

inline int EditorMap::tileCountY()
{
    return m_tileCountY;
}

inline int EditorMap::layerCount()
{
    return m_layers.size();
}

inline const EditorMap::MapLayer * EditorMap::layer(int index) const
{
    return m_layers.at(index);
}

#endif
