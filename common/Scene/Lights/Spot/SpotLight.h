#pragma once

#include "common/Scene/Lights/Light.h"

class SpotLight : public Light
{
public:
    SpotLight(const float, const float);
    virtual void ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const override;
    virtual float ComputeLightAttenuation(glm::vec3 origin) const override;

    virtual void GenerateRandomPhotonRay(Ray& ray) const override;

private:
    float cos_t1;
    float cos_t2;
};
