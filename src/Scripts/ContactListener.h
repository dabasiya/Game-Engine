#pragma once


#include <box2d/box2d.h>
#include "ObjectID.h"
#include <glm/glm.hpp>


#include <vector>

class ContactListener : public b2ContactListener {
public:

	// this is for checking player is grounded or not
	unsigned int count = 0;

	// check player collided with enemy
	// if it collided with enemy then player_die = true
	bool player_die = false;


	// for setting player spawn point this helps when player is died then player respawn at this point
	glm::vec2 player_respawn_position;

	bool onjumper = false;

	bool onForce_Applier = false;


	// for check enemy died or not
	std::vector<unsigned int> died_enemies_id;
	bool is_enemy_died = false;


	bool count_as_collision = false;

	b2Vec2 feetcollision_pos;

	bool countupcollider = false;
	b2Vec2 normal;

	float player_height = 0.0f;

	std::vector<unsigned int> collidedentities;

	bool touchinupcollider = false;

	bool player_entered_enter_option = false;

	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
		unsigned int id1 = contact->GetFixtureA()->GetUintData();
		unsigned int id2 = contact->GetFixtureB()->GetUintData();

		b2Vec2 velocity;
		if (id1 == objectid::Player)
			velocity = contact->GetFixtureA()->GetBody()->GetLinearVelocity();
		else
			velocity = contact->GetFixtureB()->GetBody()->GetLinearVelocity();


		if (isContactBetweenThis(contact, objectid::Player, objectid::Up_Collider)) {


			b2Vec2 ppos, cpos;

			touchinupcollider = false;
			countupcollider = true;

			if (id1 == objectid::Player) {
				ppos = contact->GetFixtureA()->GetBody()->GetPosition();
				cpos = contact->GetFixtureB()->GetBody()->GetPosition();
			}
			else {
				ppos = contact->GetFixtureB()->GetBody()->GetPosition();
				cpos = contact->GetFixtureA()->GetBody()->GetPosition();
			}

			float footpos = ppos.y - player_height / 2.0f;

			if (footpos < cpos.y) {
				contact->SetEnabled(false);
				countupcollider = false;
				touchinupcollider = true;
			}

		}

	}

	void BeginContact(b2Contact* contact) override {

		// if player in touch with force applier
		if (isContactBetweenThis(contact, objectid::Player, objectid::Force_Applier) && !player_die) {
			onForce_Applier = true;
		}

		if (isContactBetweenThis(contact, objectid::Player_Feet, objectid::Ground)) {


			unsigned int id1 = contact->GetFixtureA()->GetUintData();
			unsigned int id2 = contact->GetFixtureB()->GetUintData();

			bool isupcollider = false;

			unsigned int eid;
			if (id1 == objectid::Player_Feet) {
				eid = contact->GetFixtureB()->GetBody()->GetuintData();
			}
			else {
				eid = contact->GetFixtureA()->GetBody()->GetuintData();
			}

			if (id1 == objectid::Up_Collider)
				isupcollider = true;
			else if (id2 == objectid::Up_Collider)
				isupcollider = true;

			collidedentities.push_back(eid);

			count++;

		}

		else if (isContactBetweenThis(contact, objectid::Player, objectid::Up_Collider)) {
			unsigned int id1 = contact->GetFixtureA()->GetUintData();

			b2Vec2 vel;

			if (id1 == objectid::Player)
				vel = contact->GetFixtureA()->GetBody()->GetLinearVelocity();
			else
				vel = contact->GetFixtureB()->GetBody()->GetLinearVelocity();

			//if(countupcollider) {
			if (vel.y <= 0.0f)
				count++;
			//}


		}
		// for record which enemy killed by player so later enemy entity can be destroyed
		else if (isContactBetweenThis(contact, objectid::Player_Feet, objectid::Enemy) && !player_die) {
			is_enemy_died = true;
			died_enemies_id.push_back(GetEnemyID(contact));
		}

		// it is for test jumper this is remove in short time
		else if (isContactBetweenThis(contact, objectid::Player, objectid::Jumper)) {
			onjumper = true;
		}

		// if player body collided with enemy then player_die set true so player will be respawned
		else if (isContactBetweenThis(contact, objectid::Player, objectid::Enemy) && !is_enemy_died) {
			player_die = true;
		}

		// if player collided with this entity so player respawn point set to this position
		// so when player died it spawn at this position
		// by this approach i dont have to add level script for player spawn points
		else if (isContactBetweenThis(contact, objectid::Player, objectid::Respawn_Checkpoint)) {
			b2Vec2 position = (contact->GetFixtureA()->GetUintData() == objectid::Respawn_Checkpoint) ?
				contact->GetFixtureA()->GetBody()->GetPosition() :
				contact->GetFixtureB()->GetBody()->GetPosition();

			player_respawn_position = { position.x, position.y };
		}


		else if (isContactBetweenThis(contact, objectid::Player, objectid::Enter_Gate)) {
			player_entered_enter_option = true;
		}
	}

	void EndContact(b2Contact* contact) override {

		// ForceApplier and Player
		if (isContactBetweenThis(contact, objectid::Player, objectid::Force_Applier)) {
			onForce_Applier = false;
		}

		if (isContactBetweenThis(contact, objectid::Player_Feet, objectid::Ground)) {
			if (count > 0)
				count--;
		}

		else if (isContactBetweenThis(contact, objectid::Player, objectid::Up_Collider)) {
			//if(countupcollider)
			if (count > 0)
				count--;
		}

		else if (isContactBetweenThis(contact, objectid::Player, objectid::Jumper))
			onjumper = false;

		else if (isContactBetweenThis(contact, objectid::Player, objectid::Enter_Gate)) {
			player_entered_enter_option = false;
		}
	}

	bool isGrounded() {
		return count > 0;
	}

	bool isOnJumper() {
		return onjumper;
	}

	// check enemy is died or not by giving entity id as input
	bool isEnemydied(unsigned int id) {
		for (auto a : died_enemies_id) {
			if (a == id)
				return true;
		}
		return false;
	}

	// reset died enemies vector and boolean value which is store any enemy is died or not
	void reset_died_enemies_info() {
		died_enemies_id.clear();
		is_enemy_died = false;
	}

	void reset_player_collision() {
		collidedentities.clear();
		count = 0;
	}

private:

	bool isContactBetweenThis(b2Contact* contact, unsigned int i1, unsigned int i2) {
		unsigned int id1 = contact->GetFixtureA()->GetUintData();
		unsigned int id2 = contact->GetFixtureB()->GetUintData();

		if ((id1 == i1 && id2 == i2) ||
			(id2 == i1 && id1 == i2))
			return true;
		return false;
	}

	unsigned int GetEnemyID(b2Contact* contact) {
		unsigned int id1 = contact->GetFixtureA()->GetUintData();
		unsigned int id2 = contact->GetFixtureB()->GetUintData();

		if (id1 == objectid::Enemy)
			return contact->GetFixtureA()->GetBody()->GetuintData();
		else if (id2 == objectid::Enemy)
			return contact->GetFixtureB()->GetBody()->GetuintData();
	}



};


static ContactListener cl;
