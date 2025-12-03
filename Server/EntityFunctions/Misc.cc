#include <Server/EntityFunctions.hh>

#include <Server/PetalTracker.hh>

#include <Shared/Entity.hh>
#include <Shared/Simulation.hh>

#include <type_traits>

void entity_set_despawn_tick(Entity &ent, game_tick_t t) {
    ent.despawn_tick = t;
    BitMath::set(ent.flags, EntityFlags::kIsDespawning);
}

template<typename T, typename U>
class FilterCast {
public:
    static T const &get(U const &);
    static U const &set(T const &);
};

template<>
class FilterCast<EntityID, EntityID> {
public:
    static EntityID const &get(EntityID const &v) { return v; }
    static EntityID const &set(EntityID const &v) { return v; }
};

void entity_clear_references(Simulation *sim, Entity &ent) {
#define SINGLE(component, name, type) \
if constexpr (std::is_same_v<type, EntityID>) { \
    if (ent.has_component(k##component) && !sim->ent_exists(FilterCast<EntityID, type>::get(ent.get_##name()))) \
        ent.set_##name(FilterCast<type, EntityID>::get(NULL_ENTITY)); \
}
#define MULTIPLE(component, name, type, count)
PERFIELD
#undef SINGLE
#undef MULTIPLE
#define SINGLE(name, type, default) \
if constexpr (std::is_same_v<type, EntityID>) { \
    if (!sim->ent_exists(FilterCast<EntityID, type>::get(ent.name))) \
        ent.name = FilterCast<type, EntityID>::get(NULL_ENTITY); \
}
#define MULTIPLE(name, type, count, default)
PER_EXTRA_FIELD
#undef SINGLE
#undef MULTIPLE
}

constexpr std::array<uint32_t, RarityID::kNumRarities> RARITY_TO_XP = { 2, 10, 50, 200, 1000, 5000, 0 };

void delete_petal(Simulation *simulation, Entity &player, PetalID::T old_id) {
    if (old_id != PetalID::kNone && old_id != PetalID::kBasic) {
        uint8_t rarity = PETAL_DATA[old_id].rarity;
        player.set_score(player.get_score() + RARITY_TO_XP[rarity]);
        //need to delete if over cap
        if (player.deleted_petals.size() == player.deleted_petals.capacity())
            //removes old trashed petal
            PetalTracker::remove_petal(player.deleted_petals[0]);
        player.deleted_petals.push_back(old_id);
    }
}
