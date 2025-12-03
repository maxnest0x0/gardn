#pragma once

#include <Shared/StaticData.hh>

class Simulation;

namespace PetalTracker {
    void add_petal(PetalID::T);
    void remove_petal(PetalID::T);
    uint32_t get_count(PetalID::T);
}