#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

// This component includes properties that are used to define a light
// You can customize these properties to create a specific lighting
// This light component includes properties for color, attenuation, cone angles, and type
namespace our
{

    class LightComponent : public Component
    {
    public:
        int type;
        glm::vec3 color;
        glm::vec3 attenuation;
        glm::vec2 cone_angles;
        // The position and direction of the light are not included in the component,
        // as they will be calculated from the entity component
        // Reads light parameters from the given json object
        void deserialize(const nlohmann::json &data) override;
        // identify it is a light component
        static std::string getID() { return "light"; }
    };
}