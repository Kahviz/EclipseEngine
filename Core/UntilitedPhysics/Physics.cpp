#include "Physics.h"
#include "Math/UntilitedMath.h"

bool Physics::ApplyGravity(Instance& inst, float Deltatime)
{
    if (Running) {
        Vector3 pos = inst.pos;
        Vector3 vel = inst.Velocity;

        vel.y -= Gravity * Deltatime;
        pos.y += vel.y * Deltatime;

        inst.Velocity = vel;
        inst.pos = pos;

        return true;
    }
    else {
        return false;
    }
}