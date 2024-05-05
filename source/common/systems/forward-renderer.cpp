#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include "../components/light.hpp"
#include "../deserialize-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL;

            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);

            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);

            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glGenVertexArrays(1, &postProcessVertexArray);

            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        std::vector<LightComponent *> lights;

        for (auto entity : world->getEntities())
        {
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    opaqueCommands.push_back(command);
                }
            }
            if (auto lightComponent = entity->getComponent<LightComponent>(); lightComponent)
            {
                lights.push_back(lightComponent);
            }
        }

        if (camera == nullptr)
            return;

        auto owner = camera->getOwner();
        auto M = owner->getLocalToWorldMatrix();
        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = M * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);

        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  { return (glm::dot(cameraForward, first.center) > glm::dot(cameraForward, second.center)); });

        glm::mat4 P = camera->getProjectionMatrix(windowSize);
        glm::mat4 V = camera->getViewMatrix();
        glm::mat4 VP = P * V;

        glViewport(0, 0, windowSize.x, windowSize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0);
        glColorMask(true, true, true, true);
        glDepthMask(true);

        if (postprocessMaterial)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int lightsCount = 0;
        for (int i = 0; i < lights.size(); i++)
        {
            if (lights[i]->visible)
            {
                lightsCount++;
            }
        }

        for (auto command : opaqueCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", VP * command.localToWorld);

            for (int j = 0, k = 0; j < (int)lights.size(); j++)
            {
                if (!(lights[j]->visible))
                { // don't send this light if not visible
                    continue;
                }
                // TODO 11 check if correct - assumption: default light direction is bottom
                glm::vec3 position = lights[j]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);

                // TODO 11 should this be normalized???? - assuming default is down
                glm::vec4 directionVector = lights[j]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, -1, 0, 0);
                glm::vec3 direction = glm::vec3(directionVector.x, directionVector.y, directionVector.z);

                lights[j]->sendData(command.material->shader, k, direction, position); // k = index used by the shader
                k++;
            }

            command.mesh->draw();
        }

        if (this->skyMaterial)
        {
            this->skyMaterial->setup();
            glm::vec3 camPosition = eye;
            our::Transform skyTransform;
            skyTransform.position = camPosition;
            glm::mat4 skyModel = skyTransform.toMat4();

            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);

            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * skyModel);

            skySphere->draw();
        }

        for (auto command : transparentCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", VP * command.localToWorld);
            command.material->shader->set("light count", lightsCount);
            for (int j = 0, k = 0; j < (int)lights.size(); j++)
            {
                if (!(lights[j]->visible))
                {
                    continue;
                }
                // TODO 11 check if correct - assumption: default light direction is bottom
                glm::vec3 position = lights[j]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);

                // TODO 11 should this be normalized???? - assuming default is down
                glm::vec4 directionVector = lights[j]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, -1, 0, 0);
                glm::vec3 direction = glm::vec3(directionVector.x, directionVector.y, directionVector.z);

                lights[j]->sendData(command.material->shader, k, direction, position);
                k++;
            }
            command.mesh->draw();
        }

        if (postprocessMaterial)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            postprocessMaterial->setup();
            glBindVertexArray(this->postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }
}
