#include "ScriptManager.h"

// scripts
#include "MainMenu.h"
#include "Player.h"
#include "PlayerFollowCamera.h"
#include "parallax_bg_moving.h"
#include "enemytest.h"
#include "jumper.h"
#include "EnemyGenerator.h"
#include "movingpgtest.h"
#include "ForceApplier.h"

std::unordered_map<std::string, EntityScript* (*)()> scriptmanager::scriptmap;

void scriptmanager::Init() {
	registerscript<MainMenu>("main_menu");
	registerscript<player>("player");
	registerscript<playerfollowcamera>("playerfollowcamera");
	registerscript<parallax_bg_moving>("parallax_bg_moving");
	registerscript<enemytest>("enemytest");
	registerscript<jumper>("jumper");
	registerscript<EnemyGenerator>("EnemyGenerator");
	registerscript<movingpgtest>("movingpgtest");
        registerscript<ForceApplier>("ForceApplier");
}
