module;

#include "macros.h"

export module physics;

import box2d;
import aliases;
import opt;

export class Body
{
  private:
    b2::BodyID id;

  public:
    struct Options
    {
        Opt<b2::BodyType> type;
        Opt<b2::Vec2> position;
        Opt<b2::Rotation> rotation;
        Opt<b2::Vec2> linearVelocity;
        Opt<f32> angularVelocity;
        Opt<f32> linearDamping;
        Opt<f32> angularDamping;
        Opt<f32> gravityScale;
        Opt<f32> sleepThreshold;
        /// Optional body name for debugging. Up to 31 characters (excluding
        /// null termination)
        const char *name = nullptr;
        void *userData = nullptr;
        Opt<bool> enableSleep;
        Opt<bool> isAwake;
        Opt<bool> fixedRotation;
        Opt<bool> isBullet;
        Opt<bool> isEnabled;
        Opt<bool> allowFastRotation;
    };

  private:
    constexpr explicit Body(b2::BodyID id) NOEXCEPT : id(id) {}

  public:
    Body() = delete;

    constexpr static Body createBody(b2::WorldID world,
                                     const Options &options) NOEXCEPT
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

        b2::BodyID body = b2::createBody(world, definition);
        return Body(body);
    }

    constexpr operator b2::BodyID() const NOEXCEPT { return id; }

    constexpr void destroy() NOEXCEPT { b2::destroyBody(id); }
};
