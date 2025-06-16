#include "StudentWorld.h"
#include "Actor.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <iomanip> //for setGameScreen

GameWorld* createStudentWorld(std::string assetDir) {
    return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(std::string assetDir)
    : GameWorld(assetDir),
    barrelsRemaining(0), IceM(nullptr),
    m_ice(64, std::vector<Ice*>(64, nullptr))
{
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init() { //generates level's oil field, fill with ice, boulders etc.
    //A. allocate data to keep track of world
    //B. Construct oil field
    //C. Allocate and insert Iceman at a valid location
    int m_level = getLevel();
    for (int x = 0; x < 60; ++x) //INITIALIZES Oil FIELD
        for (int y = 0; y < 64; ++y)
            m_ice[y][x] = nullptr;
    int num_B = std::min(m_level / 2 + 2, 9); //boulders spawn before ICE
    for (int i = 0; i < num_B; i++) { //spawns in boulders
        int x, y;
        do { //randomly selects a point and if it passes conditions spawns
            x = std::rand() % 58;           //from [0-57]
            y = std::rand() % (53 + 1 - 20) + 20;      //from [20-53] format: rand() % (max_number + 1 - minimum_number) + minimum_number
        } while (!outRadius(x, y) || outShaft(x, y)); //while out of radius another object and shaft and in range of random gen
        Boulder* b = new Boulder(x, y, this);
        b->setVisible(true);
        AddActor(b);
    }


    for (int x = 0; x < 60; ++x) { //fill rows 0-59 with ice objects except for the vertical mine shaft
        for (int y = 0; y < 60; ++y) {
            if (x >= 30 && x <= 33 && y >= 4) ////mine shaft is [30-33][4-59] I assume rows mean random gen, columns is a straight line down
                m_ice[y][x] = nullptr;
            else if (BoulderThere(x, y)) {
                m_ice[y][x] = nullptr;
            }
            else {
                m_ice[y][x] = new Ice(x, y);
            }
        }
    }
    // create Iceman
    IceM = new IceMan(30, 60, this);
    AddActor(IceM);

    int proTickStun = max(50, 100 - (10 * static_cast<int>(getLevel())));
    int ticksToWaitBetweenMoves = max(1, 3 - static_cast<int>(getLevel() / 4));
    int proTickWait = max(1, 3 - (static_cast<int>(getLevel()) / 4));
    currentNum = 0;
    AddActor(new Protester(57, 60, IID_PROTESTER, proTickStun, ticksToWaitBetweenMoves, this));
    currentNum++;

    //Distribute the following game objects throughout the field
    // B boulders int B = min(current_level_number / 2+2, 9) must be generateed from[0-60][20-56] Ice cannot generate overlapping
    // G Gold nuggets int G = max(5-current_level_number / 2, 2) must be generateed from[0-60][0-65] start invisble
    // L barrels of oil int L = min(2 + current_level_number, 21) must be generateed from[0-60][0-65] starts invisble
    int num_G = std::max(5 - m_level / 2, 2);
    int num_L = std::min(2 + m_level, 21);
    barrelsRemaining = num_L;

    for (int i = 0; i < num_G; i++) { //randomly places gold
        int x, y;
        do {
            x = std::rand() % 58;           //from [0-57]
            y = std::rand() % 58;           //from [0-57]
        } while (!outRadius(x, y) || outShaft(x, y)); //while out of radius another object and shaft and in range of random gen
        Gold* g = new Gold(x, y, false, this); //places gold
        g->setVisible(false); //debug true
        AddActor(g);
    }
    for (int i = 0; i < num_L; i++) {
        int x, y;
        do {
            x = std::rand() % 58;           //from [0-57]
            y = std::rand() % 58;           // from[0 - 57]
        } while (!outRadius(x, y) || outShaft(x, y)); //while out of radius another object and shaft and in range of random gen
        Oil* o = new Oil(x, y, this); //places oil
        o->setVisible(false); //debug true
        AddActor(o);
    }

    setDisplayText();
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() { //called roguhly every 10-20 seconds. Each time called it must run a single tick of the game
    //1. Update status text at top of screen (player current score, remaining bonus score, level, lives
        //2. Ask all actors in the game world to do something
        //2A. If an actor causes lives to <= 0 return GWSTATUS_PLAYER_DIED;
        //2B. If all barrels collected play SOUND_FINISHED_LEVEL and then return GWSTATUS_FINISHED_LEVEL
        //3. Delete any actors that need  to be removed during the tick, clear them from ActMemory. Examples:
        //3 1. A protestor who's moved to the upper right hand corner afer being sufficiently annoyed to leave the oil field
        //3 2. A bolder has fallen down and disintegrated upon hitting another boulder or ice
        //3 3. A gold nugget has been picked up and should be removed
        //3 4. A water pool that has dried out after a certain amount of time
        //3 5. A squirt from IceMan's gun has traveled its max distance
        //3 6. etc..
        // Make some sort of return time like ClearMe, to tell game it should be removed
        // Need some sort of behavior call like doSomething()

    //#TODO: Spawning the following during runtime
     //Add actors during each tick
     //If current num of protestors is < int P = min(15,2 + current_level_number *1.5) and
     //If T num of tick have passed since last protestor was added T= max(25,200 - current_level_number);
     // Add a protestor at x = 60 y = 60 with a random chance of being hardcore
     //int probabilityOfHardcore = min(90, current_level_number * 10 + 30)
     // 1 / G chance of Goodie being added G = current_level_number * 30 + 290
     //If passed 1/G calc again 1/5 chance for new Sonar Kit and 4/5 chance new water kit
     //If Sonar Kit add sonar kit at x = 0 y = 60
     //If water kit add kit if there  is a 4x4 space free of Ice should create an array NoIce of NoIce locations

    int m_level = getLevel();

    setDisplayText();
    int num_T = max(100, 30 - 10 * m_level); //number of Ticks T a sonar kit will exist
    int G = m_level * 30 + 290;
    if (rand() % G == 0) {
        // now 1/5 chance sonar, 4/5 chance water pool
        if (rand() % 5 == 0) { //debug if % 2
            AddActor(new SonarKit(0, 60, num_T, this));  // add sonar kit at 0,60 initialize with num_T
        }
        else {
            auto& OilField = getMap();
            int rows = OilField.size(); //updates according to current oilfield        
            int cols = OilField[0].size();   //x always first subscript of y because of how initialized

            std::vector<Point> freeSpots; //search through oil field for free spots for water puddle
            for (int y = 0; y <= rows - 4; ++y) {
                for (int x = 0; x <= cols - 4; ++x) {
                    bool spot_free = true;
                    for (int dy = 0; dy < 4 && spot_free; ++dy) {
                        for (int dx = 0; dx < 4; ++dx) {
                            if (OilField[y + dy][x + dx] != nullptr) {
                                spot_free = false;
                                break;
                            }
                        }
                    }
                    if (spot_free) //if spots available add a new water puddle
                        freeSpots.emplace_back(x, y);
                }
            }
            if (!freeSpots.empty()) { // If there is a free spot pick one at random
                const Point& p = freeSpots[rand() % freeSpots.size()];
                AddActor(new Waterpool(p.x, p.y, num_T, this)); //initialize new waterpool actor
            }
        }
    }
    int spawnP = --tickBetween; //start of last shared Jose code

    if (spawnP == 0) {
        if (currentNum < pNum) {//check if current pros on map reached max pros on map
            if (diceShuff <= probabilityOfHardcore) {
                AddActor(new HardcoreProtester(57, 60, proTickStun, ticksToWaitBetweenMoves, this));//spawn hardcore protester
                cout << "a wild hardcore protester appeared!" << endl;
            }
            else {
                AddActor(new Protester(57, 60, IID_PROTESTER, proTickStun, ticksToWaitBetweenMoves, this));//spawn protester
                cout << "a wild protester appeared!" << endl;
            }
        }
        tickBetween = max(25, 200 - static_cast<int>(getLevel())); // reset tick count
        diceShuff = rand() % 101;//get another random num
    }
    for (Actor* a : m_actors) {
        if (a->doesExists())
            a->doSomething();



        if (!IceM->doesExists()) {
            decLives();
            return GWSTATUS_PLAYER_DIED; //fix for dieing bug

        }
        if (barrelsRemaining == 0) {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }

    // kills protesters after some action
    for (auto itr = m_actors.begin(); itr != m_actors.end(); ) {
        Actor* actor = *itr;

        // Debuggy: Print who you're checking
        // std::cout << "Checking actor at (" << actor->getX() << ", " << actor->getY() << ") with ID: " << actor->getID() << std::endl;

        // check if a protester is at the exit point and ready to leave
        if ((actor->getID() == IID_PROTESTER || actor->getID() == IID_HARD_CORE_PROTESTER) &&
            actor->getX() == 58 && actor->getY() == 60) {

            std::cout << "Deleting protester at exit.\n";
            delete actor;
            itr = m_actors.erase(itr);
            currentNum--;
        }
        else if (!actor->doesExists()) {
            delete actor;
            itr = m_actors.erase(itr);
        }
        else {
            ++itr;
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
    //delete actors
    for (Actor* a : m_actors)
        delete a;
    m_actors.clear();
    IceM = nullptr; //dangling pointer
    //delete Whole Oil Field
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 60; ++x) {
            delete m_ice[y][x];
            m_ice[y][x] = nullptr; //clear dangling pointer
        }
    }
}

std::vector<std::vector<Ice*>>& StudentWorld::getMap() { //changed to 2D vector for Jose's code
    return m_ice;
}

std::vector<Actor*>& StudentWorld::getCharacterList() {
    return m_actors;
}
IceMan* StudentWorld::getIceMan() const {
    return IceM;
}

void StudentWorld::AddActor(Actor* a) {
    m_actors.push_back(a);
}


bool StudentWorld::outRadius(int x, int y) const {
    for (Actor* a : m_actors) {
        double dx = a->getX() - x;
        double dy = a->getY() - y;
        if (std::sqrt(dx * dx + dy * dy) <= 6.0)
            return false;
    }
    return true;
}

string StudentWorld::setGameScreen(int lev,
    int liv,
    int hel,
    int squirt,
    int gold,
    int barrel,
    int son,
    int score) const {
    std::stringstream ss;
    ss << "Lvl: " << setw(2) << setfill(' ') << lev //setw and setfill iomanipulators, basically just give character spaces that can be filled
        << "  Lives: " << setw(1) << setfill(' ') << liv
        << "  Hlth: " << setw(3) << setfill(' ') << hel << '%'
        << "  Wtr: " << setw(2) << setfill(' ') << squirt
        << "  Gld: " << setw(2) << setfill(' ') << gold
        << "  Oil Left: " << setw(2) << setfill(' ') << barrel
        << "  Sonar: " << setw(2) << setfill(' ') << son
        << "  Scr: " << setw(6) << setfill('0') << score;
    return ss.str();
}

void StudentWorld::setDisplayText() {
    int lvl = getLevel();
    int lives = getLives();
    double health = IceM->getHealth() * 10;
    int squirts = IceM->getSquirt();
    int gold = IceM->getGold();
    barrelsRemaining = barrelsRemaining;
    int sonar = IceM->getSonarCount();
    int score = getScore();

    std::string s = setGameScreen(lvl, lives,
        health, squirts,
        gold, barrelsRemaining,
        sonar, score);
    setGameStatText(s);
}

bool StudentWorld::blockedByRocks(int x, int y) {
    for (const Point& p : rockPos) {//search thru all rocks on map
        double radius = sqrt(pow(x - p.x, 2) + pow(y - p.y, 2));
        if (radius <= 4) {//if rock is within a radius of 4 to actor set true
            return true;
        }
    }
    return false;
}

bool StudentWorld::blockedByIce(int x, int y) {
    if (y == 61 || x == -1 || x == 57 || y == -1) {
        return true; // if is blocked by the wall and can't go outside of the border
    }
    for (int i = y; i < y + 4; i++) {
        for (int j = x; j < x + 4; j++) {
            if (m_ice[i][j] != nullptr) {//check a 4x4 square if has no ice
                return true;
            }
        }
    }
    return false;

}

bool StudentWorld::blockedbyRocksOrIce(int x, int y, Actor::Direction d) {
    if (blockedByIce(x, y) || checkSpot("Boulder", x, y)) {
        return true;//check both in 1 func, to make it easier
    }

    return false;
}

void StudentWorld::dropGold(int x, int y) {
    if (IceM->getGold() > 0) {
        //set gold to pick up able by protesters
        IceM->setGold(-1);
        getCharacterList().push_back(new Gold(IceM->getX(), IceM->getY(), true, this));//rest handled in constructor
        goldPos.push_back((Point(IceM->getX(), IceM->getY())));//add gold to dropped gold pos to search for later
    }
}

bool StudentWorld::pickUpGold(int x, int y) {
    for (const Point& p : goldPos) {//search all dropped gold
        double radius = sqrt(pow(x - p.x, 2) + pow(y - p.y, 2));
        if (radius <= 4) {//if dropped gold within rad of 4
            for (Actor* a : m_actors) {
                if (a->getID() == IID_GOLD && a->getX() == p.x && a->getY() == p.y) {
                    a->setExists(false);//actor(protester) picks up gold, gold dies
                    return true;
                }
            }
        }

    }
    return false;
}

bool StudentWorld::checkSpot(string actorType, int x, int y) {
    for (Actor* act : m_actors) {
        if (actorType == "Boulder") {
            if (act->getID() == IID_BOULDER && abs(act->getX() - x) < 4 && abs(act->getY() - y) < 4) {
                //if boulder is in rad of 4 return true
                return true;
            }
        }
    }
    return false;
}

string StudentWorld::getLeadingPathDistance(int x, int y) {
    string direction = "right";//starting dir

    //fine the smallest value in array
    int smallest = (leavingPath[y][x + 1]);
    if ((leavingPath[y + 1][x]) < smallest) {
        direction = "up";
        smallest = (leavingPath[y + 1][x]);
    }
    if (leavingPath[y - 1][x] < smallest) {
        direction = "down";
        smallest = (leavingPath[y - 1][x]);
    }
    if (leavingPath[y][x - 1] < smallest) {
        direction = "left";
        smallest = (leavingPath[y][x - 1]);
    }
    return direction;
}

void StudentWorld::findPath(int x, int y, int objx, int objy) {
    // set up queue and pair
    queue<pair<pair<int, int>, int>> q;
    pair <pair<int, int>, int> temp;

    temp = make_pair(make_pair(x, y), 0);

    int distance = 0;

    q.push(temp);
    int count = 0;

    // set all array to not taken
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            leavingPath[i][j] = 1000;
        }
    }

    leavingPath[y][x] = 0; // ending place

    while (!q.empty()) {
        if (q.front().first.first >= 0) {//if current node is on left
            if (q.front().first.first == 0) {
                leavingPath[q.front().first.second][q.front().first.first] = q.front().second;
            }
            else if (m_ice.at(q.front().first.second).at(q.front().first.first - 1) == nullptr && !checkSpot("Boulder", objx - 1, objy) && leavingPath[q.front().first.second][q.front().first.first - 1] == 1000) {
                //if no ice at point, no boulders, and point is not taken
                bool addtopath = true;
                for (int i = 0; i < 4; i++) {
                    if (m_ice.at(q.front().first.second + i).at(q.front().first.first - 1) != nullptr) {
                        addtopath = false;
                        //check 4x4 square if ice
                        break;
                    }
                }
                if (addtopath) {
                    //add node to pair to make shortest path
                    leavingPath[q.front().first.second][q.front().first.first - 1] = q.front().second + 1;
                    pair<pair<int, int>, int> a = make_pair(make_pair(q.front().first.first - 1, q.front().first.second), q.front().second + 1);
                    q.push(a);
                }
            }
        }
        //checks other nodes in pair with same conditions
        if (q.front().first.second >= 0) {//if current node is on left
            if (q.front().first.second == 0) {
                leavingPath[q.front().first.second][q.front().first.first] = q.front().second;
            }
            else if (m_ice.at(q.front().first.second - 1).at(q.front().first.first) == nullptr && !checkSpot("Boulder", objx, objy - 1) && leavingPath[q.front().first.second - 1][q.front().first.first] == 1000) {
                bool addtopath = true;
                for (int i = 0; i < 4; i++) {
                    if (m_ice.at(q.front().first.second - 1).at(q.front().first.first + i) != nullptr) {
                        addtopath = false;
                        break;
                    }

                }
                if (addtopath) {
                    leavingPath[q.front().first.second - 1][q.front().first.first] = q.front().second + 1;
                    pair<pair<int, int>, int> a = make_pair(make_pair(q.front().first.first, q.front().first.second - 1), q.front().second + 1);
                    q.push(a);
                }
            }
        }
        if (q.front().first.first <= 57) {
            if (q.front().first.first == 57) {//if current node is on right
                leavingPath[q.front().first.second][q.front().first.first] = q.front().second;
            }
            else if (m_ice.at(q.front().first.second).at(q.front().first.first + 4) == nullptr && !checkSpot("Boulder", objx - 1, objy) && leavingPath[q.front().first.second][q.front().first.first + 1] == 1000) {
                //cout << "I" << endl;
                bool addtopath = true;
                for (int i = 0; i < 4; i++) {
                    if (m_ice.at(q.front().first.second + i).at(q.front().first.first + 4) != nullptr) {
                        addtopath = false;
                        break;
                    }

                }
                if (addtopath) {

                    leavingPath[q.front().first.second][q.front().first.first + 1] = q.front().second + 1;
                    pair<pair<int, int>, int> a = make_pair(make_pair(q.front().first.first + 1, q.front().first.second), q.front().second + 1);
                    q.push(a);
                }
            }
        }

        if (q.front().first.second <= 57) {
            if (q.front().first.second == 57) {//if current node is on right
                leavingPath[q.front().first.second][q.front().first.first] = q.front().second;
            }
            else if (q.front().first.second == 59 && leavingPath[q.front().first.second][q.front().first.first] == 1000) {

            }
            else if (m_ice.at(q.front().first.second + 4).at(q.front().first.first) == nullptr && !checkSpot("Boulder", objx - 1, objy) && leavingPath[q.front().first.second + 1][q.front().first.first] == 1000) {
                //cout << "I" << endl;
                bool addtopath = true;
                for (int i = 0; i < 4; i++) {
                    if (m_ice.at(q.front().first.second + 4).at(q.front().first.first + i) != nullptr) {
                        addtopath = false;
                        break;
                    }

                }
                if (addtopath) {

                    leavingPath[q.front().first.second + 1][q.front().first.first] = q.front().second + 1;
                    pair<pair<int, int>, int> a = make_pair(make_pair(q.front().first.first, q.front().first.second + 1), q.front().second + 1);
                    q.push(a);
                }
            }
        }

        if (q.front().first.first == objx && q.front().first.second == objy) {
            break;
        }
        q.pop();

    }

}

bool StudentWorld::useSpray(int x, int y) {
    if (IceM->getSquirt() > 0) {//check if iceman has squirt
        getCharacterList().push_back(new Squirt(x, y, IceM->getDirection(), this));//spawn squirt
        playSound(SOUND_PLAYER_SQUIRT);
        IceM->setWater(-1);//decrement icemans inventory
        for (Actor* p : m_actors) {
            if ((p->getID() == IID_PROTESTER || p->getID() == IID_HARD_CORE_PROTESTER) && getRadius(x, p->getX(), y, p->getY()) <= 6) {//if hpro or pro in rad of 6 deal 2 damage
                if (p->doesExists()) {
                    p->setHitpoints(-2);
                    cout << "HIT POINTS: " << p->getHitpoints() << endl;
                    playSound(SOUND_PROTESTER_ANNOYED);
                    return true;
                }
            }
        }
    }
    return false;
}

bool StudentWorld::checkFacingDirection(int x, int y, Actor::Direction d) {
    // Check if the player is in a specific direction from given coordinates (x, y)
    if (IceM->getX() < x && d == Actor::Direction::left) {
        return true;
    }
    else if (IceM->getY() < y && d == Actor::Direction::down) {
        return true;
    }
    else if (IceM->getY() > y && d == Actor::Direction::up) {
        return true;
    }
    else if (IceM->getX() > x && d == Actor::Direction::right) {
        return true;
    }
    else {
        return false;
    }
    //true if the player is in that direction
}

