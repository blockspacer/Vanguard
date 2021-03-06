#include "ingameelements/heroes/mccree.h"
#include "datastructures.h"
#include "spriteloader.h"
#include "animation.h"
#include "gamestate.h"
#include "engine.h"
#include "colorpalette.h"
#include "ingameelements/projectiles/flashbang.h"

#include <memory>
#include <cmath>
#include <allegro5/allegro_primitives.h>

void Mccree::init(uint64_t id_, Gamestate &state, EntityPtr owner_)
{
    Character::init(id_, state, owner_);

    rollanim.init(herofolder()+"roll/", false);
    flashbanganim.init(herofolder()+"flashbang/", false);
    ultwalkanim.init(herofolder()+"ultwalk/", true);
    fallanim.init(herofolder()+"falling/", true);

    rollcooldown.init(8, false);
    flashbangcooldown.init(10, false);
    ulting.init(6, std::bind(&Mccree::resetafterult, this, std::placeholders::_1), false);
    ultcooldown.init(0.5, false);
}

void Mccree::render(Renderer &renderer, Gamestate &state)
{
    Character::render(renderer, state);

    std::string spritepath;
    sf::Sprite sprite;

    if (isflipped)
    {
        sprite.setScale(-1, 1);
    }
    sprite.setPosition(x, y);

    if (flashbanganim.active())
    {
        spritepath = flashbanganim.getframepath();
        renderer.spriteloader.loadsprite(spritepath, sprite);
        renderer.midground.draw(sprite);
    }

    spritepath = currentsprite(state, false);
    renderer.spriteloader.loadsprite(spritepath, sprite);
    renderer.midground.draw(sprite);

    if (state.get<Player&>(renderer.myself).team != team)
    {
        sprite.setColor(COLOR_ENEMY_OUTLINE);
        renderer.spriteloader.loadspriteoutline(spritepath, sprite);
        renderer.midground.draw(sprite);
        sprite.setColor(sf::Color::White);
    }

    state.get<Weapon>(weapon).render(renderer, state);
}

void Mccree::beginstep(Gamestate &state, double frametime)
{
    Character::beginstep(state, frametime);

    rollanim.update(state, frametime);
    if (rollanim.active())
    {
        if (isflipped)
        {
            hspeed = -360;
        }
        else
        {
            hspeed = 360;
        }
    }
    flashbanganim.update(state, frametime);
    rollcooldown.update(state, frametime);
    flashbangcooldown.update(state, frametime);
    ulting.update(state, frametime);
    ultcooldown.update(state, frametime);

    fallanim.update(state, vspeed*frametime);

    if (isflipped)
    {
        ultwalkanim.update(state, -hspeed*frametime);
    }
    else
    {
        ultwalkanim.update(state, hspeed*frametime);
    }

    if (canuseabilities(state))
    {
        if (heldkeys.ABILITY_1 and not rollcooldown.active and onground(state) and state.engine.isserver)
        {
            useability1(state);
            state.engine.sendbuffer.write<uint8_t>(ABILITY1_USED);
            state.engine.sendbuffer.write<uint8_t>(state.findplayerid(owner));
        }
        if (heldkeys.ABILITY_2 and not flashbangcooldown.active and state.engine.isserver)
        {
            useability2(state);
            state.engine.sendbuffer.write<uint8_t>(ABILITY2_USED);
            state.engine.sendbuffer.write<uint8_t>(state.findplayerid(owner));
        }
    }

    if (heldkeys.PRIMARY_FIRE and ulting.active and state.engine.isserver)
    {
        state.engine.sendbuffer.write<uint8_t>(ULTIMATE_USED);
        state.engine.sendbuffer.write<uint8_t>(state.findplayerid(owner));
        Peacemaker &w = state.get<Peacemaker>(weapon);
        w.fireultimate(state);
    }
}

void Mccree::interpolate(Entity &prev_entity, Entity &next_entity, double alpha)
{
    Character::interpolate(prev_entity, next_entity, alpha);

    Mccree &p = static_cast<Mccree&>(prev_entity);
    Mccree &n = static_cast<Mccree&>(next_entity);

    rollanim.interpolate(p.rollanim, n.rollanim, alpha);
    rollcooldown.interpolate(p.rollcooldown, n.rollcooldown, alpha);
    flashbanganim.interpolate(p.flashbanganim, n.flashbanganim, alpha);
    flashbangcooldown.interpolate(p.flashbangcooldown, n.flashbangcooldown, alpha);
    ulting.interpolate(p.ulting, n.ulting, alpha);
    ultwalkanim.interpolate(p.ultwalkanim, n.ultwalkanim, alpha);
    ultcooldown.interpolate(p.ultcooldown, n.ultcooldown, alpha);
    fallanim.interpolate(p.fallanim, n.fallanim, alpha);
}

void Mccree::useability1(Gamestate &state)
{
    if (heldkeys.LEFT and not heldkeys.RIGHT)
    {
        isflipped = true;
    }
    else if (heldkeys.RIGHT and not heldkeys.LEFT)
    {
        isflipped = false;
    }
    // Lets roll
    rollanim.reset();
    rollcooldown.reset();
    Peacemaker &w = reinterpret_cast<Peacemaker&>(getweapon(state));
    w.clip = w.getclipsize();
    w.reloadanim.active(0);
    w.isfthing = false;
    w.fthanim.active(false);
    vspeed = 0;
}

void Mccree::useability2(Gamestate &state)
{
    // Flashbang
    flashbanganim.reset();
    flashbangcooldown.reset();
    Flashbang &f = state.get<Flashbang>(state.make_entity<Flashbang>(state, owner));
    f.x = x;
    f.y = y;
    double dir = std::atan2(mouse_y-y, mouse_x-x);
    f.hspeed = std::cos(dir) * 300;
    f.vspeed = std::sin(dir) * 300;
}

void Mccree::useultimate(Gamestate &state)
{
    if (ulting.active)
    {
        // We are already ulting and now want to fire
        Peacemaker &p = state.get<Peacemaker>(weapon);
        p.fireultimate(state);
    }
    else
    {
        // Start charging
        ulting.reset();
        state.get<Peacemaker>(weapon).deadeyetargets.clear();
    }
}

void Mccree::resetafterult(Gamestate &state)
{
    ulting.active = false;
    ultwalkanim.reset();
    ultcooldown.reset();
    Player &ownerplayer = state.get<Player>(owner);
    ownerplayer.ultcharge.active = true;
    Peacemaker &w = state.get<Peacemaker>(weapon);
    w.isfiringult = false;
    w.deadeyeanim.active(false);
    w.clip = w.getclipsize();
}

void Mccree::interrupt(Gamestate &state)
{
    if (ulting.active)
    {
        resetafterult(state);
    }
    rollanim.active(false);
}

Rect Mccree::getcollisionrect(Gamestate &state)
{
    if (crouchanim.active())
    {
        return state.engine.maskloader.get_json_rect(herofolder()+"crouch/").offset(x, y);
    }
    return getstandingcollisionrect(state);
}

Rect Mccree::getstandingcollisionrect(Gamestate &state)
{
    return state.engine.maskloader.get_json_rect(herofolder()).offset(x, y);
}

std::string Mccree::currentsprite(Gamestate &state, bool mask)
{
    if (pinanim.active())
    {
        return pinanim.getframepath();
    }
    if (stunanim.active())
    {
        return stunanim.getframepath();
    }
    if (earthshatteredfallanim.active())
    {
        return earthshatteredfallanim.getframepath();
    }
    if (earthshatteredanim.active())
    {
        return earthshatteredanim.getframepath();
    }
    if (earthshatteredgetupanim.active())
    {
        return earthshatteredgetupanim.getframepath();
    }
    if (ulting.active)
    {
        if (std::fabs(hspeed) < 5.0 and not heldkeys.LEFT and not heldkeys.RIGHT)
        {
            return herofolder()+"ult/1";
        }
        return ultwalkanim.getframepath();
    }
    if (rollanim.active())
    {
        return rollanim.getframepath();
    }
    if (crouchanim.active())
    {
        return crouchanim.getframepath();
    }
    if (not ongroundsmooth.active)
    {
        if (vspeed > 100)
        {
            return fallanim.getframepath();
        }
        else
        {
            return herofolder()+"jump/1";
        }
    }
    if (std::fabs(hspeed) < 11.0 and not heldkeys.LEFT and not heldkeys.RIGHT)
    {
        return herofolder()+"idle/1";
    }
    return runanim.getframepath();
}

bool Mccree::weaponvisible(Gamestate &state)
{
    return Character::weaponvisible(state) and not rollanim.active() and not stunanim.active()
           and (not ulting.active or state.get<Peacemaker>(weapon).isfiringult);
}
