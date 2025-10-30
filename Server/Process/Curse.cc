#include <Server/Process.hh>

#include <Server/Spawn.hh>

#include <Shared/Simulation.hh>

#include <Shared/StaticData.hh>

void tick_curse_behavior(Simulation *sim) {
    EntityID &leader_dot = sim->arena_info.leader_dot;
    EntityID old_leader_dot = leader_dot;
    leader_dot = NULL_ENTITY;
    std::vector<EntityID> &leader_dots = sim->arena_info.leader_dots;
    for (size_t i = leader_dots.size(); i > 0; --i) {
        if (!sim->ent_exists(leader_dots[i - 1])) {
            leader_dots[i - 1] = leader_dots.back();
            leader_dots.pop_back();
        }
    }
    EntityID leader = NULL_ENTITY;
    uint32_t max_score = level_to_score(60);
    sim->for_each<kCamera>([&](Simulation *sim, Entity &ent) {
        if (!sim->ent_alive(ent.get_player())) return;
        uint32_t score = sim->get_ent(ent.get_player()).get_score();
        if (score < max_score) return;
        max_score = score;
        leader = ent.get_player();
    });
    if (sim->ent_alive(leader)) {
        Entity &player = sim->get_ent(leader);
        sim->for_each<kMob>([&](Simulation *sim, Entity &ent) {
            if (!(ent.get_team() == NULL_ENTITY)) return;
            if (MOB_DATA[ent.get_mob_id()].attributes.hole) return;
            if (sim->ent_alive(ent.target)) return;
            Vector delta(player.get_x() - ent.get_x(), player.get_y() - ent.get_y());
            if (delta.magnitude() > ent.detection_radius * 2) return;
            ent.target = leader;
        });
        if (max_score >= level_to_score(75)) {
            if (sim->ent_alive(old_leader_dot) && sim->get_ent(old_leader_dot).get_parent() == player.id)
                leader_dot = old_leader_dot;
            else {
                Entity &dot = alloc_dot(sim, player);
                dot.set_color(ColorID::kGray);
                leader_dot = dot.id;
                leader_dots.push_back(dot.id);
            }
            Entity &dot = sim->get_ent(leader_dot);
            dot.set_x(player.get_x());
            dot.set_y(player.get_y());
        }
        if (max_score >= level_to_score(90)) {
            float accel = player.acceleration.magnitude() / PLAYER_ACCELERATION;
            float dmg = player.max_health / (60 * TPS) * fclamp(accel, 0, 1);
            player.health = fclamp(player.health - dmg, 0, player.max_health);
        }
    }
    if (sim->ent_alive(old_leader_dot) && old_leader_dot != leader_dot)
        sim->request_delete(old_leader_dot);
}