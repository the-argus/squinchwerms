module;

#include "macros.h"

export module physics;

import box2d;
import aliases;
import reflection;
import is_instance;
import opt;
import res;
import slice;
import allocator;

export using Vec2 = b2::Vec2;
export using AABB = b2::AABB;
export using Rotation = b2::Rotation;
export using Transform = b2::Transform;
export using BodyType = b2::BodyType;

template <bool isConst> class WorldImpl;

template <bool isConst> class BodyImpl
{
  private:
    b2::BodyID id;

  public:
    struct Options
    {
        Opt<b2::BodyType> type;

        /// The initial world position of the body. Bodies should be created
        /// with the desired position.
        /// @note Creating bodies at the origin and then moving them nearly
        /// doubles the cost of body creation, especially if the body is moved
        /// after shapes have been added.
        Opt<b2::Vec2> position;

        /// The initial world rotation of the body. Use b2MakeRot() if you have
        /// an angle.
        Opt<b2::Rotation> rotation;

        /// The initial linear velocity of the body's origin. Usually in meters
        /// per second.
        Opt<b2::Vec2> linearVelocity;

        /// The initial angular velocity of the body. Radians per second.
        Opt<f32> angularVelocity;

        /// Linear damping is used to reduce the linear velocity. The damping
        /// parameter can be larger than 1 but the damping effect becomes
        /// sensitive to the time step when the damping parameter is large.
        /// Generally linear damping is undesirable because it makes objects
        /// move slowly as if they are floating.
        Opt<f32> linearDamping;

        /// Angular damping is used to reduce the angular velocity. The damping
        /// parameter can be larger than 1.0f but the damping effect becomes
        /// sensitive to the time step when the damping parameter is large.
        /// Angular damping can be use slow down rotating bodies.
        Opt<f32> angularDamping;

        /// Scale the gravity applied to this body. Non-dimensional.
        Opt<f32> gravityScale;

        /// Sleep speed threshold, default is 0.05 meters per second
        Opt<f32> sleepThreshold;

        /// Optional body name for debugging. Up to 31 characters (excluding
        /// null termination)
        const char *name = nullptr;

        void *userData = nullptr;

        /// Set this flag to false if this body should never fall asleep.
        Opt<bool> enableSleep;

        /// Is this body initially awake or sleeping?
        Opt<bool> isAwake;

        /// Should this body be prevented from rotating? Useful for characters.
        Opt<bool> fixedRotation;

        /// Treat this body as high speed object that performs continuous
        /// collision detection against dynamic and kinematic bodies, but not
        /// other bullet bodies.
        /// @warning Bullets should be used sparingly. They are not a solution
        /// for general dynamic-versus-dynamic continuous collision. They may
        /// interfere with joint constraints.
        Opt<bool> isBullet;

        /// Used to disable a body. A disabled body does not move or collide.
        Opt<bool> isEnabled;

        /// This allows this body to bypass rotational speed limits. Should only
        /// be used for circular objects, like wheels.
        Opt<bool> allowFastRotation;
    };

  private:
    constexpr explicit BodyImpl(b2::BodyID id) NOEXCEPT : id(id) {}

  public:
    friend class BodyImpl<not isConst>;
    template <bool worldConst> friend class WorldImpl;

    BodyImpl() = delete;

    constexpr operator b2::BodyID() const NOEXCEPT { return id; }

    /// Allow conversion from non-const to const
    constexpr operator BodyImpl<true>() const NOEXCEPT
        requires(not isConst)
    {
        return BodyImpl<true>(id);
    }

    void destroy() const NOEXCEPT
        requires(not isConst)
    {
        b2::destroyBody(id);
    }

    /// Apply an angular impulse. The impulse is ignored if the body is not
    /// awake. This optionally wakes the body.
    /// @param bodyId The body id
    /// @param impulse the angular impulse, usually in units of kg*m*m/s
    /// @param wake also wake up the body
    /// @warning This should be used for one-shot impulses. If you need a steady
    /// force, use a force instead, which will work better with the sub-stepping
    /// solver.
    BodyImpl applyAngularImpulse(f32 impulse, bool wake = true) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodyApplyAngularImpulse(id, impulse, wake);
        return *this;
    }

    /// Apply a force at a world point. If the force is not applied at the
    /// center of mass, it will generate a torque and affect the angular
    /// velocity. This optionally wakes up the body. The force is ignored if the
    /// body is not awake.
    /// @param bodyId The body id
    /// @param force The world force vector, usually in newtons (N)
    /// @param point The world position of the point of application. If not
    /// provided, then the center of the body is used.
    /// @param wake Option to wake up the body
    BodyImpl applyForce(Vec2 impulse, Opt<Vec2> point = {},
                        bool wake = true) const NOEXCEPT
        requires(not isConst)
    {
        if (point) {
            b2::bodyApplyForce(id, impulse, point.unwrap(), wake);
        } else {
            b2::bodyApplyForceToCenter(id, impulse, wake);
        }
        return *this;
    }

    /// Apply an impulse at a point. This immediately modifies the velocity.
    /// It also modifies the angular velocity if the point of application
    /// is not at the center of mass. This optionally wakes the body.
    /// The impulse is ignored if the body is not awake.
    /// @param bodyId The body id
    /// @param impulse the world impulse vector, usually in N*s or kg*m/s.
    /// @param point the world position of the point of application. If not
    /// provided, then the center of the body is used.
    /// @param wake also wake up the body
    /// @warning This should be used for one-shot impulses. If you need a steady
    /// force, use a force instead, which will work better with the sub-stepping
    /// solver.
    BodyImpl applyLinearImpulse(Vec2 impulse, Opt<Vec2> point = {},
                                bool wake = true) const NOEXCEPT
        requires(not isConst)
    {
        if (point) {
            b2::bodyApplyLinearImpulse(id, impulse, point.unwrap(), wake);
        } else {
            b2::bodyApplyLinearImpulseToCenter(id, impulse, wake);
        }
        return *this;
    }

    /// This update the mass properties to the sum of the mass properties of the
    /// shapes. This normally does not need to be called unless you called
    /// SetMassData to override the mass and you later want to reset the mass.
    /// You may also use this when automatic mass computation has been disabled.
    /// You should call this regardless of body type. Note that sensor shapes
    /// may have mass.
    BodyImpl applyMassFromShapes() const NOEXCEPT
        requires(not isConst)
    {
        b2::bodyApplyMassFromShapes(id);
        return *this;
    }

    /// Apply a torque. This affects the angular velocity without affecting the
    /// linear velocity. This optionally wakes the body. The torque is ignored
    /// if the body is not awake.
    /// @param bodyId The body id
    /// @param torque about the z-axis (out of the screen), usually in N*m.
    /// @param wake also wake up the body
    BodyImpl applyTorque(f32 torque, bool wake = true) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodyApplyTorque(id, torque, wake);
        return *this;
    }

    /// Get the current world AABB that contains all the attached shapes. Note
    /// that this may not encompass the body origin. If there are no shapes
    /// attached then the returned AABB is empty and centered on the body
    /// origin.
    [[nodiscard]] AABB computeAABB() const NOEXCEPT
    {
        return b2::bodyComputeAABB(id);
    }

    /// Enable/disable contact events on all shapes.
    /// @see b2ShapeDef::enableContactEvents
    /// @warning changing this at runtime may cause mismatched begin/end touch
    /// events
    BodyImpl setContactEventsEnabled(bool isEnabled) NOEXCEPT
        requires(not isConst)
    {
        b2::bodyEnableContactEvents(id, isEnabled);
        return *this;
    }

    /// Enable/disable hit events on all shapes
    /// @see b2ShapeDef::enableHitEvents
    BodyImpl setHitEventsAreEnabled(bool isEnabled) NOEXCEPT
        requires(not isConst)
    {
        b2::bodyEnableHitEvents(id, isEnabled);
        return *this;
    }

    /// Get the touching contact data for a body.
    /// @note Box2D uses speculative collision so some contact points may be
    /// separated.
    [[nodiscard]] Res<Slice<b2::ContactData>, alloc::Error>
    contactData(Allocator &allocator) const NOEXCEPT
    {
        return b2::bodyGetContactData(allocator, id);
    }

    [[nodiscard]] Opt<b2::ContactData> firstContactData() const NOEXCEPT
    {
        b2::ContactData out;
        const int numWritten = b2::bodyGetContactDataUnsafe(id, &out, 1);

        if (numWritten)
            return out;
        return null;
    }

    /// Get the joint ids for all joints on this body
    [[nodiscard]] Res<Slice<b2::JointID>, alloc::Error>
    joints(Allocator &allocator) const NOEXCEPT
    {
        return b2::bodyGetJoints(allocator, id);
    }

    [[nodiscard]] Opt<b2::JointID> firstJoint() const NOEXCEPT
    {
        b2::JointID out;
        const int numWritten = b2::bodyGetJointsUnsafe(id, &out, 1);

        if (numWritten)
            return out;
        return null;
    }

    /// Get the shape ids for all shapes on this body
    [[nodiscard]] Res<Slice<b2::ShapeID>, alloc::Error>
    shapes(Allocator &allocator) const NOEXCEPT
    {
        return b2::bodyGetShapes(allocator, id);
    }

    [[nodiscard]] Opt<b2::ShapeID> firstShape() const NOEXCEPT
    {
        b2::ShapeID out;
        const int numWritten = b2::bodyGetShapesUnsafe(id, &out, 1);

        if (numWritten)
            return out;
        return null;
    }

    /// Get the number of shapes on this body
    [[nodiscard]] u64 shapeCount() const NOEXCEPT
    {
        return b2::bodyGetShapeCount(id); // returns u32
    }

    /// Get the number of joints on this body
    [[nodiscard]] u64 jointCount() const NOEXCEPT
    {
        return b2::bodyGetJointCount(id); // returns u32
    }

    /// @return true if this body is awake
    [[nodiscard]] bool isAwake() const NOEXCEPT { return b2::bodyIsAwake(id); }

    /// Wake a body from sleep. This wakes the entire island the body is
    /// touching.
    /// @warning Putting a body to sleep will put the entire island of bodies
    /// touching this body to sleep, which can be expensive and possibly
    /// unintuitive.
    BodyImpl setIsAwake(bool isAwakeNow) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetAwake(id, isAwakeNow);
        return *this;
    }

    /// Is this body a bullet?
    [[nodiscard]] bool isBullet() const NOEXCEPT
    {
        return b2::bodyIsBullet(id);
    }

    /// Set this body to be a bullet. A bullet does continuous collision
    /// detection against dynamic bodies (but not other bullets).
    BodyImpl setIsBullet(bool isBulletNow) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetBullet(id, isBulletNow);
        return *this;
    }

    /// Get the current gravity scale
    [[nodiscard]] f32 gravityScale() const NOEXCEPT
    {
        return b2::bodyGetGravityScale(id);
    }

    /// Adjust the gravity scale. Normally this is set in b2BodyDef before
    /// creation.
    /// @see b2BodyDef::gravityScale
    BodyImpl setGravityScale(f32 newGravityScale) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetGravityScale(id, newGravityScale);
    }

    /// Get the mass data for a body
    [[nodiscard]] b2::MassData massData() const NOEXCEPT
    {
        return b2::bodyGetMassData(id);
    }

    /// Override the body's mass properties. Normally this is computed
    /// automatically using the shape geometry and density. This information is
    /// lost if a shape is added or removed or if the body type changes.
    BodyImpl setMassData(const b2::MassData &newMassData) const NOEXCEPT
        requires(not isConst)
    {
        return b2::bodySetMassData(id, newMassData);
    }

    /// Set the velocity to reach the given transform after a given time step.
    /// The result will be close but maybe not exact. This is meant for
    /// kinematic bodies. The target is not applied if the velocity would be
    /// below the sleep threshold. This will automatically wake the body if
    /// asleep.
    BodyImpl setTargetTransform(Transform target, f32 timeStep) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetTargetTransform(id, target, timeStep);
        return *this;
    }

    /// Set the world transform of a body. This acts as a teleport and is fairly
    /// expensive.
    /// @note Generally you should create a body with then intended transform.
    /// @see b2BodyDef::position and b2BodyDef::angle
    BodyImpl setTransform(Vec2 newPosition, Rotation newRotation) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetTransform(id, newPosition, newRotation);
        return *this;
    }

    /// Get the world position of a body. This is the location of the body
    /// origin.
    [[nodiscard]] Vec2 position() const NOEXCEPT
    {
        return b2::bodyGetPosition(id);
    }

    /// Get the world rotation of a body as a cosine/sine pair (complex number)
    [[nodiscard]] Rotation rotation() const NOEXCEPT
    {
        return b2::bodyGetRotation(id);
    }

    /// Get the world transform of a body.
    [[nodiscard]] Transform transform() const NOEXCEPT
    {
        return b2::bodyGetTransform(id);
    }

    /// Get the body type: static, kinematic, or dynamic
    [[nodiscard]] BodyType type() const NOEXCEPT { return b2::bodyGetType(id); }

    /// Change the body type. This is an expensive operation. This automatically
    /// updates the mass properties regardless of the automatic mass setting.
    BodyImpl setType(BodyType newType) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetType(id, newType);
        return *this;
    }

    /// Body identifier validation. Can be used to detect orphaned ids. Provides
    /// validation for up to 64K allocations.
    [[nodiscard]] bool isValid() const NOEXCEPT { return b2::bodyIsValid(id); }

    explicit operator bool() const NOEXCEPT { return isValid(); }

    /// Get the body name. May be null.
    [[nodiscard]] const char *name() const NOEXCEPT
    {
        return b2::bodyGetName(id);
    }

    /// Set the body name. Up to 31 characters excluding 0 termination.
    BodyImpl setName(const char *newName) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetName(id, newName);
        return *this;
    }

    /// Set the user data for a body
    BodyImpl setUserData(void *userData) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetUserData(id, userData);
        return *this;
    }

    /// Get the user data stored in a body
    [[nodiscard]] void *userData() const NOEXCEPT
    {
        return b2::bodyGetUserData(id);
    }

    /// Get a local point on a body given a world point
    [[nodiscard]] Vec2 toLocal(Vec2 worldPoint) const NOEXCEPT
    {
        return b2::bodyGetLocalPoint(id, worldPoint);
    }

    /// Get a world point on a body given a local point
    [[nodiscard]] Vec2 toWorld(Vec2 localPoint) const NOEXCEPT
    {
        return b2::bodyGetWorldPoint(id, localPoint);
    }

    /// Get the linear velocity of a body's center of mass. Usually in meters
    /// per second.
    [[nodiscard]] Vec2 linearVelocity() const NOEXCEPT
    {
        return b2::bodyGetLinearVelocity(id);
    }

    /// Get the angular velocity of a body in radians per second
    [[nodiscard]] f32 angularVelocity() const NOEXCEPT
    {
        return b2::bodyGetAngularVelocity(id);
    }

    /// Set the linear velocity of a body. Usually in meters per second.
    BodyImpl setLinearVelocity(Vec2 newLinearVelocity) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetLinearVelocity(id, newLinearVelocity);
        return *this;
    }

    /// Set the angular velocity of a body in radians per second
    BodyImpl setAngularVelocity(f32 newAngularVelocity) const NOEXCEPT
        requires(not isConst)
    {
        b2::bodySetAngularVelocity(id, newAngularVelocity);
        return *this;
    }

    BodyImpl enable() const NOEXCEPT
        requires(not isConst)
    {
        b2::bodyEnable(id);
        return *this;
    }

    BodyImpl disable() const NOEXCEPT
        requires(not isConst)
    {
        b2::bodyDisable(id);
        return *this;
    }

    [[nodiscard]] bool isEnabled() const NOEXCEPT
    {
        return b2::bodyIsEnabled(id);
    }
};

export using Body = BodyImpl<false>;
export using BodyConst = BodyImpl<true>;

template <bool isConst> class WorldImpl
{
  private:
    b2::WorldID id;

    WorldImpl(b2::WorldID _id) NOEXCEPT : id(_id) {}

  public:
    friend class WorldImpl<not isConst>;

    WorldImpl() = delete;

    struct Options
    {
        Opt<Vec2> gravity;

        /// Restitution speed threshold, usually in m/s. Collisions above this
        /// speed have restitution applied (will bounce).
        Opt<f32> restitutionThreshold;

        /// Threshold speed for hit events. Usually meters per second.
        Opt<f32> hitEventThreshold;

        /// Contact stiffness. Cycles per second. Increasing this increases the
        /// speed of overlap recovery, but can introduce jitter.
        Opt<f32> contactHertz;

        /// Contact bounciness. Non-dimensional. You can speed up overlap
        /// recovery by decreasing this with the trade-off that overlap
        /// resolution becomes more energetic.
        Opt<f32> contactDampingRatio;

        /// This parameter controls how fast overlap is resolved and usually has
        /// units of meters per second. This only puts a cap on the resolution
        /// speed. The resolution speed is increased by increasing the hertz
        /// and/or decreasing the damping ratio.
        Opt<f32> maxContactPushSpeed;

        /// Maximum linear speed. Usually meters per second.
        Opt<f32> maximumLinearSpeed;

        /// Optional mixing callback for friction. The default uses
        /// sqrt(frictionA * frictionB).
        Opt<b2::FrictionCallback *> frictionCallback;

        /// Optional mixing callback for restitution. The default uses
        /// max(restitutionA, restitutionB).
        Opt<b2::RestitutionCallback *> restitutionCallback;

        /// Can bodies go to sleep to improve performance
        Opt<bool> enableSleep;

        /// Enable continuous collision
        Opt<bool> enableContinuous;

        /// Number of workers to use with the provided task system. Box2D
        /// performs best when using only performance cores and accessing a
        /// single L2 cache. Efficiency cores and hyper-threading provide little
        /// benefit and may even harm performance.
        /// @note Box2D does not create threads. This is the number of threads
        /// your applications has created that you are allocating to
        /// b2World_Step.
        /// @warning Do not modify the default value unless you are also
        /// providing a task system and providing task callbacks (enqueueTask
        /// and finishTask).
        Opt<int> workerCount;
        /// Function to spawn tasks
        Opt<b2::EnqueueTaskCallback *> enqueueTask;
        /// Function to finish a task
        Opt<b2::FinishTaskCallback *> finishTask;
        /// User context that is provided to enqueueTask and finishTask
        Opt<void *> userTaskContext;
        Opt<void *> userData;
    };

    static WorldImpl createWorld(const Options &options)
    {
        b2::WorldDef definition;
        if (options.gravity)
            definition.gravity = options.gravity.unwrap();
        if (options.restitutionThreshold)
            definition.restitutionThreshold =
                options.restitutionThreshold.unwrap();
        if (options.hitEventThreshold)
            definition.hitEventThreshold = options.hitEventThreshold.unwrap();
        if (options.contactHertz)
            definition.contactHertz = options.contactHertz.unwrap();
        if (options.contactDampingRatio)
            definition.contactDampingRatio =
                options.contactDampingRatio.unwrap();
        if (options.maxContactPushSpeed)
            definition.maxContactPushSpeed =
                options.maxContactPushSpeed.unwrap();
        if (options.maximumLinearSpeed)
            definition.maximumLinearSpeed = options.maximumLinearSpeed.unwrap();
        if (options.frictionCallback)
            definition.frictionCallback = options.frictionCallback.unwrap();
        if (options.restitutionCallback)
            definition.restitutionCallback =
                options.restitutionCallback.unwrap();
        if (options.enableSleep)
            definition.enableSleep = options.enableSleep.unwrap();
        if (options.enableContinuous)
            definition.enableContinuous = options.enableContinuous.unwrap();
        if (options.workerCount)
            definition.workerCount = options.workerCount.unwrap();
        if (options.enqueueTask)
            definition.enqueueTask = options.enqueueTask.unwrap();
        if (options.finishTask)
            definition.finishTask = options.finishTask.unwrap();
        if (options.userTaskContext)
            definition.userTaskContext = options.userTaskContext.unwrap();
        if (options.userData)
            definition.userData = options.userData.unwrap();

        return WorldImpl(b2::createWorld(definition));
    }

    Body createBody(const Options &options) NOEXCEPT
        requires(not isConst)
    {
        b2::BodyDef definition;
        if (options.type)
            definition.type =
                static_cast<decltype(definition.type)>(options.type.unwrap());
        if (options.position)
            definition.position = options.position.unwrap();
        if (options.rotation)
            definition.rotation = options.rotation.unwrap();
        if (options.linearVelocity)
            definition.linearVelocity = options.linearVelocity.unwrap();
        if (options.angularVelocity)
            definition.angularVelocity = options.angularVelocity.unwrap();
        if (options.linearDamping)
            definition.linearDamping = options.linearDamping.unwrap();
        if (options.angularDamping)
            definition.angularDamping = options.angularDamping.unwrap();
        if (options.gravityScale)
            definition.gravityScale = options.gravityScale.unwrap();
        if (options.sleepThreshold)
            definition.sleepThreshold = options.sleepThreshold.unwrap();
        if (options.enableSleep)
            definition.enableSleep = options.enableSleep.unwrap();
        if (options.isAwake)
            definition.isAwake = options.isAwake.unwrap();
        if (options.fixedRotation)
            definition.fixedRotation = options.fixedRotation.unwrap();
        if (options.isBullet)
            definition.isBullet = options.isBullet.unwrap();
        if (options.isEnabled)
            definition.isEnabled = options.isEnabled.unwrap();
        if (options.allowFastRotation)
            definition.allowFastRotation = options.allowFastRotation.unwrap();

        definition.name = options.name;
        definition.userData = options.userData;

        return Body(b2::createBody(id, definition));
    }

    void destroy() const NOEXCEPT
        requires(not isConst)
    {
        b2::destroyWorld(id);
    }

    /// Cast a capsule mover through the world. This is a special shape cast
    /// that handles sliding along other shapes while reducing clipping.
    [[nodiscard]] f32 castMover(const b2::Capsule &mover, Vec2 translation,
                                b2::QueryFilter filter) const NOEXCEPT
    {
        return b2::worldCastMover(id, mover, translation, filter);
    }

    struct RayCastOptions
    {
        /// The start point of the ray
        Vec2 origin = {};
        /// The translation of the ray from the start point to the end point
        Vec2 translation = {};
        /// Contains bit flags to filter unwanted shapes from the results
        b2::QueryFilter filter = {};
        /// A user implemented callback function
        b2::CastResultFunction *fcn = nullptr;
        /// A user context that is passed along to the callback function
        void *context = nullptr;
    };

    /// Cast a ray into the world to collect shapes in the path of the ray.
    /// Your callback function controls whether you get the closest point, any
    /// point, or n-points.
    /// @note The callback function may receive shapes in any order
    ///	@return traversal performance counters
    [[nodiscard]] b2::TreeStats
    castRay(const RayCastOptions &options) const NOEXCEPT
    {
        return b2::worldCastRay(id, options.origin, options.translation,
                                options.filter, options.context);
    }

    struct RayCastClosestOptions
    {
        /// The start point of the ray
        Vec2 origin = {};
        /// The translation of the ray from the start point to the end point
        Vec2 translation = {};
        /// Contains bit flags to filter unwanted shapes from the results
        b2::QueryFilter filter = {};
    };

    /// Cast a ray into the world to collect the closest hit. This is a
    /// convenience function. Ignores initial overlap. This is less general than
    /// castRay() and does not allow for custom filtering.
    [[nodiscard]] b2::RayResult
    castRayClosest(const RayCastClosestOptions &options) const NOEXCEPT
    {
        b2::worldCastRayClosest(id, options.origin, options.translation,
                                options.filter);
    }

    struct ShapeCastOptions
    {
        const b2::ShapeProxy *proxy = nullptr;
        Vec2 translation = {};
        b2::QueryFilter filter = {};
        b2::CastResultFunction *callback = nullptr;
        void *context = nullptr;
    };

    /// Cast a shape through the world. Similar to a cast ray except that a
    /// shape is cast instead of a point.
    ///	@see castRay
    [[nodiscard]] b2::TreeStats
    castShape(const ShapeCastOptions &options) const NOEXCEPT
    {
        b2::worldCastShape(id, *options.proxy, options.translation,
                           options.filter, options.function, options.context);
    }

    /// Enable/disable continuous collision between dynamic and static bodies.
    /// Generally you should keep continuous collision enabled to prevent fast
    /// moving objects from going through static objects. The performance gain
    /// from disabling continuous collision is minor.
    /// @see b2WorldDef
    WorldImpl setIsContinuousCollisionEnabled(bool isEnabledNow) const NOEXCEPT
        requires(not isConst)
    {
        b2::worldEnableContinuous(id, isEnabledNow);
        return *this;
    }

    /// Enable/disable sleep. If your application does not need sleeping, you
    /// can gain some performance by disabling sleep completely at the world
    /// level.
    /// @see b2WorldDef
    WorldImpl setIsSleepingEnabled(bool isEnabledNow) const NOEXCEPT
        requires(not isConst)
    {
        b2::worldEnableSleeping(id, isEnabledNow);
        return *this;
    }

    /// Adjust contact tuning parameters
    /// @param worldId The world id
    /// @param hertz The contact stiffness (cycles per second)
    /// @param dampingRatio The contact bounciness with 1 being critical damping
    /// (non-dimensional)
    /// @param pushSpeed The maximum contact constraint push out speed (meters
    /// per second)
    /// @note Advanced feature
    WorldImpl
    setContactTuning(const b2::ContactTuningOptions &options) const NOEXCEPT
        requires(not isConst)
    {
        b2::worldSetContactTuning(id, options);
        return *this;
    }

    /// Adjust the restitution threshold. It is recommended not to make this
    /// value very small because it will prevent bodies from sleeping. Usually
    /// in meters per second.
    /// @see b2WorldDef
    WorldImpl setRestitutionThreshhold(f32 value) const NOEXCEPT
        requires(not isConst)
    {
        b2::worldSetRestitutionThreshold(id, value);
        return *this;
    }

    /// Simulate a world for one time step. This performs collision detection,
    /// integration, and constraint solution.
    /// @param worldId The world to simulate
    /// @param timeStep The amount of time to simulate, this should be a fixed
    /// number. Usually 1/60.
    /// @param subStepCount The number of sub-steps, increasing the sub-step
    /// count can increase accuracy. Usually 4.
    void step(f32 timeStep = 1.0f / 60.f, u16 subStepCount = 4) const NOEXCEPT
        requires(not isConst)
    {
        b2::worldStep(id, timeStep, subStepCount);
    }
};
