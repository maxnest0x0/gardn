#include <Server/EntityFunctions.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

#include <cmath>
#include <iostream>

static bool _should_interact(Entity const &ent1, Entity const &ent2) {
    if (ent1.pending_delete || ent2.pending_delete) return false;
    if (!(ent1.get_team() == ent2.get_team())) return true;
    if (BitMath::at((ent1.flags | ent2.flags), EntityFlags::kNoFriendlyCollision)) return false;
    if ((ent1.has_component(kMob) || ent1.has_component(kFlower)) &&
        (ent2.has_component(kMob) || ent2.has_component(kFlower))) return true;
    return false;
}

static void _pickup_drop(Simulation *sim, Entity &player, Entity &drop) {
    if (!sim->ent_alive(player.get_parent())) return;
    if (drop.immunity_ticks > 0) return;

    for (uint32_t i = 0; i <  player.get_loadout_count() + MAX_SLOT_COUNT; ++i) {
        if (player.get_loadout_ids(i) != PetalID::kNone) continue;
        player.set_loadout_ids(i, drop.get_drop_id());
        drop.set_x(player.get_x());
        drop.set_y(player.get_y());
        BitMath::unset(drop.flags, EntityFlags::kIsDespawning);
        sim->request_delete(drop.id);
        //peaceful transfer, no petal tracking needed
        return;
    }
}

#define NO(component) (!ent1.has_component(component) && !ent2.has_component(component))
#define BOTH(component) (ent1.has_component(component) && ent2.has_component(component))
#define EITHER(component) (ent1.has_component(component) || ent2.has_component(component))

static void _deal_push(Entity &ent, Vector knockback, float mass_ratio, float scale) {
    if (fabsf(mass_ratio) < 0.01) return;
    knockback *= scale * mass_ratio;
    ent.collision_velocity += knockback;
}

static void _deal_knockback(Entity &ent, Vector knockback, float mass_ratio) {
    if (fabsf(mass_ratio) < 0.01) return;
    float scale = PLAYER_ACCELERATION * 2;
    knockback *= scale * mass_ratio;
    ent.collision_velocity += knockback;
    ent.velocity += knockback * 2;
}

static void _cancel_movement(Entity &ent, Vector dir, Vector add) {
    Vector push = dir;
    push.normalize();
    float dot = fclamp(push.x * add.x + push.y * add.y, PLAYER_ACCELERATION * 0.5, PLAYER_ACCELERATION * 25);
    ent.velocity += push * (PLAYER_ACCELERATION + dot * 2);
    ent.collision_velocity += push * (0.5 * PLAYER_ACCELERATION);
}

void on_collide(Simulation *sim, Entity &ent1, Entity &ent2) {
    //do a distance dependent check first (it's faster)
    float min_dist = ent1.get_radius() + ent2.get_radius();
    if (fabs(ent1.get_x() - ent2.get_x()) > min_dist || fabs(ent1.get_y() - ent2.get_y()) > min_dist) return;
    //check if collide (distance independent)
    if (!_should_interact(ent1, ent2)) return;
    //finer distance check
    Vector separation(ent1.get_x() - ent2.get_x(), ent1.get_y() - ent2.get_y());
    float dist = min_dist - separation.magnitude();
    if (dist < 0) return;
    if (NO(kDrop) && NO(kWeb) && NO(kChat)) {
        if (separation.x == 0 && separation.y == 0)
            separation.unit_normal(frand() * 2 * M_PI);
        else
            separation.normalize();
        float ratio = ent2.mass / (ent1.mass + ent2.mass);
        if (!(ent1.get_team() == ent2.get_team())) {
            if (ent1.has_component(kFlower) && !ent2.has_component(kPetal))
                _cancel_movement(ent1, separation, ent2.velocity - ent1.velocity);
            else
                _deal_knockback(ent1, separation, ratio);
            if (ent2.has_component(kFlower) && !ent1.has_component(kPetal))
                _cancel_movement(ent2, separation*-1, ent1.velocity - ent2.velocity);
            else
                _deal_knockback(ent2, separation*-1, 1 - ratio);
        } else {
            if (ent1.has_component(kFlower) && !ent1.has_component(kMob) && ent2.has_component(kMob))
                ratio = 0;
            if (ent2.has_component(kFlower) && !ent2.has_component(kMob) && ent1.has_component(kMob))
                ratio = 1;
        }
        _deal_push(ent1, separation, ratio, dist);
        _deal_push(ent2, separation*-1, 1 - ratio, dist);
    }

    if (BOTH(kHealth) && !(ent1.get_team() == ent2.get_team())) {
        if ((ent1.health > 0 || ent1.max_health == 0) && (ent2.health > 0 || ent2.max_health == 0)) {
            inflict_damage(sim, ent1.id, ent2.id, ent1.damage, DamageType::kContact);
            inflict_damage(sim, ent2.id, ent1.id, ent2.damage, DamageType::kContact);
        }
        if (ent1.health == 0 && ent1.max_health > 0) sim->request_delete(ent1.id);
        if (ent2.health == 0 && ent2.max_health > 0) sim->request_delete(ent2.id);
    }

    if (ent1.has_component(kDrop) && ent2.has_component(kFlower)) 
        _pickup_drop(sim, ent2, ent1);
    if (ent2.has_component(kDrop) && ent1.has_component(kFlower))
        _pickup_drop(sim, ent1, ent2);

    if (ent1.has_component(kWeb) && !ent2.has_component(kPetal) && !ent2.has_component(kDrop))
        ent2.speed_ratio = 0.5;
    if (ent2.has_component(kWeb) && !ent1.has_component(kPetal) && !ent1.has_component(kDrop))
        ent1.speed_ratio = 0.5;
}