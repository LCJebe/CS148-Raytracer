#include "assignment7/Assignment7.h"
#include "common/core.h"

//std::shared_ptr<Camera> Assignment7::CreateCamera() const
//{
//    const glm::vec2 resolution = GetImageOutputResolution();
//    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
//    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
//    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
//    return camera;
//}

//camera->SetPosition(glm::vec3(0.f, 0.f, 2.f));
std::shared_ptr<Camera> Assignment7::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 60.0f);

    glm::vec3 cam_pos(3.f, -21.f, 13.f);
    camera->SetPosition(cam_pos);

    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), 55.f / 180.f * PI);
    camera->Rotate(glm::vec3(0.f, 0.f, 1.f), 13.f / 180.f * PI);

    return camera;
}


// Assignment 7 Part 1 TODO: Change the '1' here.
// 0 -- Naive.
// 1 -- BVH.
// 2 -- Grid.
#define ACCELERATION_TYPE 2

std::shared_ptr<Scene> Assignment7::CreateScene() const
{
    // create scene
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    // add a point light
    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(10.f, 10.f, 10.f));
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f)*2.f);
    scene->AddLight(pointLight);

    // add a point light
    std::shared_ptr<Light> pointLight2 = std::make_shared<PointLight>();
    pointLight2->SetPosition(glm::vec3(0.f, -5.f, 5.f));
    pointLight2->SetLightColor(glm::vec3(1.f, 1.f, 1.f)*2.f);
    scene->AddLight(pointLight2);

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(0.f, 0.f, 0.f));
    //cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    //cubeMaterial->SetReflectivity(0.3f);

    // Objects (loaded from a sigle .obj file)
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> allObjects = MeshLoader::LoadMesh("Scene_Test/Scene_Test.obj", &loadedMaterials);
    std::cout << " There are " << allObjects.size() << " objects in the scene." << std::endl;
    for (size_t i = 0; i < allObjects.size(); ++i) {
        std::shared_ptr<Material> materialCopy = std::make_shared<BlinnPhongMaterial>();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
        allObjects[i]->SetMaterial(materialCopy);

        std::shared_ptr<SceneObject> sceneObject = std::make_shared<SceneObject>();
        sceneObject->AddMeshObject(allObjects[i]);

        // create acceleration structure for the scene object
        sceneObject->CreateAccelerationData(AccelerationTypes::BVH);
        sceneObject->ConfigureAccelerationStructure([](AccelerationStructure* genericAccelerator) {
            BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
            accelerator->SetMaximumChildren(2);
            accelerator->SetNodesOnLeaves(2);
        });

        sceneObject->ConfigureChildMeshAccelerationStructure(
            [](AccelerationStructure* genericAccelerator) {
                BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
                accelerator->SetMaximumChildren(2);
                accelerator->SetNodesOnLeaves(2);
            }
        );

        // add object to scene
        scene->AddSceneObject(sceneObject);

    } // end for over objects in scene


    // create acceleration structure for the scene
    scene->GenerateAccelerationData(AccelerationTypes::BVH);

    return scene;

}


//std::shared_ptr<Scene> Assignment7::CreateScene() const
//{
//    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

//    // Material
//    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
//    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
//    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
//    cubeMaterial->SetReflectivity(0.3f);

//    // Objects
//    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
//    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment7-Alt.obj", &loadedMaterials);
//    for (size_t i = 0; i < cubeObjects.size(); ++i) {
//        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
//        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
//        cubeObjects[i]->SetMaterial(materialCopy);

//        std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
//        cubeSceneObject->AddMeshObject(cubeObjects[i]);
//        cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);

//        cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
//        cubeSceneObject->ConfigureAccelerationStructure([](AccelerationStructure* genericAccelerator) {
//            BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
//            accelerator->SetMaximumChildren(2);
//            accelerator->SetNodesOnLeaves(2);
//        });

//        cubeSceneObject->ConfigureChildMeshAccelerationStructure([](AccelerationStructure* genericAccelerator) {
//            BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
//            accelerator->SetMaximumChildren(2);
//            accelerator->SetNodesOnLeaves(2);
//        });
//        newScene->AddSceneObject(cubeSceneObject);
//    }

//    // Lights
//    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
//    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
//    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));

//#if ACCELERATION_TYPE == 0
//    newScene->GenerateAccelerationData(AccelerationTypes::NONE);
//#elif ACCELERATION_TYPE == 1
//    newScene->GenerateAccelerationData(AccelerationTypes::BVH);
//#else
//    UniformGridAcceleration* accelerator = dynamic_cast<UniformGridAcceleration*>(newScene->GenerateAccelerationData(AccelerationTypes::UNIFORM_GRID));
//    assert(accelerator);
//    // Assignment 7 Part 2 TODO: Change the glm::ivec3(10, 10, 10) here.
//    accelerator->SetSuggestedGridSize(glm::ivec3(3, 3, 3));
//#endif
//    newScene->AddLight(pointLight);

//    return newScene;

//}
std::shared_ptr<ColorSampler> Assignment7::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(glm::ivec3(1, 1, 1));
    return jitter;
}

std::shared_ptr<class Renderer> Assignment7::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    return std::make_shared<BackwardRenderer>(scene, sampler);
}

int Assignment7::GetSamplesPerPixel() const
{
    return 1;
}

bool Assignment7::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int Assignment7::GetMaxReflectionBounces() const
{
    return 2;
}

int Assignment7::GetMaxRefractionBounces() const
{
    return 4;
}

glm::vec2 Assignment7::GetImageOutputResolution() const
{
    return glm::vec2(640.f, 480.f);
}
