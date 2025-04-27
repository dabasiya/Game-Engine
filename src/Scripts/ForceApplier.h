#include "EntityScript.h"

#include "ContactListener.h"

struct ForceApplier : EntityScript {

        float time = 0;
        float loop_time = 4.0f;
        float smoke_emit_time = 0.3f;
        bool will_emit_smoke = false;

        Entity player;

        void Start() override {

                auto view = m_entity.m_scene->m_registry.view<NameComponent>();

                for(auto e : view) {
                        auto& nc = view.get<NameComponent>(e);
                        if(nc.name == "player") {
                                player = {e, m_entity.m_scene};
                                break;
                        }
                }
        }

        void Update(float ts) override {

                if(time > loop_time) {
                        auto& pg = GetComponent<ParticleGeneratorComponent>();
                        pg.active = true;
                        if(cl.onForce_Applier) {
                                auto& bc2d = player.GetComponent<RigidBody2DComponent>();
                                bc2d.body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, 0.1f), true);
                        }
                }

                if(time > (loop_time + smoke_emit_time)) {
                        time = 0.0f;
                        auto& pg = GetComponent<ParticleGeneratorComponent>();
                        pg.active = false;
                }
                
                time += ts;
        }
};
