#pragma once

#include <Server/Game.hh>

#include <set>
#include <csignal>
#include <thread>
#include <barrier>

#ifdef WASM_SERVER
#include <emscripten/proxying.h>
#else
#include <Loop.h>
#endif

class Client;

size_t const MAX_PACKET_LEN = 64 * 1024;

#ifdef WASM_SERVER
class WebSocketServer {
public:
    WebSocketServer();
};
#endif

namespace Server {
    extern std::array<GameInstance, Gamemode::kNumGamemodes> games;
    extern std::array<std::atomic<uint32_t>, PetalID::kNumPetals> petal_count_tracker;
    #ifdef WASM_SERVER
    extern WebSocketServer server;
    #endif
    extern volatile sig_atomic_t is_draining;
    extern bool is_stopping;
    extern std::atomic<uint32_t> player_count;
    extern std::vector<std::thread> threads;
    extern std::barrier<> barrier;
    #ifdef WASM_SERVER
    extern emscripten::ProxyingQueue queue;
    #else
    extern uWS::Loop *loop;
    #endif
    extern void init();
    extern void run();
    extern void tick();
    extern void stop();
};