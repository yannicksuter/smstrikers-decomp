#ifndef _PHYSICSOBJECT_H_
#define _PHYSICSOBJECT_H_

#include "NL/nlMath.h"

#include "ode/objects.h"
#include "ode/collision.h"

// #include "PhysicsWorld.h"

// todo: it is at the wrong place.... needs to be in the class
float _DefaultGravity = -9.8f;


// class dContact;
// class dxSpace;
class PhysicsWorld;

// void ConvertDMat3ToNLMat4(const nlMatrix3*, nlMatrix4*);

class PhysicsObject
{
public:
    enum CoordinateType {
        option0 = 0,
    };

    void CloneObject(const PhysicsObject&);
    void Contact(PhysicsObject*, dContact*, int, PhysicsObject*);
    void Contact(PhysicsObject*, dContact*, int);
    void MakeStatic();
    void SetMass(float);
    void Reconnect(dxSpace*);
    void Disconnect();
    void EnableCollisions();
    void DisableCollisions();
    void SetWorldMatrix(const nlMatrix4&);
    void SetContactInfo(dContact*, PhysicsObject*, bool);
    void SetDefaultContactInfo(dContact*);
    void ZeroForceAccumulators();
    void AddForceAtCentreOfMass(const nlVector3&);
    void GetAngularVelocity(nlVector3*) const;
    void SetAngularVelocity(const nlVector3&);
    void GetLinearVelocity();
    void GetLinearVelocity(nlVector3*) const;
    void SetLinearVelocity(const nlVector3&);
    void GetRotation(nlMatrix4*) const;
    void SetRotation(const nlMatrix4&);
    void SetRotation(const nlMatrix3&);
    void GetPosition();
    void GetPosition(nlVector3*) const;
    void SetPosition(const nlVector3&, CoordinateType);
    void PostUpdate();
    void PreUpdate();
    void CheckForNaN();
    void SetCategory(unsigned int);
    void SetCollide(unsigned int);
    void SetDefaultCollideBits();

    ~PhysicsObject();
    PhysicsObject(PhysicsWorld*);

    u8 _padding[4];
    /* 0x04 */ dBodyID _bodyID;
    /* 0x08 */ dGeomID _geomID;
    /* 0x10 */ float _gravity;
};

#endif



// .include "macros.inc"
// .file "PhysicsObject.o"

// # 0x801FFCFC..0x80201130 | size: 0x1434
// .text
// .balign 4

// # 0x80201130..0x80201198 | size: 0x68
// .section .text, "ax", unique, 2
// .balign 4

// # .text:0x0 | 0x80201130 | size: 0x10
// # nlVector3::Set(float, float, float)

// # .text:0x10 | 0x80201140 | size: 0x24
// # nlMatrix4::SetColumn(int, const nlVector3&)

// # .text:0x34 | 0x80201164 | size: 0x34
// # nlVecAdd(nlVector3&, const nlVector3&, const nlVector3&)

// # 0x80303138..0x80303160 | size: 0x28
// .data
// .balign 8

// # .data:0x0 | 0x80303138 | size: 0x28
// # PhysicsObject::__vtable
// .obj __vt__13PhysicsObject, global
//     .4byte 0x00000000
//     .4byte 0x00000000
//     .4byte __dt__13PhysicsObjectFv
//     .4byte 0x00000000
//     .4byte SetContactInfo__13PhysicsObjectFP8dContactP13PhysicsObjectb
//     .4byte PreUpdate__13PhysicsObjectFv
//     .4byte PostUpdate__13PhysicsObjectFv
//     .4byte PreCollide__13PhysicsObjectFv
//     .4byte Contact__13PhysicsObjectFP13PhysicsObjectP8dContacti
//     .4byte Contact__13PhysicsObjectFP13PhysicsObjectP8dContactiP13PhysicsObject
// .endobj __vt__13PhysicsObject

// # 0x80373318..0x80373320 | size: 0x8
// .section .sdata, "wa"
// .balign 8

// # .sdata:0x0 | 0x80373318 | size: 0x4
// # PhysicsObject::DefaultGravity
// .obj DefaultGravity__13PhysicsObject, global
//     .float -9.8
// .endobj DefaultGravity__13PhysicsObject
//     .4byte 0x00000000

// # 0x80377460..0x80377480 | size: 0x20
// .section .sdata2, "a"
// .balign 8

// # .sdata2:0x0 | 0x80377460 | size: 0x4
// .obj "@442", local
//     .float 0
// .endobj "@442"

// # .sdata2:0x4 | 0x80377464 | size: 0x4
// .obj "@443", local
//     .float 1
// .endobj "@443"

// # .sdata2:0x8 | 0x80377468 | size: 0x4
// .obj "@525", local
//     .float -1
// .endobj "@525"

// # .sdata2:0xC | 0x8037746C | size: 0x4
// .obj "@593", local
//     .float 0.0001
// .endobj "@593"

// # .sdata2:0x10 | 0x80377470 | size: 0x4
// .obj "@594", local
//     .float 0.9
// .endobj "@594"

// # .sdata2:0x14 | 0x80377474 | size: 0x4
// .obj "@595", local
//     .float 0.2
// .endobj "@595"

// # .sdata2:0x18 | 0x80377478 | size: 0x4
// .obj "@596", local
//     .float 0.1
// .endobj "@596"
//     .4byte 0x00000000
