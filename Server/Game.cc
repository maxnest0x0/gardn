#include <Server/Game.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Binary.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>

static void _update_client(Simulation *sim, Client *client) {
    if (client == nullptr) return;
    if (!client->verified) return;
    if (sim == nullptr) return;
    if (!sim->ent_exists(client->camera)) return;
    std::set<EntityID> in_view;
    std::vector<EntityID> deletes;
    in_view.insert(client->camera);
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_exists(camera.get_player())) 
        in_view.insert(camera.get_player());
    if (sim->arena_info.gamemode == Gamemode::kTDM) {
        Entity &team = sim->get_ent(camera.get_team());
        in_view.insert(team.minimap_dots.begin(), team.minimap_dots.end());
    }
    for (EntityID dot_id : sim->arena_info.leader_dots) {
        if (sim->get_ent(dot_id).get_team() != camera.get_team())
            in_view.insert(dot_id);
    }
    Writer writer(new uint8_t[MAX_PACKET_LEN]);
    writer.write<uint8_t>(Clientbound::kClientUpdate);
    writer.write<uint8_t>(client->seen_arena);
    writer.write<uint8_t>(Server::is_draining);
    writer.write<EntityID>(client->camera);
    sim->spatial_hash.query(camera.get_camera_x(), camera.get_camera_y(), 
    960 / camera.get_fov() + 100, 540 / camera.get_fov() + 100, 
    [&](Simulation *, Entity &ent){
        in_view.insert(ent.id);
        if (ent.has_component(kSegmented) && ent.has_component(kAnimation)) {
            if (sim->ent_exists(ent.get_seg_head())) in_view.insert(ent.get_seg_head());
            if (sim->ent_exists(ent.get_seg_tail())) in_view.insert(ent.get_seg_tail());
        }
    });

    for (EntityID const &i: client->in_view) {
        if (!in_view.contains(i)) {
            writer.write<EntityID>(i);
            deletes.push_back(i);
        }
    }

    for (EntityID const &i : deletes)
        client->in_view.erase(i);

    writer.write<EntityID>(NULL_ENTITY);
    //upcreates
    for (EntityID id: in_view) {
        DEBUG_ONLY(assert(sim->ent_exists(id));)
        Entity &ent = sim->get_ent(id);
        uint8_t create = !client->in_view.contains(id);
        writer.write<EntityID>(id);
        writer.write<uint8_t>(create | (ent.pending_delete << 1));
        ent.write(&writer, BitMath::at(create, 0));
        client->in_view.insert(id);
    }
    writer.write<EntityID>(NULL_ENTITY);
    //write arena stuff
    sim->arena_info.write(&writer, !client->seen_arena);
    client->seen_arena = 1;
    assert(writer.at - writer.packet <= MAX_PACKET_LEN);
    client->send_packet(writer.packet, writer.at - writer.packet);
}

GameInstance::GameInstance(uint8_t mode) : simulation(), clients(), team_manager(&simulation), gamemode(mode) {}

void GameInstance::init() {
    simulation.arena_info.set_gamemode(gamemode);
    for (uint32_t i = 0; i < ENTITY_CAP / 2; ++i)
        Map::spawn_random_mob(&simulation, frand() * ARENA_WIDTH, frand() * ARENA_HEIGHT);
    if (gamemode == Gamemode::kTDM) {
        team_manager.add_team(ColorID::kBlue);
        team_manager.add_team(ColorID::kRed);
    }
}

void GameInstance::tick() {
    simulation.tick();
    if (gamemode == Gamemode::kTDM)
        team_manager.tick();
    for (Client *client : clients)
        _update_client(&simulation, client);
    simulation.post_tick();
}

void GameInstance::add_client(Client *client, EntityID camera_id) {
    DEBUG_ONLY(assert(client->game != this);)
    if (client->game != nullptr)
        client->game->remove_client(client);
    client->game = this;
    clients.insert(client);
    if (camera_id == NULL_ENTITY) {
        EntityID team;
        if (gamemode == Gamemode::kTDM)
            team = team_manager.get_random_team();
        client->camera = alloc_camera(&simulation, team).id;
    } else
        client->camera = camera_id;
    Entity &camera = simulation.get_ent(client->camera);
    if (camera.client != nullptr)
        camera.client->disconnect(CloseReason::kRecovered, "Session Recovered");
    camera.client = client;
    BitMath::unset(camera.flags, EntityFlags::kZombie);
    BitMath::unset(camera.flags, EntityFlags::kIsDespawning);
}

void GameInstance::remove_client(Client *client) {
    DEBUG_ONLY(assert(client->game == this);)
    clients.erase(client);
    if (simulation.ent_exists(client->camera)) {
        Entity &camera = simulation.get_ent(client->camera);
        DEBUG_ONLY(simulation.request_delete(camera.id);)
        BitMath::set(camera.flags, EntityFlags::kZombie);
        camera.client = nullptr;
    }
    client->game = nullptr;
}