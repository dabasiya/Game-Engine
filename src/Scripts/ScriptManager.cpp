#include "ScriptManager.h"

// scripts
#include "Player.h"
#include "Dice.h"
#include "CardHover.h"
#include "RollButton.h"
#include "CardCloseButton.h"


std::unordered_map<std::string, EntityScript* (*)()> scriptmanager::scriptmap;

void scriptmanager::Init() {
	registerscript<Player>("player");
	registerscript<Dice>("Dice");
	registerscript<CardHover>("cardtest");
	registerscript<RollButton>("RollButton");
	registerscript<CardCloseButton>("CardCloseButton");
}
