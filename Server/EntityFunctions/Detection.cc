#include <Server/EntityFunctions.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

EntityID find_nearest_enemy(Simulation *simulation, Entity const &entity, float radius) {
    if ((entity.id.id - entity.lifetime) % (TPS / 5) != 0) return NULL_ENTITY;
    if (entity.immunity_ticks > 0) return NULL_ENTITY;
    EntityID ret;
    float min_dist = radius;
    simulation->spatial_hash.query(entity.get_x(), entity.get_y(), radius, radius, [&](Simulation *sim, Entity &ent){
        if (!sim->ent_alive(ent.id)) return;
        if (ent.get_team() == entity.get_team()) return;
        if (ent.immunity_ticks > 0) return;
        if (!ent.has_component(kMob) && !ent.has_component(kFlower)) return;
        if (sim->ent_alive(entity.get_parent())) {
            Entity &parent = sim->get_ent(entity.get_parent());
            float dist = Vector(ent.get_x()-parent.get_x(),ent.get_y()-parent.get_y()).magnitude();
            if (dist > SUMMON_RETREAT_RADIUS) return;
        }
        float dist = Vector(ent.get_x()-entity.get_x(),ent.get_y()-entity.get_y()).magnitude();
        if (dist < min_dist) { min_dist = dist; ret = ent.id; }
    });
    return ret;
}

EntityID find_nearest_enemy_within_angle(Simulation *simulation, Entity const &entity, float radius, float angle) {
    EntityID ret;
    float min_dist = radius;
    simulation->spatial_hash.query(entity.get_x(), entity.get_y(), radius, radius, [&](Simulation *sim, Entity &ent){
        if (!sim->ent_alive(ent.id)) return;
        if (ent.get_team() == entity.get_team()) return;
        if (ent.immunity_ticks > 0) return;
        if (!ent.has_component(kMob) && !ent.has_component(kFlower)) return;
        Vector v(ent.get_x()-entity.get_x(),ent.get_y()-entity.get_y());
        float dist = v.magnitude();
        if (dist < min_dist && angle_within(entity.get_angle(), v.angle(), angle)) {
            min_dist = dist;
            ret = ent.id;
        }
    });
    return ret;
}

EntityID find_teammate_to_heal(Simulation *simulation, Entity const &entity, float radius) {
    EntityID ret;
    float min_health_ratio = 1;
    simulation->spatial_hash.query(entity.get_x(), entity.get_y(), radius, radius, [&](Simulation *sim, Entity &ent){
        if (!sim->ent_alive(ent.id)) return;
        if (ent.get_team() != entity.get_team()) return;
        if (ent.dandy_ticks > 0) return;
        if (!ent.has_component(kFlower)) return;
        if (BitMath::at(ent.flags, EntityFlags::kZombie)) return;
        float health_ratio = ent.health / ent.max_health;
        if (health_ratio < min_health_ratio) {
            min_health_ratio = health_ratio;
            ret = ent.id;
        }
    });
    return ret;
}
