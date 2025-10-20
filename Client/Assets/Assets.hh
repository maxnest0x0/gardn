#pragma once

#include <Client/Render/Renderer.hh>

#include <Shared/StaticData.hh>
#include <stdint.h>

struct FlowerRenderAttributes {
    float radius = 25;
    float eye_x = 2;
    float eye_y = 0;
    float mouth = 15;
    float cutter_angle = 0;
    uint8_t face_flags = 0;
    uint8_t equip_flags = 0;
    uint8_t color = 0;
};

struct MobRenderAttributes {
    float animation;
    float radius;
    uint32_t seed;
    uint32_t flags;
    uint8_t color;
    FlowerRenderAttributes flower_attrs;
};

struct PetalRenderAttributes {
    float animation;
    float special_animation;
    uint32_t flags;
    float eye_x;
    float eye_y;
};

namespace MobRenderFlags {
    enum {
        kFriendly,
        kBodySegment
    };
};

namespace PetalRenderFlags {
    enum {
        kEquipped,
        kAnimated,
        kSpecial
    };
};

void draw_loadout_background(Renderer &, PetalID::T, float=1);

void draw_static_flower(Renderer &, FlowerRenderAttributes);

void draw_static_petal_single(PetalID::T, Renderer &, PetalRenderAttributes);

void draw_static_petal(PetalID::T, Renderer &, PetalRenderAttributes);

void draw_static_mob(MobID::T, Renderer &, MobRenderAttributes);

void draw_web(Renderer &);
