#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <vector>
#include <string>
#include <list>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//base classs Ice derived from besides graph object
//field spawning boulders

// #TODO:
// 


//#BUGS
//Protestors just walk to the right offscreen

//#KNOWN ISSUES
// objects can be picked up through Ice, this is because of the given logic that they're picked up when within 3 units. This seems to be in the example too
// How far away from IceMan when invisble objects made visible? Not Told.

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();

    // core game loop methods
    virtual int init() override;
    virtual int move() override;
    virtual void cleanUp() override;

    // accessors
    std::vector<std::vector<Ice*>>& getMap();
    std::vector<Actor*>& getCharacterList();
    IceMan* getIceMan() const;
    //Protester* getProtester();
    // Jose, write here if you want anymore accessors
    //getWorld()->useSpray(getX(), getY());
    // getWorld()->useSonar(getX(), getY());
    bool BoulderThere(int x, int y) const { //checks if bould there
        for (Actor* a : m_actors) {
            if (a->getID() == IID_BOULDER) {
                int bx = a->getX();
                int by = a->getY();
                if (x >= bx && x <= bx + 3 && y >= by && y <= by + 3)
                    return true;
            }
        }
        return false;
    }

    // actor management
    void AddActor(Actor* a);
    bool outRadius(int x, int y) const;
    void pickUpBarrel() {
        if (barrelsRemaining > 0) {
            barrelsRemaining--;
        }
    }
    /*void pickUpGold() {
        IceM->setGold(1);
    }*/
    bool pickUpGold(int x, int y);

    // display helpers
    std::string setGameScreen(int lev,
        int liv,
        int hel,
        int squirt,
        int gold,
        int barrel,
        int son,
        int score) const;
    void setDisplayText();

    inline bool outShaft(int x, int y) { //helper so random gen doesn't spawn in shaft
        return (
            (x <= 33 && x + 3 >= 30) &&
            (y <= 59 && y + 3 >= 4)); //offset by 3 for sprite size
    }

    bool checkSpot(string actorType, int x, int y);

    string getLeadingPathDistance(int x, int y);

    void findPath(int x, int y, int objx, int objy);

    void clearIceAt(int x, int y);

    bool checkFacingDirection(int x, int y, Actor::Direction d);

    bool blockedByIce(int x, int y);
    bool blockedbyRocksOrIce(int x, int y, Actor::Direction d);//not done
    bool blockedByRocks(int x, int y);


    void dropGold(int x, int y);

    bool useSpray(int x, int y);

    double getRadius(int x1, int x2, int y2, int y1) { return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)); }

private:
    int barrelsRemaining;
    IceMan* IceM;
    //Protester* Pross;


    int diceShuff = rand() % 101;
    int tickBetween = max(25, 200 - static_cast<int>(getLevel()));
    int pNum = min(10, static_cast<int>(2 + getLevel() * 1.5));
    int proTickStun = max(50, 100 - (10 * static_cast<int>(getLevel())));
    int probabilityOfHardcore = min(90, static_cast<int>(getLevel()) * 10 + 30);
    int ticksSonarWater = max(100, (300 - 10) * static_cast<int>(getLevel()));
    int ticksToWaitBetweenMoves = max(1, 3 - static_cast<int>(getLevel() / 4));
    int currentNum = 0;

    int leavingPath[64][64];

    int pathToIceman[64][64];

    struct Point {
        int x;
        int y;
        Point(int x, int y) : x(x), y(y) {}
    };
    list<Point> goldPos;
    list<Point> rockPos;


    std::vector<Actor*> m_actors;
    std::vector<std::vector<Ice*>> m_ice; // 64×60 vector of pointers to game objects
};

#endif // STUDENTWORLD_H_