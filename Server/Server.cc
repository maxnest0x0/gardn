#include <Server/Server.hh>

#include <Server/Game.hh>
#include <Server/Client.hh>

#include <Shared/Binary.hh>

#include <chrono>
#include <iostream>

static bool was_draining = false;

void tick_worker(GameInstance *game) {
    while (true) {
        Server::barrier.arrive_and_wait();
        if (Server::is_stopping) break;
        game->tick();
        Server::barrier.arrive_and_wait();
    }
}

namespace Server {
    std::array<GameInstance, Gamemode::kNumGamemodes> games = {
        GameInstance(Gamemode::kFFA),
        GameInstance(Gamemode::kTDM)
    };
    std::array<std::atomic<uint32_t>, PetalID::kNumPetals> petal_count_tracker = {0};
    volatile sig_atomic_t is_draining = false;
    bool is_stopping = false;
    std::atomic<uint32_t> player_count = 0;
    std::vector<std::thread> threads;
    std::barrier<> barrier(Gamemode::kNumGamemodes + 1);
}

using namespace Server;

void Server::init() {
    for (GameInstance &game : Server::games) {
        game.init();
        Server::threads.emplace_back(tick_worker, &game);
    }
    Server::run();
}

void Server::tick() {
    using namespace std::chrono_literals;
    auto start = std::chrono::steady_clock::now();
    Server::barrier.arrive_and_wait();
    Server::barrier.arrive_and_wait();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> tick_time = end - start;
    if (tick_time > 1000ms / TPS / 2) std::cout << "tick took " << tick_time << '\n';

    if (Server::is_draining && !was_draining) {
        was_draining = true;
        std::cout << "draining...\n";
    }
    if (Server::is_draining && !Server::is_stopping && Server::player_count == 0)
        Server::stop();
}
