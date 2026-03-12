#include "PhysicsSystem.h"


// PhysicsSystem.cpp (Minimal Implementation)

// Standard Bullet boilerplate setup
PhysicsSystem::PhysicsSystem() {
    m_collisionConfig = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfig);
    m_broadphase = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(
        m_dispatcher, m_broadphase, m_solver, m_collisionConfig
    );
    m_dynamicsWorld->setGravity(btVector3(0, -9.8f, 0)); // Earth gravity
}

PhysicsSystem::~PhysicsSystem() {
    // Cleanup in reverse order of creation
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_broadphase;
    delete m_dispatcher;
    delete m_collisionConfig;
}

// The core function called by the game loop
void PhysicsSystem::step(float deltaTime) {
    // Sub-steps used for stability (e.g., 10 iterations max)
    m_dynamicsWorld->stepSimulation(deltaTime, 10);
}