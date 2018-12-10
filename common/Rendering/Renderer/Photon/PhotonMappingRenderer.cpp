#include "common/Rendering/Renderer/Photon/PhotonMappingRenderer.h"
#include "common/Scene/Scene.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Scene/Lights/Light.h"
#include "common/Scene/Geometry/Primitives/Primitive.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Scene/SceneObject.h"
#include "common/Scene/Geometry/Mesh/MeshObject.h"
#include "common/Rendering/Material/Material.h"
#include "glm/gtx/component_wise.hpp"
#include "common/Scene/Camera/Perspective/PerspectiveCamera.h"

#define VISUALIZE_PHOTON_MAPPING 0

PhotonMappingRenderer::PhotonMappingRenderer(std::shared_ptr<class Scene> scene, std::shared_ptr<class ColorSampler> sampler):
    BackwardRenderer(scene, sampler),
    diffusePhotonNumber(5000000),
    maxPhotonBounces(1000),
    minPhotonBounces(1), // normal is one: only don't count the first direct hit (= zero bounces)
    targetPhotonCount(10)
{
    srand(static_cast<unsigned int>(time(NULL)));
}

void PhotonMappingRenderer::InitializeRenderer()
{
    // Generate Photon Maps
    std::cout << "Tracing " << diffusePhotonNumber << " Photons..." << std::endl;
    GenericPhotonMapGeneration(diffuseMap, diffusePhotonNumber);
    diffuseMap.optimise();
    std::cout << diffuseMap.size() << " Photon bounces recorded in Photonmap" << std::endl;
    std::cout << "Photon Mapping Finished" << std::endl;
}

// kept simple: assumes spherical FOV
bool checkFrustrum(glm::vec3 point, std::shared_ptr<Camera> camera){
    glm::vec3 n = glm::vec3(camera->GetForwardDirection());

    glm::vec3 cameraRay = point - glm::vec3(camera->GetPosition()); // points from camera to point

    float normDot = glm::dot(n, cameraRay) / glm::length(n) / glm::length(cameraRay);
    float angle = std::acos(normDot); // angle between camera to obect ray and camera normal / forward direction

    // convert pointer from Camera to PerspectiveCamera to acces GetFov()
    std::shared_ptr<PerspectiveCamera> pCamera = std::dynamic_pointer_cast<PerspectiveCamera> (camera);
    return (angle < pCamera->GetFov());
}

void PhotonMappingRenderer::GenericPhotonMapGeneration(PhotonKdtree& photonMap, int totalPhotons)
{
    float totalLightIntensity = 0.f;
    size_t totalLights = storedScene->GetTotalLights();
    std::cout << "Scene has " << totalLights << " lights" << std::endl;
    for (size_t i = 0; i < totalLights; ++i) {
        const Light* currentLight = storedScene->GetLightObject(i);
        if (!currentLight) {
            continue;
        }
        totalLightIntensity += glm::length(currentLight->GetLightColor());
    }

    // Shoot photons -- number of photons for light is proportional to the light's intensity relative to the total light intensity of the scene.
    while (photonMap.size() < targetPhotonCount){
        for (size_t i = 0; i < totalLights; ++i) {
            const Light* currentLight = storedScene->GetLightObject(i);
            if (!currentLight) {
                continue;
            }

            const float proportion = glm::length(currentLight->GetLightColor()) / totalLightIntensity;
            const int totalPhotonsForLight = static_cast<const int>(proportion * totalPhotons);
            const glm::vec3 photonIntensity = currentLight->GetLightColor() / static_cast<float>(totalPhotonsForLight);
            // I wonder whether this is easily parallelizable... but looks fine. No shared variables. Works!
            #pragma omp parallel for
            for (int j = 0; j < totalPhotonsForLight; ++j) {
                Ray photonRay;
                int path;
                path = 1;
                currentLight->GenerateRandomPhotonRay(photonRay);
                TracePhoton(photonMap, &photonRay, photonIntensity, path, 1.f, maxPhotonBounces);
            }
        }
    }
}

void PhotonMappingRenderer::TracePhoton(PhotonKdtree& photonMap, Ray* photonRay, glm::vec3 lightIntensity, int& path, float currentIOR, int remainingBounces)
{
    /*
     * Assignment 8 TODO: Trace a photon into the scene and make it bounce.
     *
     *    How to insert a 'Photon' struct into the photon map.
     *        Photon myPhoton;
     *        ... set photon properties ...
     *        photonMap.insert(myPhoton);
     */

    // terminate tracing if maximum bounces are reached
    if (remainingBounces < 0){
        return;
    }

    assert(photonRay);
    IntersectionState state(0, 0);
    state.currentIOR = currentIOR;

    // test intersection of ray with scene
    const bool intersect = storedScene->Trace(photonRay, &state);

    // terminate tracing if there is no intersection
    if (!intersect){
        return;
    }

    // get intersection point, we'll need it a couple more times
    const glm::vec3 intersectionPoint = state.intersectionRay.GetRayPosition(state.intersectionT);

    // store photon only if it doesn't come directly from the light (path > 1)
    if (path > minPhotonBounces){
        // create photon
        Photon photon;

        // set position of photon as the intersection point of the light ray
        photon.position = intersectionPoint;

        // intensity is fixed
        photon.intensity = lightIntensity;

        // the toLightRay is a ray pointing in the opposite direction as photonRay
        Ray lightRay(intersectionPoint, -photonRay->GetRayDirection());
        photon.toLightRay = lightRay;

        // insert the newly created photon into the map, only if in FOV
        if (checkFrustrum(intersectionPoint, pCamera)){
            photonMap.insert(photon);
        }

    }

    // get the material that we hit to determine reflection or absorption
    const MeshObject* hitMeshObject = state.intersectedPrimitive->GetParentMeshObject();
    const Material* hitMaterial = hitMeshObject->GetMaterial();

    // get the diffuse reflection of the material and calculate reflection probablilty
    const glm::vec3 diffuseReflection = hitMaterial->GetBaseDiffuseReflection();
    const float Pr = glm::max(glm::max(diffuseReflection.x, diffuseReflection.y), diffuseReflection.z);

    // generate random number to see whether we relfect or absorb
    const float thresh = (float)std::rand()/RAND_MAX;
    if (thresh < Pr){
        // scatter photon (for diffuese: in random direction)
        // generate two random numbers for the direction of the scattered ray (sample on disk and transform to hemisphere)
        const float u1 = (float)std::rand()/RAND_MAX;
        const float u2 = (float)std::rand()/RAND_MAX;

        const float r = std::sqrt(u1);
        const float theta = 2.f*PI*u2;

        glm::vec3 rayDirection;
        rayDirection.x = r * std::cos(theta);
        rayDirection.y = r * std::sin(theta);
        rayDirection.z = std::sqrt(1.f-u1);

        // normalize ray direction
        rayDirection = glm::normalize(rayDirection);

        // now transform from tangent space to world space
        const glm::vec3 n = state.ComputeNormal();
        glm::vec3 t;
        glm::vec3 b;


        if (std::fabs(glm::dot(n, glm::vec3(1.f, 0.f, 0.f))) < 0.8f){
            t = glm::cross(n, glm::vec3(1.f, 0.f, 0.f));
            b = glm::cross(n, t);
        }
        else if (std::fabs(glm::dot(n, glm::vec3(0.f, 1.f, 0.f))) < 0.8f){
            t = glm::cross(n, glm::vec3(0.f, 1.f, 0.f));
            b = glm::cross(n, t);
        }
        else {
            t = glm::cross(n, glm::vec3(0.f, 0.f, 1.f));
            b = glm::cross(n, t);
        }

        // normalize t and b
        t = glm::normalize(t);
        b = glm::normalize(b);

        // create transform matrix
        const glm::mat3 T = glm::mat3(t, b, n);
        rayDirection = T*rayDirection;

        // now create corresponding ray and trace it again
        Ray nextRay = Ray(intersectionPoint, rayDirection);

        // add an element to the path to grow its size
        path++;

        // trace recursively
        TracePhoton(photonMap, &nextRay, lightIntensity, path, currentIOR, remainingBounces-1);
    }
}

glm::vec3 PhotonMappingRenderer::ComputeSampleColor(const struct IntersectionState& intersection, const class Ray& fromCameraRay, int sampleIdx) const
{
    glm::vec3 finalRenderColor = BackwardRenderer::ComputeSampleColor(intersection, fromCameraRay, 0);
    // enable this to only show the conribution of photon mapping
    // finalRenderColor = {0.f, 0.f, 0.f};


    // only do photon mapping for the first 2 samples! should be enough.
    if (sampleIdx >= 2){
        return finalRenderColor;
    }

#if VISUALIZE_PHOTON_MAPPING
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    const MeshObject* intersectionMeshObject = intersection.intersectedPrimitive->GetParentMeshObject();
    const Material* intersectionMaterial = intersectionMeshObject->GetMaterial();

    // find photons that are near the intersection (within constant radius)
    std::vector<Photon> foundPhotons;
    float r = 0.003;
    diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));

    // calculate the contribution of each near photon to the pixel. Compute the BRDF coming from that photon
    if (!foundPhotons.empty()) {
        finalRenderColor = glm::vec3(0.f, 1.f, 0.f);
    }
#else
    Photon intersectionVirtualPhoton;
    intersectionVirtualPhoton.position = intersection.intersectionRay.GetRayPosition(intersection.intersectionT);

    const MeshObject* intersectionMeshObject = intersection.intersectedPrimitive->GetParentMeshObject();
    const Material* intersectionMaterial = intersectionMeshObject->GetMaterial();

    // find photons that are near the intersection (within constant radius)
    std::vector<Photon> foundPhotons;
    float r = 0.005; // minimum r.
    diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));


    // try a different search approach (more similar to KNN)
    int k = 500;
    float growStep = 0.0f;
    int p = 0;
    while(int(foundPhotons.size()) < k){
        // grow radius more if few photons are found. just a runtime optimization.
        p = (k + int(foundPhotons.size())) / 2;
        if (int(foundPhotons.size()) == 0){
            growStep = 0.005;
        }
        else{
            growStep = std::sqrt(float(p) / float(foundPhotons.size())) * r - r;
        }
        growStep = std::sqrt(float(p) / float(foundPhotons.size())) * r - r;
        r = r + std::min(std::max(growStep, 0.005f), 0.05f); // grow by grow step, but at least 0.005, never more than 0.05
        foundPhotons.clear();
        diffuseMap.find_within_range(intersectionVirtualPhoton, r, std::back_inserter(foundPhotons));
    }

    // calculate the contribution of each near photon to the pixel. Compute the BRDF coming from that photon
    if (!foundPhotons.empty()) {
        for (uint p = 0; p < std::min(int(foundPhotons.size()), k); p++) {
                const glm::vec3 brdfColor = intersectionMaterial->ComputeBRDF(intersection,                    // intersection point
                                                                                 foundPhotons[p].intensity,    // intensity (always the same...)
                                                                                 foundPhotons[p].toLightRay,   // make a light ray from photon
                                                                                 fromCameraRay,                // ray from camera to intersection point
                                                                                 1.f);                         // light attenuation (1 = no attenuation)
                // calculate weights based on distance
                float dist = glm::length(foundPhotons[p].position-intersectionVirtualPhoton.position);
                float weight = std::max((r - dist) / r, 0.f);
                float photonDensityAdjust = std::max(float(foundPhotons.size()) / float(k), 1.f);

                finalRenderColor += brdfColor / (r*r) * 30.0f * weight * photonDensityAdjust;
            }
    }
#endif

    return finalRenderColor;
}

void PhotonMappingRenderer::SetNumberOfDiffusePhotons(int diffuse)
{
    diffusePhotonNumber = diffuse;
}

void PhotonMappingRenderer::setPerspectiveCamera(std::shared_ptr<PerspectiveCamera> cam){
    this->pCamera = cam;
}
