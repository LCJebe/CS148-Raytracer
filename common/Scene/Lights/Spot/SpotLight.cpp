#include "common/Scene/Lights/Spot/SpotLight.h"

SpotLight::SpotLight(const float theta1, const float theta2):
    cos_t1(std::cos(theta1)), cos_t2(std::cos(theta2))
{
}



void SpotLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
    const float distanceToOrigin = glm::distance(origin, lightPosition);
    output.emplace_back(origin, rayDirection, distanceToOrigin);
}

float SpotLight::ComputeLightAttenuation(glm::vec3 origin) const
{
    // check if point is visible using t1 and t2. Therefor get angle between normal ("forward") and ray.
    const glm::vec3 lightPosition = glm::vec3(GetPosition());
    const glm::vec3 rayDirection = glm::normalize(lightPosition - origin); // pointing TOWARDS light

    glm::vec3 normal = glm::vec3(SpotLight::GetForwardDirection());

    float cos_t = - normal.x * rayDirection.x - normal.y * rayDirection.y - normal.z * rayDirection.z;

    float attenuation = 0.f;
    if (cos_t > cos_t1){
        attenuation = 1.0f;
    }
    else if (cos_t > cos_t2){
        attenuation = (cos_t - cos_t2) / (cos_t1 - cos_t2);
    }

    return attenuation;
}

void SpotLight::GenerateRandomPhotonRay(Ray& ray) const
{
    // Assignment 8 TODO: Fill in the random point light samples here.
    const glm::vec4 pos = SpotLight::GetPosition();
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
