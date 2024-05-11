#include "movement.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our
{
    // Reads linearVelocity & angularVelocity from the given json object
    void MovementComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        speed = data.value("speed", speed);
        jumpSensitivity = data.value("jumpSensitivity", jumpSensitivity);
        gravitySensitivity = data.value("gravitySensitivity", gravitySensitivity);
        rotationSensitivity = data.value("rotationSensitivity", rotationSensitivity);
    }
}