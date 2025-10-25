#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>

#include <Client/Particle.hh>

#include <Client/Ui/Ui.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cmath>

void render_petal(Renderer &ctx, Entity const &ent) {
    ctx.scale(ent.get_radius() / PETAL_DATA[ent.get_petal_id()].radius);
    PetalRenderAttributes attrs = {ent.animation, ent.special_animation, 1 << PetalRenderFlags::kAnimated};
    if (BitMath::at(ent.get_petal_flags(), PetalFlags::kLockedOn))
        BitMath::set(attrs.flags, PetalRenderFlags::kSpecial);
    if (BitMath::at(ent.get_petal_flags(), PetalFlags::kSplitProjectile))
        draw_static_petal(ent.get_petal_id(), ctx, attrs);
    else draw_static_petal_single(ent.get_petal_id(), ctx, attrs);
    if (PETAL_DATA[ent.get_petal_id()].rarity == RarityID::kUnique && frand() < fclamp(0.25 * Ui::dt/16.67, 0, 1))
        Particle::add_unique_particle(ent.get_x(), ent.get_y());
}