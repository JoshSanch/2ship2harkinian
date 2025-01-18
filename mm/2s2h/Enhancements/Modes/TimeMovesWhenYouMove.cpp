#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_Arms_Hook/z_arms_hook.h"
void ArmsHook_Shoot(ArmsHook* hook, PlayState* play);
}

#define CVAR_NAME "gModes.TimeMovesWhenYouMove"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterTimeMovesWhenYouMove() {
    gSaveContext.save.timeSpeedOffset = 0;

    COND_HOOK(ShouldActorUpdate, CVAR, [](Actor* actor, bool* should) {
        static bool hookIsFiring = false;
        if (actor->id == ACTOR_ARMS_HOOK) {
            ArmsHook* hook = (ArmsHook*)actor;
            if (hook->actionFunc == ArmsHook_Shoot) {
                hookIsFiring = true;
            } else {
                hookIsFiring = false;
            }
        }

        if (actor->id != ACTOR_EN_ARROW &&
            (actor->id == ACTOR_PLAYER || actor->category == ACTORCAT_BG || actor->category == ACTORCAT_DOOR ||
             actor->category == ACTORCAT_SWITCH || actor->category == ACTORCAT_ITEMACTION)) {
            return;
        }

        Player* player = GET_PLAYER(gPlayState);

        if (player->linearVelocity == 0 && !(player->stateFlags1 & PLAYER_STATE1_1) &&
            !(player->stateFlags1 & PLAYER_STATE1_2) && !(player->stateFlags1 & PLAYER_STATE1_20) &&
            !(player->stateFlags1 & PLAYER_STATE1_40) && !(player->stateFlags1 & PLAYER_STATE1_80) &&
            !(player->stateFlags1 & PLAYER_STATE1_100) && !(player->stateFlags1 & PLAYER_STATE1_400) &&
            !(player->stateFlags1 & PLAYER_STATE1_1000) && !(player->stateFlags1 & PLAYER_STATE1_2000000) &&
            !(player->stateFlags1 & PLAYER_STATE1_10000000) && !(player->stateFlags1 & PLAYER_STATE1_20000000) &&
            !(player->stateFlags2 & PLAYER_STATE2_8) && !(player->stateFlags3 & PLAYER_STATE3_8) &&
            !(player->stateFlags3 & PLAYER_STATE3_2000000) && (!hookIsFiring)) {
            *should = false;
        }
    });

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, CVAR, [](Actor* actor) {
        Player* player = GET_PLAYER(gPlayState);

        gSaveContext.save.timeSpeedOffset = -R_TIME_SPEED;
        if (player->linearVelocity != 0) {
            gSaveContext.save.timeSpeedOffset = CLAMP(player->linearVelocity / 2 - R_TIME_SPEED, -R_TIME_SPEED, 10);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterTimeMovesWhenYouMove, { CVAR_NAME });
