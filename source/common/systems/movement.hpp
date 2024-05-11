#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../application.hpp"
#include "../components/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include <iostream>
#include <algorithm>
using namespace std;
namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/components/movement.hpp"
    class MovementSystem
    {
        Application *app; // The application in which the state runs
        float gravity = 0;

    public:
        void enter(Application *app)
        {
            this->app = app;
        }
        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World *world, float deltaTime, ForwardRenderer *renderer)
        {
            // For each entity in the world
            Entity *camera = nullptr;
            for (auto entity1 : world->getEntities())
            {
                if (entity1->name == "camera")
                    camera = entity1;
            }
            for (auto entity1 : world->getEntities())
            {
                bool collisionHappened = false;

                // Get the movement component if it exists
                MovementComponent *movement = entity1->getComponent<MovementComponent>();
                if (!(movement))
                    continue;
                // get the owner entity of this movement component
                Entity *entity = movement->getOwner();
                glm::vec3 &entityPosition = entity->localTransform.position;
                glm::vec3 &entityRotation = entity->localTransform.rotation;

                // get the model matrix for the current entity relative to its parent
                glm::mat4 matrix = entity->localTransform.toMat4();
                glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, 1, 0)),
                          up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)),
                          right = glm::vec3(matrix * glm::vec4(-1, 0, 0, 0));
                // get the movement configurations
                // float &maxSpeed = movement->maxSpeed;
                // float &maxAngle = movement->maxAngle;

                // float &acceleration = movement->acceleration;
                // float &rotation = movement->rotation;

                // float &accelerationSensitivity = movement->accelerationSensitivity;
                // float &brakesSensitivity = movement->brakesSensitivity;
                // float &rotationSensitivity = movement->rotationSensitivity;
                // Change the position and rotation based on the linear & angular velocity and delta time.
                float accelerationFront = 0;
                float accelerationSide = 0;
                if (app->getKeyboard().isPressed(GLFW_KEY_W))
                    accelerationFront += deltaTime * movement->speed;
                if (app->getKeyboard().isPressed(GLFW_KEY_S))
                    accelerationFront -= deltaTime * movement->speed;
                if (app->getKeyboard().isPressed(GLFW_KEY_D))
                    accelerationSide += deltaTime * movement->speed;
                if (app->getKeyboard().isPressed(GLFW_KEY_A))
                    accelerationSide -= deltaTime * movement->speed;
                if (app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT))
                    accelerationFront *= 2, accelerationSide *= 2;

                if (app->getKeyboard().isPressed(GLFW_KEY_SPACE) && entity->localTransform.position.y == -0.0f)
                    gravity += movement->jumpSensitivity;
                // front[0]=0;

                front[1] = 0;
                // front[2]=0;
                // cerr<<front[0]<<' '<<front[1]<<' '<<front[2]<<'\n';
                // entity->localTransform.rotation+=glm::vec3(0.01,0,0.01);
                glm::vec2 delta = app->getMouse().getMouseDelta();
                // camera->localTransform.rotation.x -= delta.y * 0.01; // The y-axis controls the pitch
                entity->localTransform.rotation.y -= delta.x * movement->rotationSensitivity; // The y-axis controls the pitch
                // cerr << gravity << ' ' << camera->localTransform.position.y << '\n';
                // entity->localTransform.rotation.y = camera->localTransform.rotation.y+glm::pi<float>(); // The x-axis controls the yaw
                // camera->localTransform.position += front * acceleration;
                gravity -= movement->gravitySensitivity;
                // entity->localTransform.position.y=std::max(entity->localTransform.position.y+gravity,0.0f);
                entity->localTransform.position.y = std::max(entity->localTransform.position.y + gravity, 0.0f);
                if (entity->localTransform.position.y <= -0.0f)
                    gravity = 0;
                entity->localTransform.position += front * accelerationFront;
                entity->localTransform.position += right * accelerationSide;
            }
        }
    };
}
