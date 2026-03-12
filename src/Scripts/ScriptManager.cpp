#include "ScriptManager.h"

// scripts
#include "Player.h"
#include "Dice.h"
#include "CardHover.h"
#include "RollButton.h"
#include "CardCloseButton.h"


#include "Cube.h"
#include "Character.h"
#include "PlayerCamera.h"
#include "FPS.h"
#include "Pistol.h"
#include "Door.h"
#include "Enemy.h"

std::unordered_map<std::string, EntityScript* (*)()> scriptmanager::scriptmap;

void scriptmanager::Init() {
	registerscript<Player>("player");
	registerscript<Dice>("Dice");
	registerscript<CardHover>("cardtest");
	registerscript<RollButton>("RollButton");
	registerscript<CardCloseButton>("CardCloseButton");
	registerscript<Cube>("Cube");
	registerscript<CharacterEntity>("Character");
	registerscript<PlayerCamera>("PlayerCamera");
	registerscript<FPS>("fps");
	registerscript<Pistol>("pistol");
	registerscript<Door>("Door");
	registerscript<Enemy>("Enemy");
}
