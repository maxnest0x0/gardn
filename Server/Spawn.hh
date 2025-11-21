#pragma once

#include <Shared/Entity.hh>

class Simulation;

Entity &alloc_drop(Simulation *, PetalID::T);
Entity &alloc_mob(Simulation *, MobID::T, float, float, EntityID const, game_tick_t = 0);
Entity &alloc_player(Simulation *, EntityID const);
Entity &alloc_petal(Simulation *, PetalID::T, Entity const &);
Entity &alloc_web(Simulation *, float, Entity const &);
Entity &alloc_chat(Simulation *, std::string &, Entity const &);
Entity &alloc_dot(Simulation *, Entity const &);
Entity &alloc_camera(Simulation *, EntityID const);
Entity &alloc_cpu_camera(Simulation *, EntityID const);

void player_spawn(Simulation *, Entity &, Entity &);