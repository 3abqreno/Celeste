#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our
{

    // This component denotes that the MovementSystem will move the owning entity by a certain linear and angular velocity.
    // This component is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/systems/movement.hpp"
    // For a more complex example of how to use the ECS framework, see "free-camera-controller.hpp"
    class MovementComponent : public Component
    {
    public:
        float speed = 1;
        float jumpSensitivity = 1;
        float gravitySensitivity = 0.01f;
        float rotationSensitivity = 0.01f;
        // The ID of this component type is "Movement"
        static std::string getID() { return "Movement"; }

        // Reads linearVelocity & angularVelocity from the given json object
        void deserialize(const nlohmann::json &data) override;
    };

}