#include <Server/Process.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

constexpr float CULL_EXTRA_RADIUS = 250;

void tick_culling_behavior(Simulation *sim, Entity &ent) {
    sim->spatial_hash.query(ent.get_camera_x(), ent.get_camera_y(), 960 / ent.get_fov() + CULL_EXTRA_RADIUS, 540 / ent.get_fov() + CULL_EXTRA_RADIUS, [](Simulation *, Entity &ent) {
        BitMath::unset(ent.flags, EntityFlags::kIsCulled);
    });
}
