#include "common/Scene/Camera/WideAperture/WideApertureCamera.h"
#include "common/Scene/Geometry/Ray/Ray.h"

WideApertureCamera::WideApertureCamera(float aspectRatio, float inputFov, float focalDistance, float apertureRaduis):
    PerspectiveCamera(aspectRatio, inputFov),
    f(focalDistance),
    r(apertureRaduis)
{
}

std::shared_ptr<Ray> WideApertureCamera::GenerateRayForNormalizedCoordinates(glm::vec2 coordinate) const
{
    // Sample a random position on the aperture (uniform, circular, radius r)
    const float phi = (float)std::rand()/RAND_MAX * 2 * PI;
    const float rho = r * std::sqrt((float)std::rand()/RAND_MAX);

    // in Cartesian coordinates
    const float x = rho * cos(phi);
    const float y = rho * sin(phi);

    // Send ray from the camera (aperture) to the image plane -- make the assumption that the image plane is at z = 1 in camera space.
    const glm::vec3 cameraOrigin = glm::vec3(GetPosition());
    const glm::vec3 rayOrigin = glm::vec3(GetPosition()) + glm::vec3(GetRightDirection()) * x + glm::vec3(GetUpDirection()) * y;

    // Figure out where the ray is supposed to point to. 
    // Imagine that a frustum exists in front of the camera (which we assume exists at a singular point).
    // Then, given the aspect ratio and vertical field of view we can determine where in the world the 
    // image plane will exist and how large it is assuming we know for sure that z = 1 (this is fairly arbitrary for now).
    const float planeHeight = std::tan(fov / 2.f) * 2.f;
    const float planeWidth = planeHeight * aspectRatio;

    // Assume that (0, 0) is the top left of the image which means that when coordinate is (0.5, 0.5) the 
    // pixel is directly in front of the camera...
    const float xOffset = planeWidth * (coordinate.x - 0.5f);
    const float yOffset = -1.f * planeHeight  * (coordinate.y - 0.5f);
    const glm::vec3 focalTarget = cameraOrigin + f*(glm::vec3(GetForwardDirection()) + glm::vec3(GetRightDirection()) * xOffset + glm::vec3(GetUpDirection()) * yOffset);

    const glm::vec3 rayDirection = glm::normalize(focalTarget - rayOrigin);
    return std::make_shared<Ray>(rayOrigin + rayDirection * zNear, rayDirection, zFar - zNear);
}
