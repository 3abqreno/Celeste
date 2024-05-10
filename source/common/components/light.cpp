
#include "light.hpp"
#include "../deserialize-utils.hpp"
#include "./component-deserializer.hpp"
#include <glm/gtx/euler_angles.hpp>
#include <iostream>

namespace our
{
    // Deserializes the light data from a json object
    void LightComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        color = glm::vec3(data.value("color", glm::vec3(0, 0, 1)));
        std::string typeStr = data.value("lighttype", "DIRECTIONAL");
        if (typeStr == "DIRECTIONAL")
            type = 0;

        else if (typeStr == "POINT")
            type = 1;

        else if (typeStr == "SPOT")
            type = 2;

        if (type != 0)
        {
            attenuation = glm::vec3(data.value("attenuation", glm::vec3(1, 1, 1)));
        }

        if (type == 2)
        {
            cone_angles.x = glm::radians((float)data.value("cone_angles.in", 10));
            cone_angles.y = glm::radians((float)data.value("cone_angles.out", 80));
        }
    }

}