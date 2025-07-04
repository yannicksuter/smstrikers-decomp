#ifndef _PHYSICSCOLUMN_H_
#define _PHYSICSCOLUMN_H_

#include "PhysicsObject.h"
#include "CollisionSpace.h"
#include "ode/NLGAdditions.h"
#include "ode/common.h"

class PhysicsColumn : public PhysicsObject
{
public:
    void GetRadius(float*);
    PhysicsColumn(CollisionSpace*, PhysicsWorld*, float);
    virtual ~PhysicsColumn();
    virtual int GetObjectType() const;

    /* 0x2c */ dJointID m_jointID;

};

#endif // _PHYSICSCOLUMN_H_
