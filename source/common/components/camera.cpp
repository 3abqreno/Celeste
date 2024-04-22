#include "camera.hpp"
#include "../ecs/entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace our
{
    // Reads camera parameters from the given json object
    void CameraComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        std::string cameraTypeStr = data.value("cameraType", "perspective");
        if (cameraTypeStr == "orthographic")
        {
            cameraType = CameraType::ORTHOGRAPHIC;
        }
        else
        {
            cameraType = CameraType::PERSPECTIVE;
        }
        near = data.value("near", 0.01f);
        far = data.value("far", 100.0f);
        fovY = data.value("fovY", 90.0f) * (glm::pi<float>() / 180);
        orthoHeight = data.value("orthoHeight", 1.0f);
    }

    // Creates and returns the camera view matrix
    glm::mat4 CameraComponent::getViewMatrix() const
    {
        auto owner = getOwner();
        auto M = owner->getLocalToWorldMatrix();
        // TODO: (Req 8) Complete this function
        // HINT:
        //  In the camera space:
        //  - eye is the origin (0,0,0)
        //  - center is any point on the line of sight. So center can be any point (0,0,z) where z < 0. For simplicity, we let center be (0,0,-1)
        //  - up is the direction (0,1,0)
        //  but to use glm::lookAt, we need eye, center and up in the world state.
        //  Since M (see above) transforms from the camera to thw world space, you can use M to compute:
        //  - the eye position which is the point (0,0,0) but after being transformed by M
        //  - the center position which is the point (0,0,-1) but after being transformed by M
        //  - the up direction which is the vector (0,1,0) but after being transformed by M
        //  then you can use glm::lookAt
        glm::vec4 eyeWorld = glm::vec4(0, 0, 0, 1); // Eye is at the origin (0, 0, 0) in camera space
        glm::vec4 centerWorld = glm::vec4(0, 0, -1, 1);
        glm::vec4 upWorld = glm::vec4(0, 1, 0, 0);
        // Create the view matrix using glm::lookAt
        return glm::lookAt(glm::vec3(M * eyeWorld),
                           glm::vec3(M * centerWorld),
                           glm::vec3(M * upWorld));
    }

    // Creates and returns the camera projection matrix
    // "viewportSize" is used to compute the aspect ratio
    glm::mat4 CameraComponent::getProjectionMatrix(glm::ivec2 viewportSize) const
    {
        // TODO: (Req 8) Wrtie this function
        //  NOTE: The function glm::ortho can be used to create the orthographic projection matrix
        //  It takes left, right, bottom, top. Bottom is -orthoHeight/2 and Top is orthoHeight/2.
        //  Left and Right are the same but after being multiplied by the aspect ratio
        //  For the perspective camera, you can use glm::perspective
        float aspectRatio = float(viewportSize.x) / float(viewportSize.y);

        // Left is  (-orthoHeight / 2) * aspectRatio.
        float Left = (-orthoHeight / 2) * aspectRatio;
        // Left is  (orthoHeight / 2) * aspectRatio.
        float Right = (orthoHeight / 2) * aspectRatio;
        // Bottom is -orthoHeight/2.
        float Bottom = -orthoHeight / 2;
        // Top is orthoHeight/2.
        float Top = orthoHeight / 2;

        // Defining an Orthographic Projection
        if (cameraType == CameraType::ORTHOGRAPHIC){
            // The ortho() function typically takes four parameters: left, right, bottom, top.
            // The ortho() function uses these parameters to create a 4x4 projection matrix that 
            // can be used to transform the coordinates of the 3D objects into 2D coordinates on the screen.
            glm::mat4 orthographic = glm::ortho(
            Left,
            Right,
            Bottom,
            Top
            );
            return orthographic;

        }
        // Defining a Perspective Projection
        else
        {
            // The perspective() function typically takes four parameters: fovy, aspect, near, far.
            // The perspective() function uses these parameters to create a 4x4 projection matrix that
            // can be used to transform the coordinates of the 3D objects into 2D coordinates on the screen, with the appropriate perspective distortion.
            glm::mat4 projection = glm::perspective(
                // The vertical field of view angle (smaller means more zoom in)
                fovY,
                //aspect ratio (width ÷ height of the window)
                aspectRatio,
                //near plane relative to camera
                near,
                // far plane relative to camera
                far);
            return  projection;
        }
    }
}