/***************************************************************************
                            -  description
                             -------------------
    begin                : Thu Mar 9 2000
    copyright            : (C) 2000 by henqvist
    email                : henqvist@excite.com
 ***************************************************************************/

#include <fstream>
#include <string>

#include <stdlib.h>
#include <time.h>
//#include <values.h>
#include "Pinball.h"

#include "Keyboard.h"
#include "Menu.h"
#include "Engine.h"
#include "ArmBehavior.h"
#include "CollisionBounds.h"
#include "Camera.h"
#include "Light.h"
#include "Cube.h"
#include "BigSphere.h"
#include "Cylinder.h"
#include "BounceBehavior.h"
#include "KeyBehavior.h"
#include "Shape3DUtil.h"
#include "Grid.h"
#include "KeyRotBehavior.h"
#include "Polygon.h"
#include "BumperBehavior.h"
#include "CaveBehavior.h"
#include "LockBehavior.h"
#include "TriggerBehavior.h"
#include "SwitchBehavior.h"
#include "TextureUtil.h"
#include "Score.h"
#include "StdAnimation.h"

#define CHECK_EOF(file) if (!file) throw string("eof reached too soon");

/* Normal Object */
Group * loadStdObject(ifstream & file) {
	char collision[256], shape[256];
	float tx, ty, tz;
	float rx, ry, rz;
	float r, g, b;

	file >> collision;
	file >> shape;
	file >> tx; file >> ty; file >> tz;
	file >> rx; file >> ry;	file >> rz;
	file >> r; file >> g; file >> b;

	cout << collision << endl;
	cout << shape << endl;
	cout << tx <<" "<< ty <<" "<< tz << endl;
	cout << rx <<" "<< ry <<" "<< rz << endl;
	cout << r <<" "<< g <<" "<< b << endl;

	Group* group = new Group();
	group->setTransform(tx, ty, tz, rx, ry, rz);

	// load collision bounds
	if (strstr(collision, "?") == NULL) {			
		Shape3D* shape3d = Shape3DUtil::loadShape3D(collision);
		if (shape3d != NULL) {
			//shape3d->setPolygonProperty(EM_POLY_HIDDEN);
			group->addShape3D(shape3d);
			
			CollisionBounds* bounds = new CollisionBounds(shape3d->getCollisionSize());
			if (shape3d->getCollisionSize() > 10) {
				bounds->setShape3D(shape3d, 4); //4
			} else {
				bounds->setShape3D(shape3d, 0);
			}				
			group->setCollisionBounds(bounds);
		}
	}
		
	// load shape
	if (strstr(shape, "?") == NULL) {
		Shape3D* shape3d = Shape3DUtil::loadShape3D(shape);
		group->addShape3D(shape3d);
		shape3d->setProperty(EM_SHAPE3D_HIDDEN);
	}

	return group;
}

/* light */
Group * loadStdLight(ifstream & file, Engine * engine, Behavior * behavior) {
	string str;
	Group * group = NULL;
	file >> str;
	if (str == "light" ) {
		float tx, ty, tz;
		int r, g, b;
		file >> tx; file >> ty; file >> tz;
		file >> r;  file >> g;  file >> b;
		group = new Group();
		Light * light = new Light(1.3f, 0.5f, 0.0f, r, g, b);
		//		light->setColor(r, g, b);
		light->setOn(false);
		light->setProperty(EM_IGNORE_ANGLE_FULL);
		//light->setProperty(EM_IGNORE_DISTANCE);
		light->setProperty(EM_USE_BOUNDS);
		light->setBounds(10.0);

		if (behavior != NULL) behavior->setLight(light);

		engine->addLight(light);
		group->setTranslation(tx, ty, tz);
		group->setLight(light);
		cerr << "loaded light" << endl;
	}
	return group;
}

/* Animation */
StdAnimation * loadStdAnimation(ifstream & file, Engine * engine) {
	string str;
	StdAnimation * anim = NULL;
	file >> str;
	if (str == "animation" ) {
		string str;
		int steps, speed;
		file >> str;
		file >> steps;
		file >> speed;
		if (str == "type_light") {
			anim = new StdAnimation(speed, EM_LIGHT);
		} else if (str == "type_rot") {
			anim = new StdAnimation(speed, EM_ROTATION);
		} else {
			anim = new StdAnimation(speed, EM_TRANSLATION);
		}
		for (int a=0; a<steps; a++) {
			float tx, ty, tz;
			file >> tx; file >> ty; file >> tz;
			anim->add(tx, ty, tz);
		}
		anim->setEndStart();
		cerr << "loaded animation" << endl;
	}
	return anim;
}

/* Walls */
void loadWalls(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);

	group->setUserProperty(PBL_WALLS);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);
			
	cerr << "loaded walls" << endl;
}

/* Left arm */
void loadLeftarm(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);

	ArmBehavior* armBeh = new ArmBehavior(false);

	group->addBehavior(armBeh);
	group->setUserProperty(PBL_GROUP1);
}

/* Right arm*/
void loadRightarm(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);

	ArmBehavior* armBeh = new ArmBehavior(true);

	group->addBehavior(armBeh);
	group->setUserProperty(PBL_GROUP1);
}

/* Bumper */
void loadBumper(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision cylinder
	Shape3D* shape3d_c = new Cylinder(4.0, 1.0, 1.0, 0.0, 0.0, false);
	//shape3d_c->setPolygonProperty(EM_POLY_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible cylinder
	Shape3D* shape3d = new Cylinder(4.0, 1.0, 1.0, 0.0, 0.0);
	group->addShape3D(shape3d);
	shape3d->setProperty(EM_SHAPE3D_HIDDEN);

	BumperBehavior* bumperBeh = new BumperBehavior();
	group->addBehavior(bumperBeh);
	group->setUserProperty(PBL_BUMPER);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);
				
	Group * groupL = loadStdLight(file, engine, bumperBeh);
	group->add(groupL);	
  		
	cerr << "loaded bumper" << endl;
}

/* Flat bumper
 */
void loadFlatbumper(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 4.0, 4.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid
	Shape3D* shape3d = new Grid(NULL, 4.0, 4.0, 1, 0);
	group->addShape3D(shape3d);

	BumperBehavior* bumperBeh = new BumperBehavior();
	group->addBehavior(bumperBeh);
	group->setUserProperty(PBL_BUMPER);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, bumperBeh);
	group->add(groupL);	
  		
	cerr << "loaded flatbumper" << endl;
}

/* Target
 */
void loadTarget(ifstream & file, Engine * engine) {
	string str;
	file >> str;

	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid
	Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = NULL;
	if (str == "1") {
		triggerBeh = new TriggerBehavior( PBL_SIG_NEW_TARGET, PBL_SIG_TARGET_1, 
																			PBL_SIG_TARGET_1, true);
	} else {
		triggerBeh = new TriggerBehavior( PBL_SIG_NEW_TARGET, PBL_SIG_TARGET_2, 
																			PBL_SIG_TARGET_2, true);
	}
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded target" << str<< endl;
}

/* Cave
 */
void loadCave(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	//	Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	//	group->addShape3D(shape3d);

	CaveBehavior* caveBeh = new CaveBehavior();
	group->addBehavior(caveBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, caveBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded cave" << endl;
}

/* Lock target
 */
void loadLock_target(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid
	Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = new TriggerBehavior( PBL_SIG_NEW_LOCK, PBL_SIG_ACTIVATE_LOCK, 
																										 PBL_SIG_ACTIVATE_LOCK, true);
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded lock target" << endl;
}

/* Lock 1
 */
void loadLock(ifstream & file, Engine * engine) {
	string str;
	file >> str;

	Group * group = loadStdObject(file);
	engine->add(group);

	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	//Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	//group->addShape3D(shape3d);

	LockBehavior* lockBeh = NULL;
	if (str == "1") {
		lockBeh = new LockBehavior(PBL_SIG_ACTIVATE_LOCK, PBL_SIG_RELEASE_LOCK, PBL_SIG_LOCK_1);
	}	else {
		lockBeh = new LockBehavior(PBL_SIG_LOCK_1, PBL_SIG_RELEASE_LOCK, PBL_SIG_LOCK_2);
	}
	group->addBehavior(lockBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, lockBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded lock " << str << endl;
}

/* Lock release
 */
void loadLock_release(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = new TriggerBehavior( PBL_SIG_LOCK_2, PBL_SIG_RELEASE_LOCK, 
																										 PBL_SIG_RELEASE_LOCK);
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded lock release" << endl;
}

/* Jackpot
 */
void loadJackpot(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	//Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	//group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = new TriggerBehavior( PBL_SIG_RELEASE_LOCK, 
																										 PBL_SIG_MULTIBALL_DEAD, PBL_SIG_JACKPOT);
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded jackpot" << endl;
}

/* Left loop
 */
void loadLeft_loop(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	//Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	//group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = new TriggerBehavior( PBL_SIG_ACTIVATE_LEFT_LOOP, 
																										 PBL_SIG_LEFT_LOOP, PBL_SIG_LEFT_LOOP);
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded left loop" << endl;
}

/* Right loop
 */
void loadRight_loop(ifstream & file, Engine * engine) {
	Group * group = loadStdObject(file);
	engine->add(group);
	// The collision grid
	Shape3D* shape3d_c = new Grid(NULL, 2.0, 2.0, 1, 0);
	shape3d_c->setProperty(EM_SHAPE3D_HIDDEN);
	group->addShape3D(shape3d_c);

	CollisionBounds* bounds = new CollisionBounds(shape3d_c->getCollisionSize());
	bounds->setShape3D(shape3d_c, 0);
	group->setCollisionBounds(bounds);

	// The visible grid TODO: remove me
	//Shape3D* shape3d = new Grid(NULL, 2.0, 2.0, 1, 0, 0.0, 1.0, 0.0, 1.0);
	//group->addShape3D(shape3d);

	TriggerBehavior* triggerBeh = new TriggerBehavior( PBL_SIG_ACTIVATE_RIGHT_LOOP, 
																										 PBL_SIG_RIGHT_LOOP, PBL_SIG_RIGHT_LOOP);
	group->addBehavior(triggerBeh);
	group->setUserProperty(PBL_GROUP1);
	group->setProperty(EM_GROUP_TRANSFORM_ONCE);

	Group * groupL = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (groupL != NULL) groupL->addBehavior(anim);
	group->add(groupL);	
  		
	cerr << "loaded right loop" << endl;
}

/* Light
 */
void loadExtra_light(ifstream & file, Engine * engine) {
	int active, unactive, signal;
	file >> active; 
	file >> unactive; 
	file >> signal;
	TriggerBehavior* triggerBeh = new TriggerBehavior( active, unactive, signal );

	Group * group = loadStdLight(file, engine, triggerBeh);
	StdAnimation * anim = loadStdAnimation(file, engine);
	if (group != NULL) {
		group->addBehavior(triggerBeh);
		group->setUserProperty(PBL_GROUP1);
		group->setProperty(EM_GROUP_TRANSFORM_ONCE);

		group->addBehavior(anim);
		engine->add(group);	
  }

	cerr << "loaded extra light" << endl;
}

/* Main load file function
 */
int loadFile(const char * fn, Engine * engine) {
	ifstream file(fn);
	string str;
	if (!file) {
		cerr << "file not found" << endl;
		return -1;
	}
	cerr << "file found" << endl;

	while (file >> str) {
		if (str == "allend") return 0;
		if (str == "walls")      loadWalls(file, engine);
		if (str == "leftarm")    loadLeftarm(file, engine);
		if (str == "rightarm")   loadRightarm(file, engine);
		if (str == "bumper")     loadBumper(file, engine);
		if (str == "flatbumper") loadFlatbumper(file, engine);
		if (str == "target")   loadTarget(file, engine);
		if (str == "cave")       loadCave(file, engine);
		if (str == "lock_target") loadLock_target(file, engine);
		if (str == "lock")       loadLock(file, engine);
		if (str == "lock_release") loadLock_release(file, engine);
		if (str == "jackpot")    loadJackpot(file, engine);
		if (str == "left_loop")  loadLeft_loop(file, engine);
		if (str == "right_loop") loadRight_loop(file, engine);
		if (str == "extra_light") loadExtra_light(file, engine);
		while (file >> str && str != "end") {
			cerr << "Ignored " << str << endl;
		}
  }
	return 0;
}

/******************************************************/

void newloadMisc(ifstream & file, Engine * engine, Group * group, Behavior * beh);

/* set properties for a group */
void newloadProperties(ifstream & file, Group * g) {
	cerr << "properties" << endl;

	string str;
	
	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	file >> str;
	while (str != "}") {
		CHECK_EOF(file);
		if (str == "transform_once") {
			g->setProperty(EM_GROUP_TRANSFORM_ONCE);
		} else if (str == "no_light") {
			g->setProperty(EM_GROUP_NO_LIGHT);
		} else if (str == "light_once") {
			g->setProperty(EM_GROUP_LIGHT_ONCE);
		} else if (str == "no_signal") {
			g->setProperty(EM_GROUP_NO_SIGNAL);
		} else if (str == "user_walls") {
			g->setUserProperty(PBL_WALLS);
		} else if (str == "user_group1") {
			g->setUserProperty(PBL_GROUP1);
		} else {
			throw string("Unkown property in property block");
		}
		file >> str;
	}
}

void newloadArmBehavior(ifstream & file, Group * group) {
	cerr << "arm" << endl;

	string str;

	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	file >> str;
	if (str == "right") {
		group->addBehavior(new ArmBehavior(true));
	} else {
		group->addBehavior(new ArmBehavior(false));
	}

	file >> str;
	if (str != "}") {
		throw string("Parse error \"}\" missing");
	}
}

void newloadAnimation(ifstream & file, Engine * engine, Group * group, Behavior * beh) {
	cerr << "anim" << endl;

	string str;

	EmAssert(beh != NULL, "Behavior NULL in newloadAnimation");
	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	int type = EM_LIGHT;
	file >> str;
	if (str == "rotation") {
		type = EM_ROTATION;
	} else if (str == "translation") {
		type = EM_TRANSLATION;
	}
	StdAnimation* anim = new StdAnimation(20, type);

	int count;
	file >> count;
	for (; count > 0; count--) {
		float a, b, c;
		file >> a; file >> b; file >> c;
		if (count == 1) {
			anim->setEnd(a, b, c);
		} else {
			anim->add(a, b, c);
		}
	}
	group->addBehavior(anim);

	newloadMisc(file, engine, group, anim);
}


void newloadBehaviorLight(ifstream & file, Engine * engine, Group * group, Behavior * beh) {
	cerr << "light" << endl;

	string str;
	
	EmAssert(beh != NULL, "Behavior NULL in newloadBehaviorLight");
	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	float r, g, b, x, y, z;
	file >> r; file >> g; file >> b;
	file >> x; file >> y; file >> z;

	Light * light = new Light(1.3f, 0.5f, 0.0f, r, g, b);
	light->setOn(false);
	light->setProperty(EM_IGNORE_ANGLE_FULL);
	//light->setProperty(EM_IGNORE_DISTANCE);
	light->setProperty(EM_USE_BOUNDS);
	light->setBounds(10.0);
	
	Group * gl = new Group();
	gl->addTranslation(x, y, z);
	gl->setLight(light);
	group->add(gl);
	engine->addLight(light);
	
	beh->setLight(light);

	newloadMisc(file, engine, gl, beh);
}

void newloadCaveBehavior(ifstream & file, Engine * engine, Group* group) {
	cerr << "cave" << endl;

	string str;

	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	Behavior* b = new CaveBehavior();
	group->addBehavior(b);

	Shape3D* shape = new Cube(1, 0, 1, 0, 1);
	group->addShape3D(shape);
	CollisionBounds* bounds = new CollisionBounds(shape->getCollisionSize());
	bounds->setShape3D(shape, 0); 
	group->setCollisionBounds(bounds);

	newloadMisc(file, engine, group, b);
}

void newloadBumperBehavior(ifstream & file, Engine * engine, Group * group) {
	cerr << "bumper" << endl;

	string str;

	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	Behavior* b = new BumperBehavior();
	group->addBehavior(b);

	newloadMisc(file, engine, group, b);
}

void newloadSwitchBehavior(ifstream & file, Engine * engine, Group * group) {
	cerr << "switch" << endl;

	string str;

	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	int asig, usig, init;
	file >> asig;	file >> usig;	file >> init;

	Behavior* b = new SwitchBehavior(asig, usig, (init==1));
	group->addBehavior(b);

	newloadMisc(file, engine, group, b);
}

/* thins added to object, e.g. lights, animation, behavior*/
void newloadMisc(ifstream & file, Engine * engine, Group * group, Behavior * beh) {
	cerr << "misc" << endl;

	string str;

	EmAssert(engine != NULL && group != NULL, "Engine or group NULL in newloadMisc");

	file >> str;
	while (str != "}") {
		CHECK_EOF(file);
		if (str == "properties"){
			newloadProperties(file, group);
		} else if (str == "arm_behavior") {
			newloadArmBehavior(file, group);
		} else if (str == "switch_behavior") {
			newloadSwitchBehavior(file, engine, group);
		} else if (str == "cave_behavior") {
			newloadCaveBehavior(file, engine, group);
		} else if (str == "bumper_behavior") {
			newloadBumperBehavior(file, engine, group);
			group->setUserProperty(PBL_BUMPER);
		} else if (str == "light") {
			newloadBehaviorLight(file, engine, group, beh);
		} else if (str == "animation") {
			newloadAnimation(file, engine, group, beh);
		} else {
			throw string("Unkown in misc block");
		}
		file >> str;
	}
}

/* Normal Object */
Group * newloadStdObject(ifstream & file, Engine * engine) {
	cerr << "object" << endl;

	char collision[256], shape[256];
	float tx, ty, tz;
	float rx, ry, rz;
	string str;

	file >> str;
	if (str != "{") {
		throw string("Parse error \"{\" missing");
	}

	file >> collision;
	file >> shape;
	file >> tx; file >> ty; file >> tz;
	file >> rx; file >> ry;	file >> rz;
	
	Group* group = new Group();
	group->setTransform(tx, ty, tz, rx, ry, rz);

	// load collision bounds
	if (strstr(collision, "?") == NULL) {			
		Shape3D* shape3d = Shape3DUtil::loadShape3D(collision);
		if (shape3d == NULL) {
			throw string("File not found");
		}
		// TODO add this
		//shape3d->setProperty(EM_SHAPE3D_HIDDEN);
		group->addShape3D(shape3d);
		
		CollisionBounds* bounds = new CollisionBounds(shape3d->getCollisionSize());
		if (shape3d->getCollisionSize() > 10) {
			bounds->setShape3D(shape3d, 4); //4
		} else {
			bounds->setShape3D(shape3d, 0);
		}				
		group->setCollisionBounds(bounds);
	}
		
	// load shape
	if (strstr(shape, "?") == NULL) {
		Shape3D* shape3d = Shape3DUtil::loadShape3D(shape);
		if (shape3d == NULL) {
			throw string("File not found");
		}
		group->addShape3D(shape3d);
		// TODO remove
		shape3d->setProperty(EM_SHAPE3D_HIDDEN);
	}

	//
	newloadMisc(file, engine, group, NULL);

	return group;
}

int newloadFile(const char* fn, Engine * engine) {
	ifstream file(fn);
	try {
		if (!file) {
			throw string("Shape3DUtil::load() : file not found: ");
		}
		string str;
		Group * g = NULL;

		while (file >> str) {
			if (str == "object") {
				g = newloadStdObject(file, engine); 
				engine->add(g);
			}
		}
	} catch (string str) {
		cerr << str << endl;
	}
	return 0;
}

#undef CHECK_EOF
