#ifndef MCCREE_H
#define MCCREE_H

#include "character.h"
#include "player.h"


class Mccree : public Character
{
    public:
        Mccree(Gamestate *state, Player *owner);
        virtual ~Mccree();
        void render(ALLEGRO_BITMAP *background, ALLEGRO_BITMAP *midground, ALLEGRO_BITMAP *foreground, int cam_x, int cam_y);
        Mccree* clone(Gamestate *state);
    protected:
    private:
};

#endif // MCCREE_H
