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
        bool hasJump = 0;

    public:
        void enter(Application *app)
        {
            this->app = app;
        }
        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World *world, float deltaTime, ForwardRenderer *renderer)
        {

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

                if (app->getKeyboard().isPressed(GLFW_KEY_SPACE) && (entity->localTransform.position.y == 0.5 || hasJump))
                    gravity += movement->jumpSensitivity * (1 + hasJump * movement->boostSensitivity), hasJump = 0;
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
                entity->localTransform.position.y = std::max(entity->localTransform.position.y + gravity, 0.5f);
                if (entity->localTransform.position.y <= 0.5f)
                    gravity = 0;
                entity->localTransform.position += front * accelerationFront;
                entity->localTransform.position += right * accelerationSide;

                MeshRendererComponent *MeshRenderMonkey = entity->getComponent<MeshRendererComponent>();
                // cout << "size is" << MeshRender->mesh->vertices.size() << '\n';
                glm::mat4 localToWorldMat = entity->getLocalToWorldMatrix();
                float monkeyMinX = 1e9, monkeyMaxX = -1e9;
                float monkeyMinY = 1e9, monkeyMaxY = -1e9;
                float monkeyMinZ = 1e9, monkeyMaxZ = -1e9;
                // for (auto i : localToWorldMat.value)
                // {
                //     for (int j = 0; j < 4; j++)
                //         cout << i[j] << ' ';
                //     cout << '\n';
                // }
                // cout << "fff\n";
                for (auto point : MeshRenderMonkey->mesh->vertices)
                {
                    glm::vec4 worldPoint = localToWorldMat * glm::vec4(point, 1.0f);

                    monkeyMinX = min(monkeyMinX, worldPoint.x);
                    monkeyMaxX = max(monkeyMaxX, worldPoint.x);
                    monkeyMinY = min(monkeyMinY, worldPoint.y);
                    monkeyMaxY = max(monkeyMaxY, worldPoint.y);
                    monkeyMinZ = min(monkeyMinZ, worldPoint.z);
                    monkeyMaxZ = max(monkeyMaxZ, worldPoint.z);
                    // cout << "origin is" << point.x << ' ' << point.z << '\n';
                    // cout << "point is" << worldPoint.x << ' ' << worldPoint.z << '\n';
                }
                float monkeyCetnerX = (monkeyMinX + monkeyMaxX) / 2;
                float monkeyCetnerZ = (monkeyMinZ + monkeyMaxZ) / 2;
                // monkeyCenter = monkeyCenter * entity->getLocalToWorldMatrix;
                // check floor detection
                // cerr << entity->localTransform.position.y << '\n';

                // cerr << "checking" << '\n';
                int cnt = 0;
                bool colidePlane = 0, colideSpike = 0;
                if (fabs(entity->localTransform.position.y - 0.5f) < 0.1 && app->getKeyboard().isPressed(GLFW_KEY_Q))
                {

                    for (auto entity2 : world->getEntities())
                    {
                        // cerr << entity2->name << '\n';
                        if (entity2->name == "plane" || entity2->name == "spikes")
                        {
                            cnt++;
                            // cout << "start\n";
                            MeshRendererComponent *MeshRenderPlane = entity2->getComponent<MeshRendererComponent>();
                            // cout << "size is" << MeshRender->mesh->vertices.size() << '\n';
                            glm::mat4 localToWorldMat = entity2->getLocalToWorldMatrix();
                            float planeMinX = 1e9, planeMaxX = -1e9;
                            float planeMinZ = 1e9, planeMaxZ = -1e9;
                            // for (auto i : localToWorldMat.value)
                            // {
                            //     for (int j = 0; j < 4; j++)
                            //         cout << i[j] << ' ';
                            //     cout << '\n';
                            // }
                            // cout << "fff\n";
                            for (auto point : MeshRenderPlane->mesh->vertices)
                            {
                                glm::vec4 worldPoint = localToWorldMat * glm::vec4(point, 1.0f);

                                planeMinX = min(planeMinX, worldPoint.x);
                                planeMaxX = max(planeMaxX, worldPoint.x);
                                planeMinZ = min(planeMinZ, worldPoint.z);
                                planeMaxZ = max(planeMaxZ, worldPoint.z);
                                // cout << "origin is" << point.x << ' ' << point.z << '\n';
                                // cout << "point is" << worldPoint.x << ' ' << worldPoint.z << '\n';
                            }
                            glm::vec4 worldPos = glm::vec4(entity2->localTransform.position, 1.0f) * entity2->getLocalToWorldMatrix();
                            // cout<<"wolrd "<<worldPos.x<<' '<<worldPos.y<<' '<<worldPos.z<<'\n';
                            // cout << (planeMinX <= monkeyCenter.x) << (monkeyCenter.x <= planeMaxX) << (monkeyCenter.z <= planeMinZ) << (monkeyCenter.z <= planeMaxZ);
                            // cerr << "monkey " << monkeyCetnerX << ' ' << monkeyCetnerZ << ' ' << "planeMin " << planeMinX << ' ' << planeMinZ << ' ' << "planeMax " << planeMaxX << ' ' << planeMaxZ << '\n';

                            if (planeMinX <= monkeyCetnerX && monkeyCetnerX <= planeMaxX && planeMinZ <= monkeyCetnerZ && monkeyCetnerZ <= planeMaxZ)
                            {
                                hasJump = 0;
                                colidePlane |= entity2->name == "plane";
                                if (colidePlane)
                                    break;
                                colideSpike |= entity2->name == "spikes";
                                cerr << "collision with " << entity2->name << '\n';
                            }
                        }
                    }
                    if (!colidePlane)
                    {
                        cout << "No PLane" << '\n';
                        // app->changeState("menu");
                        // break;
                    }
                }

                // collision detection with objects
                if (app->getKeyboard().isPressed(GLFW_KEY_E))
                    for (auto object : world->getEntities())
                    {
                        int berries = world->getBerries();
                        if (object->name != ("moon" + to_string(berries)))
                            continue;
                        MeshRendererComponent *objectMeshRender = object->getComponent<MeshRendererComponent>();
                        // cout << "size is" << MeshRender->mesh->vertices.size() << '\n';
                        glm::mat4 objectLocalToWorldMat = object->getLocalToWorldMatrix();
                        float objectMinX = 1e9, objectMaxX = -1e9;
                        float objectMinY = 1e9, objectMaxY = -1e9;
                        float objectMinZ = 1e9, objectMaxZ = -1e9;
                        // for (auto i : localToWorldMat.value)
                        // {
                        //     for (int j = 0; j < 4; j++)
                        //         cout << i[j] << ' ';
                        //     cout << '\n';
                        // }
                        // cout << "fff\n";
                        for (auto point : objectMeshRender->mesh->vertices)
                        {
                            glm::vec4 worldPoint = objectLocalToWorldMat * glm::vec4(point, 1.0f);

                            objectMinX = min(objectMinX, worldPoint.x);
                            objectMaxX = max(objectMaxX, worldPoint.x);
                            objectMinY = min(objectMinY, worldPoint.y);
                            objectMaxY = max(objectMaxY, worldPoint.y);
                            objectMinZ = min(objectMinZ, worldPoint.z);
                            objectMaxZ = max(objectMaxZ, worldPoint.z);
                            // cout << "origin is" << point.x << ' ' << point.z << '\n';
                            // cout << "point is" << worldPoint.x << ' ' << worldPoint.z << '\n';
                        }
                        // cout << "monkey min" << monkeyMinX << ' ' << monkeyMinY << ' ' << monkeyMinZ << '\n';
                        // cout << "monkey max" << monkeyMaxX << ' ' << monkeyMaxY << ' ' << monkeyMaxZ << '\n';
                        // cout << "object max" << objectMinX << ' ' << objectMinY << ' ' << objectMinZ << '\n';
                        // cout << "object max" << objectMinX << ' ' << objectMinY << ' ' << objectMinZ << '\n';
                        if (monkeyMinX <= objectMaxX &&
                            monkeyMaxX >= objectMinX &&
                            monkeyMinY <= objectMaxY &&
                            monkeyMaxY >= objectMinY &&
                            monkeyMinZ <= objectMaxZ &&
                            monkeyMaxZ >= objectMinZ)
                        {
                            hasJump = true;
                            cout << "collision with moon babyyy\n", world->markForRemoval(object), world->deleteMarkedEntities(), world->addBerry();
                            for (auto newBerry : world->getEntities())
                            {
                                if (newBerry->name != ("moon" + to_string(berries + 1)))
                                    continue;
                                newBerry->localTransform.scale = glm::vec3(0.25, 0.25, 0.25);
                            }
                        }
                    }
            }
        }
    };
}
