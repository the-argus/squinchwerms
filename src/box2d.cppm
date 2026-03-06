module;

#include "macros.h"
#include <box2d/box2d.h>

export module box2d;
import aliases;
import allocator;
import slice;
import res;

/// Helper for calling GetCapacity() -> Get*(T* buf, int capacity) functions
/// from box2d
template <typename T, typename IDType, typename GetCapacityCallable,
          typename GetBufferCallable>
[[nodiscard]] Res<Slice<T>, alloc::Error>
getDataBufferHelper(Allocator &allocator, IDType id,
                    const GetCapacityCallable &getCapacity,
                    const GetBufferCallable &getBuffer)
{
    const u32 needed = getCapacity(id);
    const auto buffer = allocator.allocate(alloc::Request{
        .numBytes = sizeof(T) * needed,
        .alignment = alignof(T),
    });
    if (not isSuccess(buffer)) [[unlikely]]
        return buffer.error();

    auto *const start =
        reinterpret_cast<T *>(buffer->uncheckedAddressOfFirstItem());
    w_assert(buffer->sizeInBytes() >= sizeof(T) * needed, "box2d bug");

    const int used = getBuffer(id, start, needed);
    w_assert(needed == static_cast<u32>(used), "box2d bug");

    return unsafe::rawSlice(*start, needed);
}

export namespace b2 {
using Vec2 = Vec2;
using Segment = b2Segment;
using Capsule = b2Capsule;
using Circle = b2Circle;
using AABB = b2AABB;
using ContactData = b2ContactData;
using MassData = b2MassData;
using Transform = b2Transform;
using Rotation = b2Rot;

enum class BodyType
{
    /// zero mass, zero velocity, may be manually moved
    Static = 0,
    /// zero mass, velocity set by user, moved by solver
    Kinematic = 1,
    /// positive mass, velocity determined by forces, moved by solver
    Dynamic = 2,
};

using BodyID = BodyID;
using ShapeID = b2ShapeId;
using WorldID = b2WorldId;
using JointID = b2JointId;

struct BodyDef : public b2BodyDef
{
    BodyDef() NOEXCEPT : b2BodyDef(b2DefaultBodyDef()) {}
    explicit BodyDef(BodyType type) NOEXCEPT : b2BodyDef(b2DefaultBodyDef())
    {
        this->type = static_cast<b2BodyType>(type);
    }
    BodyDef(const b2BodyDef &actual) NOEXCEPT : b2BodyDef(actual) {}
};

struct ShapeDef : public b2ShapeDef
{
    ShapeDef() NOEXCEPT : b2ShapeDef(b2DefaultShapeDef()) {}
    ShapeDef(const b2ShapeDef &actual) NOEXCEPT : b2ShapeDef(actual) {}
};

struct WorldDef : public b2WorldDef
{
    WorldDef() NOEXCEPT : b2WorldDef(b2DefaultWorldDef()) {}
    WorldDef(const b2WorldDef &actual) NOEXCEPT : b2WorldDef(actual) {}
};

[[nodiscard]] BodyID createBody(WorldID world, const BodyDef &def) NOEXCEPT
{
    return b2CreateBody(world, &def);
}

[[nodiscard]] ShapeID createSegmentShape(BodyID body,
                                         const ShapeDef &definition,
                                         const Segment &segment) NOEXCEPT
{
    return b2CreateSegmentShape(body, &definition, &segment);
}

[[nodiscard]] ShapeID createCapsuleShape(BodyID body,
                                         const ShapeDef &definition,
                                         const Capsule &capsule) NOEXCEPT
{
    return b2CreateCapsuleShape(body, &definition, &capsule);
}

[[nodiscard]] ShapeID b2CreateCircleShape(BodyID body,
                                          const ShapeDef &definition,
                                          const Circle &circle) NOEXCEPT
{
    return b2CreateCircleShape(body, &definition, &circle);
}

[[nodiscard]] WorldID createWorld(const WorldDef &definition) NOEXCEPT
{
    return b2CreateWorld(&definition);
}

void destroyBody(BodyID body) NOEXCEPT { b2DestroyBody(body); }
void destroyShape(ShapeID shape, bool updateBodyMass) NOEXCEPT
{
    b2DestroyShape(shape, updateBodyMass);
}
void destroyWorld(WorldID world) NOEXCEPT { destroyWorld(world); }

using CastResultFunction = b2CastResultFcn;
using CustomFilterFunction = b2CustomFilterFcn;
using EnqueueTaskCallback = b2EnqueueTaskCallback;
using FinishTaskCallback = b2FinishTaskCallback;
using FrictionCallback = b2FrictionCallback;
using OverlapResultFunction = b2OverlapResultFcn;
using PreSolveFunction = b2PreSolveFcn;
using RestitutionCallback = b2RestitutionCallback;
using TaskCallback = b2TaskCallback;
using QueryFilter = b2QueryFilter;
using TreeStats = b2TreeStats;
using RayResult = b2RayResult;
using ShapeProxy = b2ShapeProxy;
using ExplosionDef = b2ExplosionDef;

/// Cast a capsule mover through the world. This is a special shape cast that
/// handles sliding along other shapes while reducing clipping.
[[nodiscard]] f32 worldCastMover(WorldID worldId, const Capsule &mover,
                                 Vec2 translation, QueryFilter filter) NOEXCEPT
{
    return b2World_CastMover(worldId, &mover, translation, filter);
}

/// Cast a ray into the world to collect shapes in the path of the ray.
/// Your callback function controls whether you get the closest point, any
/// point, or n-points.
/// @note The callback function may receive shapes in any order
/// @param worldId The world to cast the ray against
/// @param origin The start point of the ray
/// @param translation The translation of the ray from the start point to the
/// end point
/// @param filter Contains bit flags to filter unwanted shapes from the results
/// @param fcn A user implemented callback function
/// @param context A user context that is passed along to the callback function
///	@return traversal performance counters
[[nodiscard]] TreeStats worldCastRay(WorldID worldId, Vec2 origin,
                                     Vec2 translation, QueryFilter filter,
                                     CastResultFunction *fcn,
                                     void *context) NOEXCEPT
{
    return b2World_CastRay(worldId, origin, translation, filter, fcn, context);
}

/// Cast a ray into the world to collect the closest hit. This is a convenience
/// function. Ignores initial overlap. This is less general than
/// b2World_CastRay() and does not allow for custom filtering.
[[nodiscard]] RayResult worldCastRayClosest(WorldID worldId, Vec2 origin,
                                            Vec2 translation,
                                            QueryFilter filter) NOEXCEPT
{
    return b2World_CastRayClosest(worldId, origin, translation, filter);
}

/// Cast a shape through the world. Similar to a cast ray except that a shape is
/// cast instead of a point.
///	@see b2World_CastRay
[[nodiscard]] TreeStats worldCastShape(WorldID worldId, const ShapeProxy &proxy,
                                       Vec2 translation, QueryFilter filter,
                                       CastResultFunction *function,
                                       void *context) NOEXCEPT
{
    return b2World_CastShape(worldId, &proxy, translation, filter, function,
                             context);
}

/// Enable/disable continuous collision between dynamic and static bodies.
/// Generally you should keep continuous collision enabled to prevent fast
/// moving objects from going through static objects. The performance gain from
/// disabling continuous collision is minor.
/// @see b2WorldDef
void worldEnableContinuous(WorldID worldId, bool enabled) NOEXCEPT
{
    b2World_EnableContinuous(worldId, enabled);
}

/// Enable/disable sleep. If your application does not need sleeping, you can
/// gain some performance by disabling sleep completely at the world level.
/// @see b2WorldDef
void worldEnableSleeping(WorldID worldId, bool enabled) NOEXCEPT
{
    b2World_EnableSleeping(worldId, enabled);
}

/// Enable/disable constraint warm starting. Advanced feature for testing.
/// Disabling warm starting greatly reduces stability and provides no
/// performance gain.
void worldEnableWarmStarting(WorldID worldId, bool enabled) NOEXCEPT
{
    b2World_EnableWarmStarting(worldId, enabled);
}

/// Apply a radial explosion
/// @param worldId The world id
/// @param explosionDef The explosion definition
void worldExplode(WorldID worldId, const ExplosionDef &explosionDef) NOEXCEPT
{
    b2World_Explode(worldId, &explosionDef);
}

struct ContactTuningOptions
{
    float hertz;
    float dampingRatio;
    float pushSpeed;
};

/// Adjust contact tuning parameters
/// @param worldId The world id
/// @param hertz The contact stiffness (cycles per second)
/// @param dampingRatio The contact bounciness with 1 being critical damping
/// (non-dimensional)
/// @param pushSpeed The maximum contact constraint push out speed (meters per
/// second)
/// @note Advanced feature
void worldSetContactTuning(WorldID worldId,
                           const ContactTuningOptions &tuning) NOEXCEPT
{
    b2World_SetContactTuning(worldId, tuning.hertz, tuning.dampingRatio,
                             tuning.pushSpeed);
}

/// Set the gravity vector for the entire world. Box2D has no concept of an up
/// direction and this is left as a decision for the application. Usually in
/// m/s^2.
/// @see b2WorldDef
void worldSetGravity(WorldID worldId, Vec2 gravity) NOEXCEPT
{
    b2World_SetGravity(worldId, gravity);
}

/// Adjust the hit event threshold. This controls the collision speed needed to
/// generate a b2ContactHitEvent. Usually in meters per second.
/// @see b2WorldDef::hitEventThreshold
void worldSetHitEventThreshold(WorldID worldId, f32 value) NOEXCEPT
{
    b2World_SetHitEventThreshold(worldId, value);
}

// struct JointTuningOptions
// {
// 	float hertz;
// 	float dampingRatio;
// };
// void worldSetJointTuning 	( 	WorldID 	worldId, const
// JointTuningOptions& options ) NOEXCEPT { 	b2World_SetJointTuning(worldId,
// options.hertz, options.dampingRatio);
// }

/// Adjust the restitution threshold. It is recommended not to make this value
/// very small because it will prevent bodies from sleeping. Usually in meters
/// per second.
/// @see b2WorldDef
void worldSetRestitutionThreshold(WorldID worldId, f32 value) NOEXCEPT
{
    b2World_SetRestitutionThreshold(worldId, value);
}

/// Simulate a world for one time step. This performs collision detection,
/// integration, and constraint solution.
/// @param worldId The world to simulate
/// @param timeStep The amount of time to simulate, this should be a fixed
/// number. Usually 1/60.
/// @param subStepCount The number of sub-steps, increasing the sub-step count
/// can increase accuracy. Usually 4.
void worldStep(WorldID worldId, f32 timeStep, i32 subStepCount = 4) NOEXCEPT
{
    b2World_Step(worldId, timeStep, subStepCount);
}

// body --------------------------

/// Apply an angular impulse. The impulse is ignored if the body is not awake.
/// This optionally wakes the body.
/// @param bodyId The body id
/// @param impulse the angular impulse, usually in units of kg*m*m/s
/// @param wake also wake up the body
/// @warning This should be used for one-shot impulses. If you need a steady
/// force, use a force instead, which will work better with the sub-stepping
/// solver.
void bodyApplyAngularImpulse(BodyID bodyId, f32 impulse, bool wake) NOEXCEPT
{
    b2Body_ApplyAngularImpulse(bodyId, impulse, wake);
}

/// Apply a force at a world point. If the force is not applied at the center of
/// mass, it will generate a torque and affect the angular velocity. This
/// optionally wakes up the body. The force is ignored if the body is not awake.
/// @param bodyId The body id
/// @param force The world force vector, usually in newtons (N)
/// @param point The world position of the point of application
/// @param wake Option to wake up the body
void bodyApplyForce(BodyID bodyId, Vec2 force, Vec2 point, bool wake) NOEXCEPT
{
    b2Body_ApplyForce(bodyId, force, point, wake);
}

/// Apply a force to the center of mass. This optionally wakes up the body.
/// The force is ignored if the body is not awake.
/// @param bodyId The body id
/// @param force the world force vector, usually in newtons (N).
/// @param wake also wake up the body
void bodyApplyForceToCenter(BodyID bodyId, Vec2 force, bool wake) NOEXCEPT
{
    b2Body_ApplyForceToCenter(bodyId, force, wake);
}

/// Apply an impulse at a point. This immediately modifies the velocity.
/// It also modifies the angular velocity if the point of application
/// is not at the center of mass. This optionally wakes the body.
/// The impulse is ignored if the body is not awake.
/// @param bodyId The body id
/// @param impulse the world impulse vector, usually in N*s or kg*m/s.
/// @param point the world position of the point of application.
/// @param wake also wake up the body
/// @warning This should be used for one-shot impulses. If you need a steady
/// force, use a force instead, which will work better with the sub-stepping
/// solver.
void bodyApplyLinearImpulse(BodyID bodyId, Vec2 impulse, Vec2 point,
                            bool wake) NOEXCEPT
{
    b2Body_ApplyLinearImpulse(bodyId, impulse, point, wake);
}

/// Apply an impulse to the center of mass. This immediately modifies the
/// velocity. The impulse is ignored if the body is not awake. This optionally
/// wakes the body.
/// @param bodyId The body id
/// @param impulse the world impulse vector, usually in N*s or kg*m/s.
/// @param wake also wake up the body
/// @warning This should be used for one-shot impulses. If you need a steady
/// force, use a force instead, which will work better with the sub-stepping
/// solver.
void bodyApplyLinearImpulseToCenter(BodyID bodyId, Vec2 impulse,
                                    bool wake) NOEXCEPT
{
    b2Body_ApplyLinearImpulseToCenter(bodyId, impulse, wake);
}

/// This update the mass properties to the sum of the mass properties of the
/// shapes. This normally does not need to be called unless you called
/// SetMassData to override the mass and you later want to reset the mass. You
/// may also use this when automatic mass computation has been disabled. You
/// should call this regardless of body type. Note that sensor shapes may have
/// mass.
void bodyApplyMassFromShapes(BodyID bodyId) NOEXCEPT
{
    b2Body_ApplyMassFromShapes(bodyId);
}

/// Apply a torque. This affects the angular velocity without affecting the
/// linear velocity. This optionally wakes the body. The torque is ignored if
/// the body is not awake.
/// @param bodyId The body id
/// @param torque about the z-axis (out of the screen), usually in N*m.
/// @param wake also wake up the body
void bodyApplyTorque(BodyID bodyId, float torque, bool wake) NOEXCEPT
{
    b2Body_ApplyTorque(bodyId, torque, wake);
}

/// Get the current world AABB that contains all the attached shapes. Note that
/// this may not encompass the body origin. If there are no shapes attached then
/// the returned AABB is empty and centered on the body origin.
[[nodiscard]] AABB bodyComputeAABB(BodyID bodyId) NOEXCEPT
{
    return b2Body_ComputeAABB(bodyId);
}

/// Enable/disable contact events on all shapes.
/// @see b2ShapeDef::enableContactEvents
/// @warning changing this at runtime may cause mismatched begin/end touch
/// events
void bodyEnableContactEvents(BodyID bodyId, bool isEnabled) NOEXCEPT
{
    b2Body_EnableContactEvents(bodyId, isEnabled);
}

/// Enable/disable hit events on all shapes
/// @see b2ShapeDef::enableHitEvents
void bodyEnableHitEvents(BodyID bodyId, bool isEnabled) NOEXCEPT
{
    b2Body_EnableHitEvents(bodyId, isEnabled);
}

/// Get the maximum capacity required for retrieving all the touching contacts
/// on a body
[[nodiscard]] u32 bodyGetContactCapacity(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetContactCapacity(bodyId));
}

/// Get the touching contact data for a body.
/// @note Box2D uses speculative collision so some contact points may be
/// separated.
/// @returns the number of elements filled in the provided array
/// @warning do not ignore the return value, it specifies the valid number of
/// elements
[[nodiscard]] int bodyGetContactDataUnsafe(BodyID bodyId,
                                           ContactData *contactData,
                                           u32 capacity) NOEXCEPT
{
    return b2Body_GetContactData(bodyId, contactData,
                                 static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<ContactData>, alloc::Error>
bodyGetContactData(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<ContactData>(
        allocator, bodyId, bodyGetContactCapacity, bodyGetContactDataUnsafe);
}

/// Get the number of joints on this body
[[nodiscard]] u32 bodyGetJointCount(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetJointCount(bodyId));
}

/// Get the joint ids for all joints on this body, up to the provided capacity
/// @returns the number of joint ids stored in the user array
[[nodiscard]] int bodyGetJointsUnsafe(BodyID bodyId, JointID *jointIdBuffer,
                                      u32 capacity) NOEXCEPT
{
    return b2Body_GetJoints(bodyId, jointIdBuffer, static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<JointID>, alloc::Error>
bodyGetJoints(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<JointID>(allocator, bodyId, bodyGetJointCount,
                                        bodyGetJointsUnsafe);
}

/// Get the number of shapes on this body
[[nodiscard]] u32 bodyGetShapeCount(BodyID bodyId) NOEXCEPT
{
    return static_cast<u32>(b2Body_GetShapeCount(bodyId));
}

/// Get the shape ids for all shapes on this body, up to the provided capacity.
/// @returns the number of shape ids stored in the user array
[[nodiscard]] int bodyGetShapesUnsafe(BodyID bodyId, ShapeID *shapeIdBuffer,
                                      u32 capacity) NOEXCEPT
{
    return b2Body_GetShapes(bodyId, shapeIdBuffer, static_cast<int>(capacity));
}

[[nodiscard]] Res<Slice<ShapeID>, alloc::Error>
bodyGetShapes(Allocator &allocator, BodyID bodyId) NOEXCEPT
{
    return getDataBufferHelper<ShapeID>(allocator, bodyId, bodyGetShapeCount,
                                        bodyGetShapesUnsafe);
}

/// Wake a body from sleep. This wakes the entire island the body is touching.
/// @warning Putting a body to sleep will put the entire island of bodies
/// touching this body to sleep, which can be expensive and possibly
/// unintuitive.
void bodySetAwake(BodyID body, bool newIsAwake) NOEXCEPT
{
    return b2Body_SetAwake(body, newIsAwake);
}

/// @return true if this body is awake
[[nodiscard]] bool bodyIsAwake(BodyID body) NOEXCEPT
{
    return b2Body_IsAwake(body);
}

/// Set this body to be a bullet. A bullet does continuous collision detection
/// against dynamic bodies (but not other bullets).
void bodySetBullet(BodyID body, bool newIsBullet) NOEXCEPT
{
    b2Body_SetBullet(body, newIsBullet);
}

/// Is this body a bullet?
[[nodiscard]] bool bodyIsBullet(BodyID body) NOEXCEPT
{
    return b2Body_IsBullet(body);
}

/// Adjust the gravity scale. Normally this is set in b2BodyDef before creation.
/// @see b2BodyDef::gravityScale
void bodySetGravityScale(BodyID body, f32 newGravityScale) NOEXCEPT
{
    b2Body_SetGravityScale(body, newGravityScale);
}

/// Get the current gravity scale
[[nodiscard]] f32 bodyGetGravityScale(BodyID body) NOEXCEPT
{
    return b2Body_GetGravityScale(body);
}

/// Override the body's mass properties. Normally this is computed automatically
/// using the shape geometry and density. This information is lost if a shape is
/// added or removed or if the body type changes.
void bodySetMassData(BodyID body, MassData newMassData) NOEXCEPT
{
    b2Body_SetMassData(body, newMassData);
}

/// Get the mass data for a body
[[nodiscard]] MassData bodyGetMassData(BodyID body) NOEXCEPT
{
    return b2Body_GetMassData(body);
}

/// Set the velocity to reach the given transform after a given time step.
/// The result will be close but maybe not exact. This is meant for kinematic
/// bodies. The target is not applied if the velocity would be below the sleep
/// threshold. This will automatically wake the body if asleep.
void bodySetTargetTransform(BodyID body, Transform transform,
                            f32 timeStep) NOEXCEPT
{
    b2Body_SetTargetTransform(body, transform, timeStep);
}

/// Set the world transform of a body. This acts as a teleport and is fairly
/// expensive.
/// @note Generally you should create a body with then intended transform.
/// @see b2BodyDef::position and b2BodyDef::angle
void bodySetTransform(BodyID body, Vec2 position, Rotation rotation) NOEXCEPT
{
    b2Body_SetTransform(body, position, rotation);
}

/// Get the world position of a body. This is the location of the body origin.
Vec2 bodyGetPosition(BodyID bodyId) { return b2Body_GetPosition(bodyId); }

/// Get the world rotation of a body as a cosine/sine pair (complex number)
Rotation bodyGetRotation(BodyID bodyId) { return b2Body_GetRotation(bodyId); }

/// Get the world transform of a body.
[[nodiscard]] Transform bodyGetTransform(BodyID body) NOEXCEPT
{
    return b2Body_GetTransform(body);
}

/// Change the body type. This is an expensive operation. This automatically
/// updates the mass properties regardless of the automatic mass setting.
void bodySetType(BodyID body, BodyType newType) NOEXCEPT
{
    b2Body_SetType(body, static_cast<b2BodyType>(newType));
}

/// Get the body type: static, kinematic, or dynamic
[[nodiscard]] BodyType bodyGetType(BodyID bodyId) NOEXCEPT
{
    return static_cast<BodyType>(b2Body_GetType(bodyId));
}

/// Body identifier validation. Can be used to detect orphaned ids. Provides
/// validation for up to 64K allocations.
[[nodiscard]] bool bodyIsValid(BodyID body) NOEXCEPT
{
    return b2Body_IsValid(body);
}

/// Set the body name. Up to 31 characters excluding 0 termination.
void bodySetName(BodyID bodyId, const char *name) NOEXCEPT
{
    b2Body_SetName(bodyId, name);
}

/// Get the body name. May be null.
const char *bodyGetName(BodyID bodyId)
{
    auto *out = b2Body_GetName(bodyId);
    return out ? out : "UNKNOWNBODYNAME";
}

/// Set the user data for a body
void setUserData(BodyID bodyId, void *userData)
{
    b2Body_SetUserData(bodyId, userData);
}

/// Get the user data stored in a body
void *bodyGetUserData(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetUserData(bodyId);
}

/// Get a local point on a body given a world point
[[nodiscard]] Vec2 bodyGetLocalPoint(BodyID bodyId, Vec2 worldPoint) NOEXCEPT
{
    return b2Body_GetLocalPoint(bodyId, worldPoint);
}

/// Get a world point on a body given a local point
[[nodiscard]] Vec2 bodyGetWorldPoint(BodyID bodyId, Vec2 localPoint) NOEXCEPT
{
    return b2Body_GetWorldPoint(bodyId, localPoint);
}

/// Get a local vector on a body given a world vector
[[nodiscard]] Vec2 bodyGetLocalVector(BodyID bodyId, Vec2 worldVector) NOEXCEPT
{
    return b2Body_GetLocalVector(bodyId, worldVector);
}

/// Get a world vector on a body given a local vector
[[nodiscard]] Vec2 bodyGetWorldVector(BodyID bodyId, Vec2 localVector) NOEXCEPT
{
    return b2Body_GetWorldVector(bodyId, localVector);
}

/// Get the linear velocity of a body's center of mass. Usually in meters per
/// second.
[[nodiscard]] Vec2 bodyGetLinearVelocity(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetLinearVelocity(bodyId);
}

/// Get the angular velocity of a body in radians per second
[[nodiscard]] f32 bodyGetAngularVelocity(BodyID bodyId) NOEXCEPT
{
    return b2Body_GetAngularVelocity(bodyId);
}

/// Set the linear velocity of a body. Usually in meters per second.
void bodySetLinearVelocity(BodyID bodyId, Vec2 linearVelocity) NOEXCEPT
{
    b2Body_SetLinearVelocity(bodyId, linearVelocity);
}

/// Set the angular velocity of a body in radians per second
void bodySetAngularVelocity(BodyID bodyId, f32 angularVelocity) NOEXCEPT
{
    b2Body_SetAngularVelocity(bodyId, angularVelocity);
}

} // namespace b2
