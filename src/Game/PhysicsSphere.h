#ifndef _PHYSICSSPHERE_H_
#define _PHYSICSSPHERE_H_

#include "PhysicsObject.h"

class CollisionSpace;
class PhysicsWorld;

class PhysicsSphere : public PhysicsObject
{
public:
    void SetRadius(float);
    float GetRadius();
    PhysicsSphere(CollisionSpace*, PhysicsWorld*, float);
};

#endif // _PHYSICSSPHERE_H_
