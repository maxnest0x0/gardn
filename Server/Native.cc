#ifndef WASM_SERVER
#include <Server/Server.hh>

#include <Server/Client.hh>
#include <Shared/Config.hh>

static us_listen_socket_t *socket;
static struct us_timer_t *delayTimer;
static int curr_id = 0;
std::unordered_map<int, WebSocket *> WS_MAP;
uWS::Loop *Server::loop;

static uWS::App make_server() {
    return uWS::App().ws<Client>("/*", {
        /* Settings */
        .compression = uWS::DISABLED,
        .maxPayloadLength = 1024,
        .idleTimeout = 15,
        .maxBackpressure = 1024 * MAX_PACKET_LEN,
        .closeOnBackpressureLimit = true,
        .resetIdleTimeoutOnSend = false,
        .sendPingsAutomatically = true,
        /* Handlers */
        .upgrade = nullptr,
        .open = [](WebSocket *ws) {
            std::cout << "client connection\n";
            Client *client = ws->getUserData();
            client->ws = ws;
            client->ws_id = curr_id++;
            WS_MAP.insert({client->ws_id, ws});
        },
        .message = [](WebSocket *ws, std::string_view message, uWS::OpCode opCode) {
            Client::on_message(ws, message, opCode);
        },
        .dropped = [](WebSocket *ws, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
            std::cout << "dropped packet\n";
            Client *client = ws->getUserData();
            if (client == nullptr) {
                ws->end(1006, "Dropped Message");
                return;
            }
            client->disconnect();
            /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
        },
        .drain = [](WebSocket */*ws*/) {
            //assert(!1);
            /* Check ws->getBufferedAmount() here */
        },
        .close = [](WebSocket *ws, int code, std::string_view message) {
            Client::on_disconnect(ws, code, message);
            Client *client = ws->getUserData();
            if (client == nullptr) return;
            WS_MAP.erase(client->ws_id);
        }
    }).listen(SERVER_PORT, [](auto *listen_socket) {
        assert(listen_socket);
        socket = listen_socket;
        std::cout << "Listening on port " << SERVER_PORT << std::endl;
    });
}

void Server::run() {
    struct sigaction sa;
    sa.sa_handler = [](int signum){
        Server::is_draining = true;
    };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    assert(sigaction(SIGUSR2, &sa, nullptr) == 0);
    std::atexit([](){
        std::cout << "exiting...\n";
    });

    Server::loop = uWS::Loop::get();
    delayTimer = us_create_timer((struct us_loop_t *) Server::loop, 0, 0);
    us_timer_set(delayTimer, [](us_timer_t *x){ Server::tick(); }, 1, 1000 / TPS);

    uWS::App server = make_server();
    server.run();
}

void Server::stop() {
    Server::is_stopping = true;
    std::cout << "stopping...\n";
    us_listen_socket_close(0, socket);
    for (auto &p : WS_MAP)
        p.second->end(1001, "Shutting Down");
    us_timer_close(delayTimer);
    Server::barrier.arrive_and_wait();
    for (std::thread &thread : Server::threads)
        thread.join();
}

void Client::send_packet(uint8_t const *packet, size_t size) {
    int ws_id = this->ws_id;
    Server::loop->defer([=](){
        auto iter = WS_MAP.find(ws_id);
        if (iter == WS_MAP.end()) return;
        std::string_view message(reinterpret_cast<char const *>(packet), size);
        iter->second->send(message, uWS::OpCode::BINARY, 0);
        delete[] packet;
    });
}
#endif