#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>
#include <vector>
#include <algorithm>

bool Actor::outOfField(int x, int y, Actor::Direction d) {
    switch (d) {
    case left:  return x <= 0;
    case right: return x >= 57; //fixed, IceMan occupies 4 x 4 block, id starts from first 
    case up:    return y >= 60;
    case down:  return y <= 0;
    }
    return true;
}




void Boulder::doSomething() {
    StudentWorld* world = getWorld();
    if (!doesExists()) //update this to throw
        return;

    int x = getX();
    int y = getY();

    switch (currentState) {
    case stable: {
        bool supported = false;
        if (y > 0) { //if not on the bottom check if ice beneath and supporting
            Ice* leftIce = world->getMap()[y - 1][x];
            Ice* rightIce = world->getMap()[y - 1][x + 1];
            Ice* right2Ice = world->getMap()[y - 1][x + 2];
            Ice* right3Ice = world->getMap()[y - 1][x + 3];
            if (leftIce != nullptr || rightIce != nullptr || right2Ice != nullptr || right3Ice != nullptr)
                supported = true;
        }

        if (!supported) { //if not supported fall
            waitTime--;
        }

        if (waitTime == 0) {
            currentState = fall;
            world->playSound(SOUND_FALLING_ROCK);
        }
        break;
    }

    case fall: {
        if (y == 0) { //If boulder is at y == 0 hit surface immedeatley, not enough space for Actors so don't bother checking
            setExists(false);
            return;
        }

        int bx = x;        //boulder bottom‐left X
        int by = y - 1;     //boulder bottom‐left Y
        for (Actor* a : world->getCharacterList()) {
            if (!a->doesExists() || a->getID() == IID_BOULDER)
                continue;
            int ax = a->getX();
            int ay = a->getY();
            if (ax + 3 >= bx && ax <= bx + 3 //if actor in spot to be next occupied by boulder
                && ay + 3 >= by && ay <= by + 3) {
                int id = a->getID();
                if (id == IID_PLAYER) { // if IceMan or Protestor beneath do damage
                    world->playSound(SOUND_PLAYER_GIVE_UP);
                    a->setExists(false);
                }
                else if (id == IID_PROTESTER || id == IID_HARD_CORE_PROTESTER) {
                    a->setHitpoints(-1000);
                    world->playSound(SOUND_PROTESTER_ANNOYED);
                }
                // boulder shatters on impact
                setExists(false);
                return;
            }
        }

        Ice* leftIce = world->getMap()[y - 1][x];
        Ice* rightIce = world->getMap()[y - 1][x + 1];
        Ice* right2Ice = world->getMap()[y - 1][x + 2];
        Ice* right3Ice = world->getMap()[y - 1][x + 3];
        if (leftIce == nullptr && rightIce == nullptr
            && right2Ice == nullptr && right3Ice == nullptr) { // if nothing supports any part, keep falling
            moveTo(x, y - 1);
            return;
        }

        setExists(false); //else break apart
        break;
    }
    }
}

void Gold::doSomething() {
    // rewrote the logic to make bool work
    if (doesExists()) {
        IceMan* man = getWorld()->getIceMan();
        if (!isVisible() && getState() == icePickup && radiusof4(studW) == "IceMan") {
            setVisible(true); // appear on screen
            return;
        }
        else if (getState() == icePickup && radiusOf3(studW) == "IceMan") {
            setExists(false);
            getWorld()->playSound(SOUND_GOT_GOODIE);
            man->setGold(1);

            studW->increaseScore(10);// increment score by 10
            // increment iceMan's gold  by 1
        }

        if (getState() == proPickup) {


            setVisible(true); // appears on screen
            wait--;
            cout << wait << endl;

        }

        if (wait == 0) {
            setExists(false);
        }
    }
}

void Oil::doSomething() {
    StudentWorld* world = getWorld();
    IceMan* m_IceGuy = world->getIceMan(); //deal with
    int Ix = m_IceGuy->getX();
    int Iy = m_IceGuy->getY();
    if (!doesExists()) //update this to throw
        return;

    int x = getX();
    int y = getY();
    if (std::abs(x - Ix) <= 3 && std::abs(y - Iy) <= 6) { //if within radius of 6 make visible
        setVisible(true); //#TODO Don't actually know how many units before visible
    }
    if (std::abs(x - Ix) <= 3 && std::abs(y - Iy) <= 3) { //if within radius of 3 pickup Oil
        world->pickUpBarrel();
        world->playSound(SOUND_FOUND_OIL);
        world->increaseScore(100); //increase score by 100
        setExists(false);
    }
    //Can be picked up by Iceman
}

void Boulder::overlap(StudentWorld* world) {

}


void IceMan::doSomething() {
    if (doesExists()) {
        overlap(studW); // this for dig func, if iceman overlaps with ice, ice disappear
        //inRange(studW);
        isAnnoyed();
        int a;
        if (getWorld()->getKey(a) == true) { // player has pressed key
            bool blocked = false;
            switch (a) { //commented out for porject 1
            case KEY_PRESS_SPACE:// use water gun
                getWorld()->useSpray(getX(), getY());
                break;

            case KEY_PRESS_LEFT:// move left
                if (getDirection() != left) {
                    setDirection(left); // turn direction
                }
                else if (outOfField(getX(), getY(), getDirection())) { // out of scope
                    break;
                }
                else {
                    for (Actor* a : getWorld()->getCharacterList()) {
                        if (a->getID() == IID_BOULDER && a->getX() + 3 == getX() - 1 && abs(getY() - a->getY()) < 4) {
                            blocked = true;// if path blocked by boulder, stop move, player wont clip throu
                        }
                    }
                    if (!blocked) { // move if nothing is in the way
                        moveTo(getX() - 1, getY());
                    }
                }
                break;
            case KEY_PRESS_RIGHT:
                if (getDirection() != right) {
                    setDirection(right); // turn direction 
                }
                else if (outOfField(getX(), getY(), getDirection())) { // out of bounds
                    break;
                }
                else {
                    for (Actor* a : getWorld()->getCharacterList()) {
                        if (a->getID() == IID_BOULDER && a->getX() == getX() + 4 && abs(getY() - a->getY()) < 4) {
                            blocked = true; // boulder in way moment
                        }
                    }
                    if (!blocked) { // move if nothing blocks path
                        moveTo(getX() + 1, getY());
                    }

                }
                break;
            case KEY_PRESS_UP:
                if (getDirection() != up) {
                    setDirection(up); // turn direction 
                }
                else if (outOfField(getX(), getY(), getDirection())) { // when out of scope
                    break;
                }
                else {
                    for (Actor* a : getWorld()->getCharacterList()) {
                        if (a->getID() == IID_BOULDER && a->getY() == getY() + 4 && abs(getX() - a->getX()) < 4) {
                            blocked = true;// if blocked by boulder dont move
                        }
                    }
                    if (!blocked) {// move if path clear
                        moveTo(getX(), getY() + 1);
                    }
                }
                break;
            case KEY_PRESS_DOWN:
                if (getDirection() != down) {
                    setDirection(down); // turn direction
                }
                else if (outOfField(getX(), getY(), getDirection())) { // scope of out
                    break;
                }
                else {
                    for (Actor* a : getWorld()->getCharacterList()) {
                        if (a->getID() == IID_BOULDER && a->getY() + 3 == getY() - 1 && abs(getX() - a->getX()) < 4) {
                            blocked = true; // if blocked, dont move
                        }
                    }
                    if (!blocked) {// if unblocked, move
                        moveTo(getX(), getY() - 1);
                    }
                }
                break;

            case 'Z':
            case 'z':
                useSonar(studW); // Z key to use sonar
                break;
            case KEY_PRESS_TAB:
                getWorld()->dropGold(getX(), getY());//drop gold if tab
                break;
            }
        }
        else {

            return;
        }
    }
}

void IceMan::overlap(StudentWorld* world) { // Ice breaking for player
    bool digging = false; // False initally, could cause problems otherwise
    if (getY() <= 60) { // Check if y is less than 60
        for (int x = 0; x < 4; x++) { // Creates a box around player to remove the ice 
            for (int y = 0; y < 4; y++) { // Other end of the box
                if (getX() + x <= 59) { // Checks if the ice 'overlaps' player
                    if (getY() + y < 60) { //fixed aborting, Should check if its actually ice before digging
                        if ((world->getMap().at(getY() + y).at(getX() + x)) != nullptr) { // Should dynamically check if Ice then Delete ice 
                            Ice* temp = world->getMap().at(getY() + y).at(getX() + x);
                            world->getMap().at(getY() + y).at(getX() + x) = nullptr;
                            delete temp;
                            digging = true;
                        }
                    }
                    else {}
                }
            }
        }
    }
    // Play sound when true
    if (digging) {
        world->playSound(SOUND_DIG);
    }
}

void IceMan::isAnnoyed() {
    if (getHitpoints() <= 0) {
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);//if no health set dead
        setExists(false);
    }
}

void IceMan::useSonar(StudentWorld* world) { //this should make all blocks that are not visible visible in a twelve block radius, doesn't actually say what radius should be
    if (getSonarCount() <= 0) //if no kits do nothing
        return;

    setSonar(-1); //else use, play sound and update text
    world->playSound(SOUND_SONAR);

    const int Cx = getX();
    const int Cy = getY();

    for (Actor* a : world->getCharacterList()) {
        int id = a->getID();
        if ((id == IID_BARREL   //if itemObj and not visible
            || id == IID_GOLD)
            && !a->isVisible())
        {
            int Ax = a->getX() - Cx;
            int Ay = a->getY() - Cy;
            if ((Ax * Ax) + (Ay * Ay) <= 144) { //if the square difference is less than or equal to the suare of the search area
                a->setVisible(true); //make it visible
            }
        }
    }

}

string Actor::radiusOf3(StudentWorld* world) {
    if (doesExists()) {
        for (Actor* a : world->getCharacterList()) {
            int x = a->getX();
            int y = a->getY();
            double radius = sqrt(pow(x - getX(), 2) + pow(y - getY(), 2));
            if (radius <= 3) {//checks if actor is a rad of 3 away
                if (a->getID() == IID_PLAYER) {
                    return "IceMan";
                }
                else if (a->getID() == IID_PROTESTER) {
                    return "Reg Protester";
                }
                else if (a->getID() == IID_HARD_CORE_PROTESTER) {
                    return "Hard Protester";
                }
            }
        }
    }
    return "nothing near";
}

string Actor::radiusof4(StudentWorld* world) {
    if (!doesExists())
        return "nothing near";

    for (Actor* a : world->getCharacterList()) {
        double radius = sqrt(pow(a->getX() - getX(), 2) + pow(a->getY() - getY(), 2));
        bool isClose = (radius <= 4);

        string result;

        switch (a->getID()) {
        case IID_PLAYER:
            result = isClose ? "IceMan" : "Greater IceMan";
            break;
        case IID_PROTESTER:
            result = isClose ? "Protester" : "Greater Protester";
            break;
        case IID_HARD_CORE_PROTESTER:
            result = isClose ? "Hard Protester" : "Greater Hard Protester";
            break;
        default:
            continue; // Skip irrelevant actors
        }

        return result;
    }

    return "nothing near";
}

void Protester::isAnnoyed() {
    if (getHitpoints() <= 0) {
        studW->increaseScore(100);
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        leave_field = true;
        return;
    }
}


string doSomethingHelper(StudentWorld* world, int x, int y) {
    world->findPath(57, 60, x, y);//find shortest path to exit
    return world->getLeadingPathDistance(x, y);
}

bool followingPath = false;
void Protester::doSomething() {
    vector<Direction> paths;
    if (!doesExists()) {
        std::cout << "Protester is dead, skipping action.\n";
        return;
    }
    if (doesExists()) {
        if (getHitpoints() <= 0 && leave_field == false) {//check if no health
            getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
            leave_field = true;//leave field

        }
        if (ticksToWait > 0) {
            ticksToWait--;//decrement ticks
            return;
        }
        if (hasShouted > 0) {
            hasShouted--;//decrement ticks
            return;
        }
        if (ticksToWait == 0) {
            if (leave_field) {
                //cout << "PROTEST X: " << getX() << endl;
                //cout << "PROTEST Y: " << getY() << endl;

                string movement = doSomethingHelper(getWorld(), getX(), getY());

                if (movement == "left") {
                    setDirection(left);
                    moveTo(getX() - 1, getY());//move left if x != 60
                }
                else if (movement == "right") {
                    setDirection(right);
                    moveTo(getX() + 1, getY());//move right if x != 60
                }
                else if (movement == "down") {
                    setDirection(down);
                    moveTo(getX(), getY() - 1);//move up if y != 60
                }
                else if (movement == "up") {
                    setDirection(up);
                    moveTo(getX(), getY() + 1);//move down if y != 60
                }
                if (getX() == 57 && getY() == 60) {
                    setExists(false);//exit reached, set dead
                }
            }
            else {
                tryGold(getX(), getY());//try to pick up gold every step
                if (radiusof4(studW) == "IceMan" && getWorld()->checkFacingDirection(getX(), getY(), getDirection())) {
                    if (hasShouted >= 0) {//yell if 4 away from iceman and facing him
                        getWorld()->playSound(SOUND_PROTESTER_YELL);//Playannoyedsound
                        hasShouted = 15;
                        getWorld()->getIceMan()->setHitpoints(-2);
                        return;
                    }
                    moveTo(getX(), getY() - 1);
                }
                else if (radiusof4(studW) == "Greater IceMan" && iceManisInSight(getX(), getY(), getWorld())) {
                    
                    moveOne(getX(), getY(), getDirection());//if farther away move 1
                    numSquaresToMoveInCurrentDirection = 0;
                }
                else {
                    
                    if (numSquaresToMoveInCurrentDirection == 0) {
                        
                        bool chooseDirection = false;
                        int randomDir = rand() % 4;
                        while (!chooseDirection) {
                            cout << randomDir << endl;
                            //change direction to where there is no ice or boulders
                            if (availableDirections[randomDir] == left) {
                                if (!getWorld()->blockedByIce(getX() - 1, getY()) && !getWorld()->checkSpot("Boulder", getX() - 1, getY()) && getX() != 0) {
                                    chooseDirection = true;
                                    break;
                                }
                            }
                            if (availableDirections[randomDir] == right) {
                                if (!getWorld()->blockedByIce(getX() + 1, getY()) && !getWorld()->checkSpot("Boulder", getX() + 1, getY()) && getX() != 60) {
                                    chooseDirection = true;
                                    break;
                                }
                            }
                            else if (availableDirections[randomDir] == up) {
                                if (!getWorld()->blockedByIce(getX(), getY() + 1) && !getWorld()->checkSpot("Boulder", getX(), getY() + 1) && getY() != 60) {
                                    chooseDirection = true;
                                    break;
                                }
                            }
                            else if (availableDirections[randomDir] == down) {
                                if (!getWorld()->blockedByIce(getX(), getY() - 1) && !getWorld()->checkSpot("Boulder", getX(), getY() - 1) && getY() != 0) {
                                    chooseDirection = true;
                                    break;
                                }
                            }
                            randomDir = rand() % 4;//reset
                        }
                        setDirection(availableDirections[randomDir]);
                        numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);
                    }
                    
                    if (ticksforFork == 0) {//check fork time
                        
                        if (isAtFork(getX(), getY(), getWorld(), paths)) {
                            int random = rand() % paths.size();//turn random direction in fork
                            setDirection(paths.at(random));
                            numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);//reset moves
                            ticksforFork = 200;
                            paths.clear();//ticks
                        }
                    }
                    else {
                        ticksforFork--;//decrement ticks
                    }


                    moveOne(getX(), getY(), getDirection());
                    numSquaresToMoveInCurrentDirection--;
                    //cout << "Number of Squares: " << numSquaresToMoveInCurrentDirection << endl;

                    //if out of scope set moves to 0
                    if (getDirection() == left && getX() == 0) {
                        numSquaresToMoveInCurrentDirection = 0;
                    }
                    else if (getDirection() == right && getX() == 60) {
                        numSquaresToMoveInCurrentDirection = 0;
                    }
                    else if (getDirection() == up && getY() == 60) {
                        numSquaresToMoveInCurrentDirection = 0;
                    }
                    else if (getDirection() == down && getX() == 0) {
                        numSquaresToMoveInCurrentDirection = 0;
                    }
                }

            }
            ticksToWait = max(0, 3 - static_cast<int>(getWorld()->getLevel() / 4));//reset ticks
        }
    }
    return;
}

void Protester::tryGold(int x, int y) {
    if (getWorld()->pickUpGold(getX(), getY())) {
        studW->increaseScore(25);
        getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
        leave_field = true;
    }
}


void Protester::moveOne(int x, int y, Direction d) {
    switch (d) {
        // move 1 where there is no walls and not out of scope
    case up:
        if (!outOfField(x, y, d) && !getWorld()->blockedbyRocksOrIce(x, y + 1, getDirection())) {
            moveTo(getX(), getY() + 1);
            break;
        }
        else if (outOfField(x, y + 1, d) || getWorld()->blockedbyRocksOrIce(x, y + 1, getDirection())) {
            // down
            return;
        }
        break;
    case down:
        if (!outOfField(x, y, d) && !getWorld()->blockedbyRocksOrIce(x, y - 1, getDirection())) {
            moveTo(getX(), getY() - 1);
            break;
        }
        else if (outOfField(x, y - 1, d) || getWorld()->blockedbyRocksOrIce(x, y - 1, getDirection())) {
            // up
            return;
            break;
        }
        break;
    case right:
        if (!outOfField(x, y, d) && !getWorld()->blockedbyRocksOrIce(x + 1, y, getDirection())) {
            moveTo(getX() + 1, getY());
            break;
        }
        else if (outOfField(x + 1, y, d) || getWorld()->blockedbyRocksOrIce(x + 1, y, getDirection())) {
            // left
            return;
            break;
        }
        break;
    case left:
        if (!outOfField(x, y, d) && !getWorld()->blockedbyRocksOrIce(x - 1, y, getDirection())) {
            moveTo(getX() - 1, getY());
            break;
        }
        else if (outOfField(x - 1, y, d) || getWorld()->blockedbyRocksOrIce(x - 1, y, getDirection())) {
            // right
            return;
            break;
        }
        break;
    case none:
        return;
    }
}

bool Protester::isAtFork(int x, int y, StudentWorld* world, vector<Direction>& path) {

    // right
    int paths = 0;

    // if protester is moving up/down, check if it can move left/right
    if (getDirection() != left && getDirection() != right) {
        // Checking left path if no ice or boulders
        if (!getWorld()->blockedByIce(getX() - 1, getY()) && !getWorld()->checkSpot("Boulder", getX() - 1, getY())) {
            paths++;
            path.push_back(left);
        }
        // Checking right path if no ice or boulders
        if (!getWorld()->blockedByIce(getX() + 1, getY()) && !getWorld()->checkSpot("Boulder", getX() + 1, getY())) {
            paths++;
            path.push_back(right);
        }
    }

    // if protester is moving left/right, check if it can move up/down
    if (getDirection() != up && getDirection() != down) {
        // Checking up path if no ice or boulders
        if (!getWorld()->blockedByIce(getX(), getY() + 1) && !getWorld()->checkSpot("Boulder", getX(), getY() - 1)) {
            paths++;
            path.push_back(up);

        }

        // Checking down path if no ice or boulders
        if (!getWorld()->blockedByIce(getX(), getY() - 1) && !getWorld()->checkSpot("Boulder", getX(), getY() - 1)) {
            paths++;
            path.push_back(down);
        }
    }

    // cout << paths << endl;

    return paths != 0 ? true : false;
}

bool Actor::isFacingIceMan(Direction d, StudentWorld* world) {
    for (Actor* a : world->getCharacterList()) {
        if (a->getID() == IID_PLAYER && a->getDirection() != getDirection()) {//if not facing iceman return false
            break;
            return false;
        }
        break;
        return true;
    }
    return false;
}
void Actor::setFacingIceMan(Direction d, StudentWorld* world) {
    if (!isFacingIceMan(getDirection(), studW)) {//if not facing iceman, set direction to his direction
        setDirection(world->getIceMan()->getDirection());
    }
}


bool squirtIsInBounds(int x, int y) {
    return x >= 0 && x < VIEW_WIDTH && y >= 0 && y < VIEW_HEIGHT;
}

void Squirt::doSomething() {
    int currX = getX();
    int currY = getY();


    // Check if initial position is invalid
    if (!squirtIsInBounds(currX, currY) || getWorld()->blockedbyRocksOrIce(currX, currY, getDirection())) {
        setExists(false);
        return;
    }

    if (distance == 0) {
        setExists(false);
        return;
    }

    int targetX = currX;
    int targetY = currY;

    switch (getDirection()) {
    case up:    targetY += 1; break;
    case down:  targetY -= 1; break;
    case left:  targetX -= 1; break;
    case right: targetX += 1; break;
    default: break;
    }

    // Check if new position is within bounds
    if (!squirtIsInBounds(targetX, targetY) || getWorld()->blockedbyRocksOrIce(targetX, targetY, getDirection())) {
        setExists(false);
        return;
    }

    moveTo(targetX, targetY);
    deDIS(); // decrease distance
}


bool Protester::iceManisInSight(int x, int y, StudentWorld* world) {
    if (world->getIceMan() != NULL) {
        int xI = world->getIceMan()->getX();
        int yI = world->getIceMan()->getY();
        if (x == xI) { // if they are on same row
            if (y <= yI) {
                for (int j = y; j <= yI; j++) {
                    for (int i = x; i < 4; i++) {
                        if (world->getMap()[j][x + i] != nullptr || world->checkSpot("Boulder", x + i, j)) {
                            return false; // if ice or boulder
                        }
                    }
                }
                cout << "UP" << endl;
                setDirection(up);
                return true;
            }
            else {
                for (int j = y; j >= yI; j--) {
                    for (int i = 0; i < 4; i++) {
                        if (world->getMap()[j][x + i] != nullptr || world->checkSpot("Boulder", x + i, j)) {
                            return false;
                        }
                    }
                }
                cout << "Down" << endl;
                setDirection(down);
                return true;
            }
        }
        else if (y == yI) { // if they are on same column
            if (x <= xI) {
                for (int j = x; j <= xI; j++) {
                    for (int i = 0; i < 4; i++) {
                        if (world->getMap()[y + i][j] != nullptr || world->checkSpot("Boulder", j, y + i)) {
                            return false;
                        }
                    }
                }
                cout << "Right" << endl;
                setDirection(right);
                return true;
            }
            else {
                for (int j = x; j >= xI; j--) {
                    for (int i = 0; i < 4; i++) {
                        if (world->getMap()[y + i][j] != nullptr || world->checkSpot("Boulder", j, y + i)) {
                            return false;
                        }
                    }
                }
                cout << "left" << endl;
                setDirection(left);
                return true;
            }
        }
    }
    return false;
}

void HardcoreProtester::doSomething() {
    if (!doesExists()) return;

    if (getHitpoints() <= 0 && !leave_field) {
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        studW->increaseScore(250);
        leave_field = true;
    }

    // waiting 
    if (ticksToWait > 0) {
        ticksToWait--;
        return;
    }
    if (hasShouted > 0) {
        hasShouted--;
        return;
    }

    // General leaving pathfinding
    if (leave_field) {
        getWorld()->findPath(57, 60, getX(), getY());
        string movement = getWorld()->getLeadingPathDistance(getX(), getY());

        if (movement == "left") {
            setDirection(left);
            moveTo(getX() - 1, getY());
        }
        else if (movement == "right") {
            setDirection(right);
            moveTo(getX() + 1, getY());
        }
        else if (movement == "down") {
            setDirection(down);
            moveTo(getX(), getY() - 1);
        }
        else if (movement == "up") {
            setDirection(up);
            moveTo(getX(), getY() + 1);
        }

        if (getX() == 57 && getY() == 60) {
            setExists(false);
        }
    }
    else {
        tryGold(getX(), getY());
        if (radiusof4(studW) == "IceMan" && getWorld()->checkFacingDirection(getX(), getY(), getDirection())) {
            if (hasShouted >= 0) {
                getWorld()->playSound(SOUND_PROTESTER_YELL);
                hasShouted = 15;
                getWorld()->getIceMan()->setHitpoints(-2);
                return;
            }
            moveTo(getX(), getY() - 1);
        }

        else if (radiusof4(studW) == "Greater IceMan" && iceManisInSight(getX(), getY(), getWorld())) {
            moveOne(getX(), getY(), getDirection());
            numSquaresToMoveInCurrentDirection = 0;
        }

        // random movement and turning
        else {
            if (numSquaresToMoveInCurrentDirection == 0) {
                bool directionChosen = false;

                while (!directionChosen) {
                    int randomDir = rand() % 4;
                    Direction dir = availableDirections[randomDir];
                    int nx = getX(), ny = getY();

                    if (dir == left) nx--;
                    else if (dir == right) nx++;
                    else if (dir == up) ny++;
                    else if (dir == down) ny--;

                    bool valid = !getWorld()->blockedByIce(nx, ny) &&
                        !getWorld()->checkSpot("Boulder", nx, ny) &&
                        nx >= 0 && nx <= 57 &&
                        ny >= 0 && ny <= 60;

                    if (valid) {
                        setDirection(dir);
                        numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);
                        directionChosen = true;
                    }
                }
            }

            if (ticksforFork == 0) {
                vector<Direction> forkPaths;
                if (isAtFork(getX(), getY(), getWorld(), forkPaths)) {
                    int randomFork = rand() % forkPaths.size();
                    setDirection(forkPaths[randomFork]);
                    numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);
                    ticksforFork = 200;
                }
            }
            else {
                ticksforFork--;
            }

            moveOne(getX(), getY(), getDirection());
            numSquaresToMoveInCurrentDirection--;

            // if at edge of map
            int x = getX(), y = getY();
            Direction d = getDirection();
            if ((d == left && x == 0) || (d == right && x == 56) ||
                (d == up && y == 60) || (d == down && y == 0)) {
                numSquaresToMoveInCurrentDirection = 0;
            }
        }
    }

    ticksToWait = max(0, 3 - static_cast<int>(getWorld()->getLevel() / 4));
}

void HardcoreProtester::tryGold(int x, int y) {
    if (getWorld()->pickUpGold(getX(), getY())) {
        if (getGold() == 1) { // hardpro leaves only if picks up 2 gold
            cout << "im rich" << endl;
            getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            studW->increaseScore(50);
            leave_field = true;
        }
        else { // other wise continues persuit
            cout << "youre too cheap" << endl;
            setGoldInv(1); // increment inventory
        }
    }
}


void SonarKit::doSomething() { //if picked up by IceMan add 75 points
    //play sound SOUND_GOT_GOODIE
    //Increment IceMan inventory of SonarKits
    //Spawn in visible, can't be annoyed or block squirts
    //if picked up by Iceman or its lifespan is over set dead
    StudentWorld* world = getWorld();
    if (!doesExists())
        return;

    IceMan* m_IceGuy = world->getIceMan();
    if (m_IceGuy == nullptr)
        return;

    int Ix = m_IceGuy->getX();
    int Iy = m_IceGuy->getY();
    int x = getX();
    int y = getY();


    if (std::abs(x - Ix) <= 3 && std::abs(y - Iy) <= 3) { //if within radius of 3 pickup gold
        world->setDisplayText();
        m_IceGuy->setSonar(1); //#debug setSonar(99);
        world->increaseScore(75); //increment score by 75
        world->playSound(SOUND_GOT_GOODIE);
        setExists(false);
    }
    deTIC();
    if (ticksToWait <= 0)
        setExists(false);
}

void Waterpool::doSomething() { //if picked up by IceMan add 100 points
    //play sound SOUND_GOT_GOODIE
    //give IceMan 5 squirts for his water gun
    //Spawn in visible, can't be annoyed or block squirts
    //if picked up by Iceman or its lifespan is over set dead
    StudentWorld* world = getWorld();
    if (!doesExists())
        return;

    IceMan* m_IceGuy = world->getIceMan();
    if (m_IceGuy == nullptr)
        return;

    int Ix = m_IceGuy->getX();
    int Iy = m_IceGuy->getY();
    int x = getX();
    int y = getY();


    if (std::abs(x - Ix) <= 3 && std::abs(y - Iy) <= 3) { //if within radius of 3 pickup gold
        world->setDisplayText();
        m_IceGuy->setWater(5);
        world->increaseScore(100); //increase score by 100
        world->playSound(SOUND_GOT_GOODIE);
        setExists(false);
    }
    deTIC();
    if (ticksToWait <= 0)
        setExists(false);
}