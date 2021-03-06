#include "common/RayTracer.h"
#include "common/Application.h"
#include "common/Scene/Scene.h"
#include "common/Scene/Camera/Camera.h"
#include "common/Scene/Geometry/Ray/Ray.h"
#include "common/Intersection/IntersectionState.h"
#include "common/Sampling/ColorSampler.h"
#include "common/Output/ImageWriter.h"
#include "common/Rendering/Renderer.h"
#include "thread"

#include "common/Scene/Geometry/Primitives/Triangle/Triangle.h"

#define BOX 1


RayTracer::RayTracer(std::unique_ptr<class Application> app):
    storedApplication(std::move(app))
{
}

void RayTracer::Run()
{
    // Scene Setup -- Generate the camera and scene.
    std::shared_ptr<Camera> currentCamera = storedApplication->CreateCamera();
    std::shared_ptr<Scene> currentScene = storedApplication->CreateScene();
    std::shared_ptr<ColorSampler> currentSampler = storedApplication->CreateSampler();
    std::shared_ptr<Renderer> currentRenderer = storedApplication->CreateRenderer(currentScene, currentSampler);
    assert(currentScene && currentCamera && currentSampler && currentRenderer);

    currentSampler->InitializeSampler(storedApplication.get(), currentScene.get());

    // Scene preprocessing -- generate acceleration structures, etc.
    // After this call, we are guaranteed that the "acceleration" member of the scene and all scene objects within the scene will be non-NULL.
    currentScene->GenerateDefaultAccelerationData();
    currentScene->Finalize();

    currentRenderer->InitializeRenderer();

    // Prepare for Output
    const glm::vec2 currentResolution = storedApplication->GetImageOutputResolution();
    ImageWriter imageWriter(storedApplication->GetOutputFilename(), static_cast<int>(currentResolution.x), static_cast<int>(currentResolution.y));

    // Perform forward ray tracing
    const int maxSamplesPerPixel = storedApplication->GetSamplesPerPixel();
    assert(maxSamplesPerPixel >= 1);

    const unsigned numCPU = std::thread::hardware_concurrency();
    std::cout << "Number of CPUs: " << numCPU << std::endl;

    // define box which we render (for testing purposes)
    int row_start = 0;
    int row_end = static_cast<int>(currentResolution.y);
    int col_start = 0;
    int col_end = static_cast<int>(currentResolution.x);

    if (BOX) {
        row_start = static_cast<int>(currentResolution.y) * 340 / 540 - 50;
        row_end = static_cast<int>(currentResolution.y) * 340 / 540 + 50;
        col_start = static_cast<int>(currentResolution.x) * 160 / 960 - 50;
        col_end = static_cast<int>(currentResolution.x) * 160 / 960 + 50;
    }

    for (int r = row_start; r < row_end; ++r) {
        #pragma omp parallel for
        for (int c = col_start; c < col_end; ++c) {
            imageWriter.SetPixelColor(currentSampler->ComputeSamplesAndColor(maxSamplesPerPixel, 2, [&](glm::vec3 inputSample, int sampleIdx) {
                const glm::vec3 minRange(-0.5f, -0.5f, 0.f);
                const glm::vec3 maxRange(0.5f, 0.5f, 0.f);
                const glm::vec3 sampleOffset = (maxSamplesPerPixel == 1) ? glm::vec3(0.f, 0.f, 0.f) : minRange + (maxRange - minRange) * inputSample;

                glm::vec2 normalizedCoordinates(static_cast<float>(c) + sampleOffset.x, static_cast<float>(r) + sampleOffset.y);
                normalizedCoordinates /= currentResolution;

                // Construct ray, send it out into the scene and see what we hit.
                std::shared_ptr<Ray> cameraRay = currentCamera->GenerateRayForNormalizedCoordinates(normalizedCoordinates);
                assert(cameraRay);

                IntersectionState rayIntersection(storedApplication->GetMaxReflectionBounces(), storedApplication->GetMaxRefractionBounces());
                bool didHitScene = currentScene->Trace(cameraRay.get(), &rayIntersection);

                // Use the intersection data to compute the BRDF response.
                glm::vec3 sampleColor;
                if (didHitScene) {
                    sampleColor = currentRenderer->ComputeSampleColor(rayIntersection, *cameraRay.get(), sampleIdx);
                }

                // perform gamma - correction
                sampleColor = glm::pow(sampleColor, glm::vec3(1.f, 1.f, 1.f) * 1.0f / 2.2f);

                return sampleColor;
            }), c, r);
//            if ((r < float(currentResolution.y) / numCPU) && ((c % 100) == 0)) {
//                std::cout << (r + c / currentResolution.x) / currentResolution.y * numCPU * 100.0f << " % finished..." << std::endl;
//            }
        }
         std::cout << "Row " << r << " of " << currentResolution.y << " finished..." << std::endl;
    }

    // Apply post-processing steps (i.e. tone-mapper, etc.).
    storedApplication->PerformImagePostprocessing(imageWriter);

    // Now copy whatever is in the HDR data and store it in the bitmap that we will save (aka everything will get clamped to be [0.0, 1.0]).
    imageWriter.CopyHDRToBitmap();
    // Save image.
    imageWriter.SaveImage();
}
