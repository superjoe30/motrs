#include "Gameplay.h"

#include "ResourceManager.h"
#include "Input.h"
#include "Tile.h"
#include "Debug.h"
#include "os_config.h"

#include <cmath>

#include <iostream>

#ifndef RELEASE
const char * Gameplay::ResourceFilePath = "resources.dat";
#else
const char * Gameplay::ResourceFilePath = RESOURCE_DIR "/resources.dat";
#endif

const int Gameplay::WingDirectionsMap[] = {
    1 << Entity::East | 1 << Entity::South,
    1 << Entity::SouthEast | 1 << Entity::SouthWest,
    1 << Entity::South | 1 << Entity::West,
    1 << Entity::NorthEast | 1 << Entity::SouthEast,
    0,
    1 << Entity::SouthWest | 1 << Entity::NorthWest,
    1 << Entity::North | 1 << Entity::East,
    1 << Entity::NorthWest | 1 << Entity::NorthEast,
    1 << Entity::West | 1 << Entity::North,
};

Gameplay * Gameplay::s_inst = NULL;

Gameplay::Gameplay(SDL_Surface * screen, int fps) :
    m_good(true),
    m_screen(screen),
    m_fps(fps),
    m_interval(1000/fps), //frames per second -> miliseconds
    m_frameCount(0),
    m_screenX(0.0), m_screenY(0.0),
    m_universe(NULL),
    m_currentWorld(NULL),
    m_loadedMaps(),
    m_player(NULL)
{
    Debug::assert(s_inst == NULL, "only one Gameplay allowed");
    s_inst = this;
    m_universe = ResourceManager::loadUniverse(ResourceFilePath, "main.universe");
    if (!(m_universe != NULL && m_universe->isGood())) {
        m_good = false;
        return;
    }
    if (m_universe->worldCount() == 0) {
        std::cerr << "no worlds" << std::endl;
        m_good = false;
        return;
    }
    m_currentWorld = m_universe->startWorld();
    m_loadedMaps.insert(m_currentWorld->getMap());
    m_player = m_universe->player();
}

Gameplay::~Gameplay() {
    if (m_universe != NULL)
        delete m_universe;
    s_inst = NULL;
}

void Gameplay::mainLoop() {
    Uint32 next_time = 0;
    while (true) {
        //input
        if (!processEvents())
            return;

        //set up an interval
        Uint32 now = SDL_GetTicks();
        if (now >= next_time) {
            SDL_Flip(m_screen); //show the frame that is already drawn
            nextFrame(); //main gameplay loop
            updateDisplay(); //begin drawing next frame

            next_time = now + m_interval;
        }

        SDL_Delay(1); //give up the cpu
    }
}

bool Gameplay::isGood() {
    return m_good;
}

bool Gameplay::processEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_KEYDOWN:
                // Handle Alt+F4 for windows
                if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & KMOD_ALT))
                    return false;
                break;
            case SDL_QUIT:
                return false;
        }
    }
    Input::refresh();

    return true;
}


void Gameplay::nextFrame() {
    // iterate map set once
    std::vector<Map*> maps;
    maps.reserve(m_loadedMaps.size());
    for (std::set<Map*>::iterator iMap = m_loadedMaps.begin(); iMap != m_loadedMaps.end(); iMap++)
        maps.push_back(*iMap);

    // determin directional input
    int north = Input::state(Input::North) ? 1 : 0;
    int east = Input::state(Input::East) ? 1 : 0;
    int south = Input::state(Input::South) ? 1 : 0;
    int west = Input::state(Input::West) ? 1 : 0;
    int input_dx = east - west;
    int input_dy = south - north;
    Entity::Direction direction = (Entity::Direction)((input_dx + 1) + 3 * (input_dy + 1));
    if (direction == Entity::Center) {
        m_player->setMovementMode(Entity::Stand);
    } else {
        m_player->setMovementMode(Entity::Run);
        m_player->setOrientation(direction);
    }

    // calculate the desired location
    double speed = 3.0;
    double dx = speed * input_dx;
    double dy = speed * input_dy;
    double x = m_player->centerX() + dx, y = m_player->centerY() + dy;
    double radius = m_player->radius();
    int layer = m_player->layer();

    // resolve collisions
    std::vector<Map::TileAndLocation> tiles;
    for (unsigned int i = 0; i < maps.size(); i++)
        maps[i]->intersectingTiles(tiles, x, y, radius, layer);
    //  ask them where to go
    int hitDirections = 0;
    for (unsigned int i = 0; i < tiles.size(); i++)
        tiles[i].tile->resolveCollision(tiles[i].x, tiles[i].y, x, y, radius, hitDirections);
    if (hitDirections != 0) {
        // head on collisions
        Entity::Direction fatalDirection = (Entity::Direction)(8 - direction);
        if ((1 << fatalDirection) & hitDirections) {
            // stopped
            x = m_player->centerX();
            y = m_player->centerY();
        } else {
            // diagonal collisions
            int wingDirections = WingDirectionsMap[direction];
            if ((hitDirections & wingDirections) == wingDirections) {
                // stopped
                x = m_player->centerX();
                y = m_player->centerY();
            } else if ((hitDirections & wingDirections) != 0) {
                // diagonal push
                int oppositeWay = (wingDirections & ~hitDirections);
                switch (oppositeWay) {
                case 1 << Entity::NorthWest: dx =  speed; dy =  speed; break;
                case 1 << Entity::North:     dx =  0.0;   dy =  speed; break;
                case 1 << Entity::NorthEast: dx = -speed; dy =  speed; break;
                case 1 << Entity::West:      dx =  speed; dy =  0.0;   break;
                case 1 << Entity::Center: Debug::assert(false, "bad wingDirection computation"); break;
                case 1 << Entity::East:      dx = -speed; dy =  0.0;   break;
                case 1 << Entity::SouthWest: dx =  speed; dy = -speed; break;
                case 1 << Entity::South:     dx =  0.0;   dy = -speed; break;
                case 1 << Entity::SouthEast: dx = -speed; dy = -speed; break;
                default: Debug::assert(false, "bad wingDirection computation"); break;
                }
                x = m_player->centerX() + dx;
                y = m_player->centerY() + dy;
            }
        }
    }

    m_player->setCenter(x, y);

    // scroll the screen
    double marginNorth = m_player->centerY() - m_screenY;
    double marginEast = m_screenX + screenWidth() - (m_player->centerX() + m_player->radius());
    double marginSouth = m_screenY + screenHeight() - (m_player->centerY() + m_player->radius());
    double marginWest = m_player->centerX() - m_screenX;

    if (marginNorth < minMarginNorth())
        m_screenY -= minMarginNorth() - marginNorth;
    else if (marginSouth < minMarginSouth())
        m_screenY += minMarginSouth() - marginSouth;
    if (marginWest < minMarginWest())
        m_screenX -= minMarginWest() - marginWest;
    else if (marginEast < minMarginEast())
        m_screenX += minMarginEast() - marginEast;

    m_frameCount++;
}

void Gameplay::updateDisplay() {
    //generic background color
    SDL_FillRect(m_screen, NULL, SDL_MapRGB(m_screen->format, 0,0,0));

    //blit the map
    Map * map = m_currentWorld->getMap();
    for (int layer = 0; layer < map->layerCount(); layer++) {
        map->draw(m_screenX, m_screenY, screenWidth(), screenHeight(), layer);
        if (layer == m_player->layer())
            m_player->draw(m_screenX, m_screenY);
    }
}
