#include "common/Scene/Lights/Sphere/SphereLight.h"

SphereLight::SphereLight(float radius):
    samplesToUse(16), lightRadius(radius)
{
}

glm::vec3 sampleUnitSphere(){

    float u = (float)std::rand() / RAND_MAX * 2 -1; //uniform in [0, 1)
    float theta = (float)std::rand() / RAND_MAX *2 * PI; //uniform in [0, 2*PI)

    float x = std::sqrt(1-u*u) * std::cos(theta);
    float y = std::sqrt(1-u*u) * std::sin(theta);
    float z = u;

    glm::vec3 pos(x, y, z);

    return pos;
}

void SphereLight::ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const
{
    origin += normal * LARGE_EPSILON;
    for (int i = 0; i < samplesToUse; ++i) {

        const glm::vec3 lightCenter = glm::vec3(GetPosition());
        const glm::vec3 cRay = glm::normalize(lightCenter - origin); // ray TOWARDS light center
        glm::vec3 sample = sampleUnitSphere();

//        while (cRay.x*sample.x + cRay.y*sample.y + cRay.z*sample.z > 0){ // half space constraint
//            sample = sampleUnitSphere();
//        }

        glm::vec3 lightPosition = sample*lightRadius + lightCenter;

        const glm::vec3 rayDirection = glm::normalize(lightPosition - origin);
        const float distanceToOrigin = glm::distance(origin, lightPosition);
        output.emplace_back(origin, rayDirection, distanceToOrigin);
    }
}

float SphereLight::ComputeLightAttenuation(glm::vec3 origin) const
{
//    const glm::vec3 lightToPoint = glm::normalize(origin - glm::vec3(GetPosition()));
//    if (glm::dot(lightToPoint, glm::vec3(GetForwardDirection())) < -SMALL_EPSILON) {
//        return 0.f;
//    }
//    return 1.f / static_cast<float>(samplesToUse);
    return 1.f;
}


void SphereLight::GenerateRandomPhotonRay(Ray& ray) const
{
    // get random position on the sphere
    const glm::vec3 centerPos = glm::vec3(SphereLight::GetPosition());
    const glm::vec3 sample = sampleUnitSphere();

    // get ray positon from sample
    const glm::vec3 rayPos = sample*lightRadius + centerPos;

    // get random direction (has to be in halfspace defined bu tangent plane....
    // normal is defined by sample. check dot product is positive
    float x = 1.;
    float y = 1.;
    float z = 1.;
//    while ((x*x + y*y + z*z > 1) && (x*sample.x + y*sample.y && z*sample.z < 0)){ // with half space constraint
    while (x*x + y*y + z*z > 1){ // without half space constraint
        x = (float)std::rand() / RAND_MAX * 2 - 1;
        y = (float)std::rand() / RAND_MAX * 2 - 1;
        z = (float)std::rand() / RAND_MAX * 2 - 1;
    }

    glm::vec3 rayDir = glm::vec3(x, y, z);
    ray.SetRayPosition(rayPos);
    ray.SetRayDirection(rayDir);
}
