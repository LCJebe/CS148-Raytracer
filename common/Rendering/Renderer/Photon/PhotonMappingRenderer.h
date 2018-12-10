#pragma once

#include "common/Rendering/Renderer.h"
#include "common/Rendering/Renderer/Photon/Photon.h"
#include <kdtree++/kdtree.hpp>
#include <functional>
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Renderer/Backward/BackwardRenderer.h"

class PhotonMappingRenderer : public BackwardRenderer
{
public:
    PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler);
    virtual void InitializeRenderer() override;
    glm::vec3 ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, int sampleIdx) const;

    void SetNumberOfDiffusePhotons(int diffuse);

    void setPerspectiveCamera(std::shared_ptr<class PerspectiveCamera> cam);
private:
    using PhotonKdtree = KDTree::KDTree<3, Photon, PhotonAccessor>;
    PhotonKdtree diffuseMap;

    int diffusePhotonNumber;
    int maxPhotonBounces;
    int minPhotonBounces;
    uint targetPhotonCount;
    std::shared_ptr<class PerspectiveCamera> pCamera;

    void GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons);
    void TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, int& path, float currentIOR, int remainingBounces);
};
