#pragma once

#include <btBulletDynamicsCommon.h>

// PhysicsSystem.h (Minimal Definition)
class PhysicsSystem {
private:
    btDefaultCollisionConfiguration* m_collisionConfig;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_broadphase;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

public:
    PhysicsSystem();
    ~PhysicsSystem();
    void step(float deltaTime);
    btDiscreteDynamicsWorld* getWorld() { return m_dynamicsWorld; }
};
