#include <Server/PetalTracker.hh>

#include <Server/Server.hh>

#include <Shared/Simulation.hh>

using namespace PetalTracker;

void PetalTracker::add_petal(PetalID::T id) {
    DEBUG_ONLY(assert(id < PetalID::kNumPetals);)
    if (id == PetalID::kNone) return;
    ++Server::petal_count_tracker[id];
}

void PetalTracker::remove_petal(PetalID::T id) {
    DEBUG_ONLY(assert(id < PetalID::kNumPetals);)
    if (id == PetalID::kNone) return;
    --Server::petal_count_tracker[id];
}

uint32_t PetalTracker::get_count(PetalID::T id) {
    DEBUG_ONLY(assert(id < PetalID::kNumPetals);)
    if (id == PetalID::kNone) return 0;
    return Server::petal_count_tracker[id];
}
