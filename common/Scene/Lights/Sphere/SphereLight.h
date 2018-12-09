#pragma once

#include "common/Scene/Lights/Light.h"
#include "common/Sampling/Jitter/JitterColorSampler.h"

class SphereLight : public Light
{
public:
    SphereLight(float radius);

    virtual void ComputeSampleRays(std::vector<Ray>& output, glm::vec3 origin, glm::vec3 normal) const override;
    virtual float ComputeLightAttenuation(glm::vec3 origin) const override;

    virtual void GenerateRandomPhotonRay(Ray& ray) const override;

private:
    int samplesToUse;
    float lightRadius;
};
