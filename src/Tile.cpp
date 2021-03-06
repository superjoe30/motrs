#include "Tile.h"

#include "Utils.h"
#include "Gameplay.h"
#include "ResourceManager.h"
#include "Physics.h"

#include <cmath>

const double Tile::size = 16.0;
const int Tile::sizeInt = (int)Tile::size;

Tile * Tile::loadFromMemory(const char** cursor)
{
    Tile * out = new Tile();

    out->m_shape = (Shape)Utils::readInt(cursor);
    out->m_surfaceType = (SurfaceType)Utils::readInt(cursor);
    std::string graphicId = Utils::readString(cursor);
    out->m_graphic = ResourceManager::getGraphic(graphicId);
    if (out->m_graphic == NULL) {
        std::cerr << "Unable to load graphic for tile" << std::endl;
        delete out;
        return NULL;
    }

    return out;
}

Tile::Tile() :
    m_shape(tsSolidWall),
    m_surfaceType(stNormal),
    m_graphic(NULL)
{
}

Tile::Tile(const Tile & tile) :
    m_shape(tile.m_shape),
    m_surfaceType(tile.m_surfaceType),
    m_graphic(tile.m_graphic)
{
}

Tile::~Tile() {
}

void Tile::draw(int screenX, int screenY) {
    if (m_graphic == NULL)
        return;
    m_graphic->draw(Gameplay::instance()->screen(), screenX, screenY);
}

bool Tile::hasMinPresence(PhysicalPresence minPresence) {
    switch (m_shape) {
    case tsSolidWall: return minPresence <= ppWall;
    case tsSolidFloor: return minPresence <= ppFloor;
    case tsSolidHole: return minPresence <= ppHole;
    case tsDiagFloorWallNW: return minPresence <= ppWall;
    case tsDiagFloorWallNE: return minPresence <= ppWall;
    case tsDiagFloorWallSE: return minPresence <= ppWall;
    case tsDiagFloorWallSW: return minPresence <= ppWall;
    case tsFloorRailN: return minPresence <= ppRail;
    case tsFloorRailE: return minPresence <= ppRail;
    case tsFloorRailS: return minPresence <= ppRail;
    case tsFloorRailW: return minPresence <= ppRail;
    default: assert(false); return false;
    }
}

void Tile::resolveCircleCollision(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    switch (m_shape) {
    case tsSolidWall:
        resolveCircleOnSquare(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
        break;
    case tsSolidFloor:
        // nothing
        break;
    case tsSolidHole:
        // TODO: tile shapes
        assert(false);
        break;
    // floor + wall diagonals. tsDiag(pp1)(pp2)(where-pp2-is)
    case tsDiagFloorWallNW:
        resolveCircleOnTriangleNW(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
        break;
    case tsDiagFloorWallNE:
        resolveCircleOnTriangleNE(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
        break;
    case tsDiagFloorWallSE:
        resolveCircleOnTriangleSE(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
        break;
    case tsDiagFloorWallSW:
        resolveCircleOnTriangleSW(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
        break;
    // floor + rail orientations
    case tsFloorRailN:
        assert(false);
        break;
    case tsFloorRailE:
        {
            if (!(tileY < objectCenterY && objectCenterY < tileY + Tile::size))
                break;
            double tileRight = tileX + Tile::size;
            double leftThreshold = tileRight - objectRadius;
            if (!(leftThreshold < objectCenterX && objectCenterX < tileX + Tile::size))
                break;
            objectCenterX -= 3.0;
            if (objectCenterX < leftThreshold)
                objectCenterX = leftThreshold;
        }
        break;
    case tsFloorRailS:
        // TODO: tile shapes
        assert(false);
        break;
    case tsFloorRailW:
        {
            if (!(tileY < objectCenterY && objectCenterY < tileY + Tile::size))
                break;
            double rightThreshold = tileX + objectRadius;
            if (!(tileX < objectCenterX && objectCenterX < rightThreshold))
                break;
                objectCenterX += 3.0;
            if (objectCenterX > rightThreshold)
                objectCenterX = rightThreshold;
        }
        break;
    default:
        assert(false);
        break;
    }
}

void Tile::resolveCircleOnSquare(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    double dx, dy;
    Physics::squareAndCircle(tileX + Tile::size * 0.5, tileY + Tile::size * 0.5, Tile::size * 0.5, objectCenterX, objectCenterY, objectRadius, dx, dy);
    objectCenterX += dx;
    objectCenterY += dy;
}

void Tile::resolveCircleOnTriangleNW(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    // thing = rectilinear distance of object's center from the right-angle vertex
    double thing = objectCenterX - tileX + objectCenterY - tileY;
    bool pastTheBar = thing > Tile::size;
    if (!pastTheBar) {
        // from here, it's the same as a square.
        resolveCircleOnSquare(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
    } else {
        // zones:
        // +-, 0
        // |/ \    (don't warn me about multi-line comments)
        // ' 1
        //  \      (don't warn me about multi-line comments)
        // 2
        double thing1 = -(objectCenterX - (tileX + Tile::size)) + (objectCenterY - tileY);
        double thing2 = -(objectCenterX - tileX) + (objectCenterY - (tileY + Tile::size));
        int zone = (thing1 > 0.0) + (thing2 > 0.0);
        switch (zone) {
        case 0:
            resolveCircleOnPoint(tileX + Tile::size, tileY, objectCenterX, objectCenterY, objectRadius);
            break;
        case 1:
            {
                double rectDistance = thing - Tile::size;
                double rectRadius = objectRadius / Utils::RadHalf;
                double rectOverlap = rectRadius - rectDistance;
                if (rectOverlap > 0.0) {
                    objectCenterX += rectOverlap * 0.5;
                    objectCenterY += rectOverlap * 0.5;
                }
            }
            break;
        case 2:
            resolveCircleOnPoint(tileX, tileY + Tile::size, objectCenterX, objectCenterY, objectRadius);
            break;
        }
    }
}

void Tile::resolveCircleOnTriangleNE(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    // thing = rectilinear distance of object's center from the right-angle vertex
    double thing = -(objectCenterX - (tileX + Tile::size)) + (objectCenterY - tileY);
    bool pastTheBar = thing > Tile::size;
    if (!pastTheBar) {
        // from here, it's the same as a square.
        resolveCircleOnSquare(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
    } else {
        // regions:
        // 0 .-+
        //  / \|
        //   1 '
        //    /
        //     2
        double thing1 = (objectCenterX - tileX) + (objectCenterY - tileY);
        double thing2 = (objectCenterX - (tileX + Tile::size)) + (objectCenterY - (tileY + Tile::size));
        int zone = (thing1 > 0.0) + (thing2 > 0.0);
        switch (zone) {
        case 0:
            resolveCircleOnPoint(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
            break;
        case 1:
            {
                double rectDistance = thing - Tile::size;
                double rectRadius = objectRadius / Utils::RadHalf;
                double rectOverlap = rectRadius - rectDistance;
                if (rectOverlap > 0.0) {
                    objectCenterX -= rectOverlap * 0.5;
                    objectCenterY += rectOverlap * 0.5;
                }
            }
            break;
        case 2:
            resolveCircleOnPoint(tileX + Tile::size, tileY + Tile::size, objectCenterX, objectCenterY, objectRadius);
            break;
        }
    }
}

void Tile::resolveCircleOnTriangleSE(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    // thing = rectilinear distance of object's center from the right-angle vertex
    double thing = -(objectCenterX - (tileX + Tile::size)) - (objectCenterY - (tileY + Tile::size));
    bool pastTheBar = thing > Tile::size;
    if (!pastTheBar) {
        // from here, it's the same as a square.
        resolveCircleOnSquare(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
    } else {
        // regions:
        //     0
        //    \    (don't warn me about multi-line comments)
        //   1 .
        //  \ /|
        // 2 `-+
        double thing1 = -(objectCenterX - (tileX + Tile::size)) + (objectCenterY - tileY);
        double thing2 = -(objectCenterX - tileX) + (objectCenterY - (tileY + Tile::size));
        int zone = (thing1 > 0.0) + (thing2 > 0.0);
        switch (zone) {
        case 0:
            resolveCircleOnPoint(tileX + Tile::size, tileY, objectCenterX, objectCenterY, objectRadius);
            break;
        case 1:
            {
                double rectDistance = thing - Tile::size;
                double rectRadius = objectRadius / Utils::RadHalf;
                double rectOverlap = rectRadius - rectDistance;
                if (rectOverlap > 0.0) {
                    objectCenterX -= rectOverlap * 0.5;
                    objectCenterY -= rectOverlap * 0.5;
                }
            }
            break;
        case 2:
            resolveCircleOnPoint(tileX, tileY + Tile::size, objectCenterX, objectCenterY, objectRadius);
            break;
        }
    }
}

void Tile::resolveCircleOnTriangleSW(double tileX, double tileY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    // thing = rectilinear distance of object's center from the right-angle vertex
    double thing = (objectCenterX - tileX) - (objectCenterY - (tileY + Tile::size));
    bool pastTheBar = thing > Tile::size;
    if (!pastTheBar) {
        // from here, it's the same as a square.
        resolveCircleOnSquare(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
    } else {
        // regions:
        // 0
        //  /
        // . 1
        // |\ /
        // +-` 2
        double thing1 = (objectCenterX - tileX) + (objectCenterY - tileY);
        double thing2 = (objectCenterX - (tileX + Tile::size)) + (objectCenterY - (tileY + Tile::size));
        int zone = (thing1 > 0.0) + (thing2 > 0.0);
        switch (zone) {
        case 0:
            resolveCircleOnPoint(tileX, tileY, objectCenterX, objectCenterY, objectRadius);
            break;
        case 1:
            {
                double rectDistance = thing - Tile::size;
                double rectRadius = objectRadius / Utils::RadHalf;
                double rectOverlap = rectRadius - rectDistance;
                if (rectOverlap > 0.0) {
                    objectCenterX += rectOverlap * 0.5;
                    objectCenterY -= rectOverlap * 0.5;
                }
            }
            break;
        case 2:
            resolveCircleOnPoint(tileX + Tile::size, tileY + Tile::size, objectCenterX, objectCenterY, objectRadius);
            break;
        }
    }
}

void Tile::resolveCircleOnPoint(double pointX, double pointY, double & objectCenterX, double & objectCenterY, double objectRadius) {
    double distance = Utils::distance(objectCenterX, objectCenterY, pointX, pointY);
    if (distance < objectRadius) {
        double normX = (objectCenterX - pointX) / distance;
        double normY = (objectCenterY - pointY) / distance;
        objectCenterX = pointX + normX * objectRadius;
        objectCenterY = pointY + normY * objectRadius;
    }
}

Tile * Tile::nullTile() {
    static Tile * s_nullTile = new Tile();
    return s_nullTile;
}
