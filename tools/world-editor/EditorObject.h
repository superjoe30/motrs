#ifndef EDITOROBJECT_H
#define EDITOROBJECT_H

#include "Tile.h"
#include "Array3.h"
#include "EditorGraphic.h"

#include "ResourceFile.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QList>

// An EditorObject defines which art makes up an object. it is a source form.
// The WorldView must make instances of these objects
class EditorObject
{
public: //variables
    typedef struct {
        int x;
        int y;
        int width;
        int height;
        QString graphicName; // file name in resources/graphics
        EditorGraphic * graphic;
        int layer;

        QRectF geometry() { return QRectF(QPointF(x,y), QSizeF(width, height)); }
    } ObjectGraphic;

public: //methods
    // default Object. arbitrary defaults.
    EditorObject();
    ~EditorObject();

    // load / save from text file format
    static EditorObject * load(QString file);
    void save(QString file);

    // messing with layers
    void renameLayer(int index, QString newName);
    QString layerName(int index);

    void addLayer(QString name = "");
    void deleteLayer(int index);
    void swapLayer(int i, int j);

    // add / remove(negative amount) tiles
    void addTilesLeft(int amount);
    void addTilesRight(int amount);
    void addTilesTop(int amount);
    void addTilesBottom(int amount);

    // dimensions of the object in tiles
    int tileCountX();
    int tileCountY();
    int layerCount();

    // messing with tiles
    Tile::Shape shape(int x, int y, int z);
    void setShape(int x, int y, int z, Tile::Shape);

    Tile::SurfaceType surfaceType(int x, int y, int z);
    void setSurfaceType(int x, int y, int z, Tile::SurfaceType);

    // you are free to mess with the graphics list directly
    // maps layer number to a list of graphics
    QList<QList<ObjectGraphic *> *> * graphics() { return &m_graphics; }

    // properties
    QString name() { return m_name; }
    void setName(QString name) { m_name = name; }

    QString description() { return m_description; }
    void setDescription(QString description) { m_description = description; }

    QString resourceName() { return m_resourceName; }

    // compile the object into graphics for each tile and put them in the
    // resources file. Use compiledGraphicAt to get the graphic ids of each tile.
    // returns success
    bool build(ResourceFile & resources);

    // get the id of the graphic at
    QString compiledGraphicAt(int x, int y, int z);

private: //variables
    QStringList m_layerNames;

    // use a full array of tiles and then later we can compile it into a map
    Array3<Tile::SurfaceType> * m_surfaceTypes;
    Array3<Tile::Shape> * m_shapes;

    QString m_name;
    QString m_description;

    // the resource identifier string (thing that ends in .object)
    QString m_resourceName;

    // a list of graphics per layer
    QList<QList<ObjectGraphic *> *> m_graphics;

    Array3<QString> * m_compiledGraphics;

private: //methods
    void render(QPainter & p, int layerIndex, int frame, int fps);
};

#endif // EDITOROBJECT_H
