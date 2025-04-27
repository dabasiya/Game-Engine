#pragma once


typedef enum {
	Player = 1,
	Ground = 2,
	Enemy = 3,
	Player_Feet = 4,
	Jumper = 5,
	Up_Collider = 6, // in this type collision will happen if player up on this object
        Respawn_Checkpoint = 7, // for set respawn checkpoint when player died it respawn at this point
        Force_Applier = 8
} objectid;

 

