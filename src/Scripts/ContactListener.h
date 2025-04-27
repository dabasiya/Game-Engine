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

        
	b2Vec2 normal;

	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override {
		unsigned int id1 = contact->GetFixtureA()->GetUintData();
		unsigned int id2 = contact->GetFixtureB()->GetUintData();


		b2WorldManifold wmf;
		contact->GetWorldManifold(&wmf);

		b2Vec2 normal = wmf.normal;

		if ((id1 == objectid::Player && id2 == objectid::Up_Collider) || (id2 == objectid::Player && id1 == objectid::Up_Collider)) {
			if (normal.y < 0.0f || !isGrounded())
				contact->SetEnabled(false);
		}
	}

	void BeginContact(b2Contact* contact) override {


                // for setting player is grounded or not so if player grounded then only player can jump
		if ((isContactBetweenThis(contact, objectid::Player_Feet, objectid::Ground) ||
                    isContactBetweenThis(contact, objectid::Player_Feet, objectid::Up_Collider)) && !player_die) {
			b2Vec2 velocity = (contact->GetFixtureA()->GetUintData() == objectid::Player_Feet) ?
				contact->GetFixtureA()->GetBody()->GetLinearVelocity() :
				contact->GetFixtureB()->GetBody()->GetLinearVelocity();
			if (velocity.y <= 0.0f || isGrounded()) {
				count++;
			}
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
                else if(isContactBetweenThis(contact, objectid::Player, objectid::Respawn_Checkpoint)) {
                        b2Vec2 position = (contact->GetFixtureA()->GetUintData() == objectid::Respawn_Checkpoint) ?
                                contact->GetFixtureA()->GetBody()->GetPosition() :
                                contact->GetFixtureB()->GetBody()->GetPosition();

                        player_respawn_position = {position.x, position.y};
                }

                // if player in touch with force applier
                else if(isContactBetweenThis(contact, objectid::Player, objectid::Force_Applier) && !player_die) {
                        onForce_Applier = true;
                }
	}

	void EndContact(b2Contact* contact) override {

		if (isContactBetweenThis(contact, objectid::Player_Feet, objectid::Ground) ||
			isContactBetweenThis(contact, objectid::Player_Feet, objectid::Up_Collider)) {
			if (count > 0)
				count--;
		}

		else if (isContactBetweenThis(contact, objectid::Player, objectid::Jumper))
			onjumper = false;

                else if(isContactBetweenThis(contact, objectid::Player, objectid::Force_Applier)) {
                        onForce_Applier = false;
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
                for(auto a : died_enemies_id) {
                        if(a == id)
                                return true;
                }
                return false;
        }

        // reset died enemies vector and boolean value which is store any enemy is died or not
        void reset_died_enemies_info() {
                died_enemies_id.clear();
                is_enemy_died = false;
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
