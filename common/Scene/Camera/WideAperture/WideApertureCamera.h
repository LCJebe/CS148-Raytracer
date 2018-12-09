#pragma once

#include "common/Scene/Camera/Perspective/PerspectiveCamera.h"

class WideApertureCamera : public PerspectiveCamera
{
public:
    // inputFov is in degrees. 
    WideApertureCamera(float aspectRatio, float inputFov, float focalDistance, float apertureRadius);
    virtual std::shared_ptr<class Ray> GenerateRayForNormalizedCoordinates(glm::vec2 coordinate) const override;

private:
    float f;
    float r;
};
