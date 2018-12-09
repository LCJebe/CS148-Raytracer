#include "common/Scene/Lights/Point/PointLight.h"


void PointLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    output.emplace_back(origin, rayDirection, distanceToOrigin);
}

float PointLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    return 1.f;
}

void PointLight::GenerateRandomPhotonRay(Ray& ray) const
{
    // Assignment 8 TODO: Fill in the random point light samples here.
    const glm::vec4 pos = PointLight::GetPosition();
    const glm::vec3 rayPos = glm::vec3(pos.x, pos.y, pos.z);

    float x = 1.;
    float y = 1.;
    float z = 1.;
    while (x*x + y*y + z*z > 1){
        x = (float)std::rand() / RAND_MAX * 2 - 1;
        y = (float)std::rand() / RAND_MAX * 2 - 1;
        z = (float)std::rand() / RAND_MAX * 2 - 1;
    }
    glm::vec3 rayDir = glm::vec3(x, y, z);
    ray.SetRayPosition(rayPos);
    ray.SetRayDirection(rayDir);
}
