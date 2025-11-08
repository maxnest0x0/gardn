#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>

#include <Client/Game.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cmath>

void render_drop(Renderer &ctx, Entity const &ent) {
    if (ent.get_picked_up_by() != NULL_ENTITY && ent.get_picked_up_by() == Game::player_id) {
        if (!Ui::UiLoadout::deleted_drops.contains(ent.id)) {
            Ui::UiLoadoutDrop *elt = new Ui::UiLoadoutDrop(ent.get_drop_id());
            Ui::UiLoadout::deleted_drops[ent.id] = elt;
            Game::game_ui_window.add_child(elt);
        }
    }
    float animation_value = sinf(Game::timestamp / 100);
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.rotate(M_PI * 2 * ent.deletion_animation);
    ctx.scale(1 - ent.deletion_animation);
    ctx.scale(1 + animation_value * 0.03);
    ctx.scale(ent.get_radius() / 30);
    ctx.set_fill(0x40000000);
    ctx.begin_path();
    ctx.round_rect(-33, -33, 66, 66, 4);
    ctx.fill();
    draw_loadout_background(ctx, ent.get_drop_id());
}