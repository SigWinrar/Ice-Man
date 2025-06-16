#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameWorld.h"
#include <cstdlib>
#include <cmath>
#include <queue>

using namespace std;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

class Actor : public GraphObject {
private: // just here in case we need it
protected: // here in case we need it
	int hitPoints;
	int xCoord, yCoord;
	bool doesitExist;
	StudentWorld* studW;

public:
	//StudentWorld* studW;
	Actor(int imageID, int startX, int startY, Direction startDirection, StudentWorld* world, double size = 1.0, int depth = 0) :
		GraphObject(imageID, startX, startY, startDirection, size, depth) {
		doesitExist = true; // check if something is spawned
		studW = world;

	};
	virtual void overlap(StudentWorld* world) {}
	StudentWorld* getWorld() { return studW; }
	bool outOfField(int x, int y, Direction d);
	string radiusof4(StudentWorld* world);
	string radiusOf3(StudentWorld* world);
	bool isFacingIceMan(Direction d, StudentWorld* world);
	void setFacingIceMan(Direction d, StudentWorld* world);
	int getHitpoints() { return hitPoints; }
	virtual void setHitpoints(int a) { hitPoints += a; }
	bool doesExists() { return doesitExist; };
	void setExists(bool exists) { doesitExist = exists; };
	virtual void doSomething() = 0;
	virtual ~Actor() {
		//std::cout << "Actor destroyed (base)\n";
	}
};

class AnnoyedObj : public Actor {
public:
	AnnoyedObj(int imageID, int startX, int startY, Direction startDirection, StudentWorld* world, double size = 1.0, int depth = 0) :
		Actor(imageID, startX, startY, startDirection, world, size, depth) {

	}
protected:
	virtual void isAnnoyed() = 0;

};

class Protester : public AnnoyedObj {
protected:
	bool leave_field;
	//StudentWorld* studW; //student world needed
	int moves;
	int shout;
	int hasShouted;
	int tickStun;
	int ticksToWait;
	int numSquaresToMoveInCurrentDirection;
	bool stun;
	string direction = "left";
	Direction availableDirections[4] = { left, right, down, up };
	int ticksforFork = 200;
	int path[64][64]; // set the field where the protestor can mov, such 15x15
public:
	Protester(int startX, int startY, int imageID, int tStun, int waitTime, StudentWorld* world) :
		AnnoyedObj(imageID, startX, startY, left, world, 1.0, 0) {
		numSquaresToMoveInCurrentDirection = 8 + (rand() % 53);
		hitPoints = 5; // temporary value
		shout = 15; // temp value
		stun = false;
		tickStun = tStun;
		ticksToWait = waitTime;
		hasShouted = 15; // temp value
		leave_field = false; // does not leave field yet, squirt
		setVisible(true); // appear on screen
	};
	virtual void isAnnoyed();
	//	virtual bool setHit(int a) { }
	void virtual tryGold(int x, int y);
	//	virtual void deSHOU() { shout--; };
	virtual int getStun() { return tickStun; }
	virtual void deSTUN() { tickStun--; };
	//	virtual void resetSHOU() { hasShouted = 10; }
	virtual bool iceManisInSight(int x, int y, StudentWorld* world);
	virtual bool isAtFork(int x, int y, StudentWorld* world, vector<Direction>& path);
	virtual void doSomething() override;
	virtual void setLeaveField(bool leave) { leave_field = leave; }
	virtual void moveOne(int x, int y, Direction d);
	virtual ~Protester() { std::cout << "Protester at (" << getX() << ", " << getY() << ") destroyed.\n"; };

};


class HardcoreProtester : public Protester {
public:
	HardcoreProtester(int startX, int startY, int ticks_to_stare, int waitTime, StudentWorld* world) : Protester(startX, startY, IID_HARD_CORE_PROTESTER, ticks_to_stare, waitTime, world) {
		numSquaresToMoveInCurrentDirection = 8 + (rand() % 60);
		goldInv = 0;
		leave_field = false;//doesn't leave field bc is Alive
		hitPoints = 20;//set data members numbers specified by packet
		setVisible(true);//appear on screen
	};
	virtual void doSomething() override;
	virtual void tryGold(int x, int y) override;
	void setGoldInv(int a) { goldInv += a; }
	int getGold() { return goldInv; }
	int getTickStare() { return ticks_to_stare; }
	virtual ~HardcoreProtester() {};
private:
	int ticksToWaitBetweenMoves;
	int ticks_to_stare;
	int goldInv;
};

class ItemObj : public Actor {
public:
	int tickRange;
	ItemObj(int imageID, int startX, int startY, float size, int depth, Direction startDirection, StudentWorld* world)
		: Actor(imageID, startX, startY, startDirection, world) {

	}
	bool pickUpAble() { return true; }
	virtual void doSomething() = 0;
	bool pickUp() { return true; }
	bool disappear(int numTicks) { return true; }
	int getTicks() { return 0; }


	virtual ~ItemObj() {}
};

class IceMan : public Actor {
private:
	int damage;
	int waterSquirt;
	int sonarkit;
	int oil;
	int gold;
public:
	IceMan(int startX, int startY, StudentWorld* world) : Actor(IID_PLAYER, startX, startY, left, world, 1.0, 0) {
		hitPoints = 10;
		oil = 0;
		studW = world;
		waterSquirt = 20;
		sonarkit = 99; //debug 99
		gold = 0;
		damage = 100; // set data
		setVisible(true);

	};
	void isAnnoyed();
	void dropGold(StudentWorld* world);
	int getGold() { return gold; }
	int getOil() { return oil; }
	int getSquirt() { return waterSquirt; }
	int getSonarCount() { return sonarkit; }
	double getHealth() { return hitPoints; }
	void setWater(int a) { waterSquirt += a; }
	void setOil(int a) { oil += a; }
	void setGold(int a) { gold += a; }
	void setSonar(int a) { sonarkit += a; }
	void useSonar(StudentWorld* world);
	virtual void overlap(StudentWorld* world) override;
	virtual void doSomething() override;
	virtual ~IceMan() {}
};

class Ice : public GraphObject { //change back to GraphObject if you want
public:
	int* dataIce; // temporary data value for destructor, feel free to use or not use
	Ice(int startX, int startY)
		: GraphObject(IID_ICE, startX, startY, left, 0.25, 3) {
		dataIce = new int[100]; // temporary data value, change number as see
		setVisible(true);
	};
	void overlap(StudentWorld* world); //this is not defined initially reference it like Ice was an Acotr #TODO
	virtual ~Ice() { // virtual is here to better handle polymorph stuff.
		delete[] dataIce;
		//std::cout << "Destructor called, Ice culled..." << endl;
	} // This should be callable, use how see fit 
};

class Squirt : public ItemObj {
public:
	Squirt(int startX, int startY, Direction d, StudentWorld* world)
		: ItemObj(IID_WATER_SPURT, startX, startY, 1.0, 1, d, world) {
		if (!isFacingIceMan(getDirection(), world)) {
			setFacingIceMan(getDirection(), studW);
		}
		distance = 4;
		setVisible(true); // appear on screen

	}
	virtual void doSomething() override;
	void deTIC() { ticksWait--; }
	void deDIS() { distance--; }
	virtual ~Squirt() {}
private:
	int distance;
	int ticksWait;
};

class Oil : public ItemObj {
public:
	Oil(int startX, int startY, StudentWorld* world)
		: ItemObj(IID_BARREL, startX, startY, 1.0, 2, right, world) {
		setVisible(false); // appear on scrn
	}
	virtual void doSomething() override;
	virtual ~Oil() {}
};

class Boulder : public ItemObj {
public:
	Boulder(int startX, int startY, StudentWorld* world) : ItemObj(IID_BOULDER, startX, startY, 1.0, 1, down, world) {
		setVisible(false);
		currentState = stable; // boulder's intial state, buried in 'stable' ground
		doesitExist = true; // boulder is in game
		waitTime = 10;
	}
	virtual void doSomething() override;
	virtual void overlap(StudentWorld* world) override;
	virtual ~Boulder() {}

	// state 0 1 2 
	enum state {
		stable,
		wait,
		fall
	};

	state getState() { return currentState; }
	state currentState;
	int waitTime;    //ticks to wait before falling
};

class Gold : public ItemObj {
public:
	Gold(int startX, int startY, bool isDropped, StudentWorld* world) : ItemObj(IID_GOLD, startX, startY, 1.0, 2, right, world) {
		if (isDropped == false) {
			setVisible(false);
			currentState = icePickup;

		}

		if (isDropped == true) {
			setState(proPickup);
			setVisible(true);
		}

	}
	// will remain in temp state, will disappear if a protester picks up or expires after some time
	//bool canPickup(Actor Iceman, StudentWorld* world);

	// iceman drops
	virtual void doSomething() override;
	virtual ~Gold() {}

	// different states
private:
	enum state {
		waiting,
		icePickup, // player picks up gold
		proPickup // protester picks up gold
	};
	state currentState;
	state getState() { return currentState; }
	void setState(enum state s) { currentState = s; }
	int wait = 50;
};

class SonarKit : public ItemObj {
public:
	SonarKit(int startX, int startY, int ticks, StudentWorld* world) : ItemObj(IID_SONAR, startX, startY, 1.0, 2, right, world) {
		ticksToWait = ticks;
		setVisible(true);
		setExists(true);
	}
	virtual void doSomething() override;
	void deTIC() { ticksToWait--; };
	virtual ~SonarKit() {}
private:
	int ticksToWait;
};

class Waterpool : public ItemObj {
public:
	Waterpool(int startX, int startY, int ticksWait, StudentWorld* world) : ItemObj(IID_WATER_POOL, startX, startY, 1.0, 2, right, world) {
		ticksToWait = ticksWait;
		setVisible(true);
	}
	virtual void doSomething() override; //like Sonar. Need to implement in student world cleanup of Non-existant objects
	void deTIC() { ticksToWait--; };
	bool tickPassed(int ticksWait);
	virtual ~Waterpool() {};
private:
	int ticksToWait;
};



#endif // ACTOR_H_ 
