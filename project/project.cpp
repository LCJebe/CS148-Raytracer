#include "project/project.h"
#include "common/core.h"

void addSphereLight(glm::vec3 center, float radius, int numPointLights, glm::vec3 lightColor, std::shared_ptr<Scene> scene){

    for (int i = 0; i < numPointLights; i++){
        // sample random uniformly on unit sphere
        float u = (float)std::rand() / RAND_MAX * 2 -1; //uniform in [0, 1)
        float theta = (float)std::rand() / RAND_MAX *2 * PI; //uniform in [0, 2*PI)


        float x = std::sqrt(1-u*u) * std::cos(theta);
        float y = std::sqrt(1-u*u) * std::sin(theta);
        float z = u;

        glm::vec3 lightPos(x, y, z);

        // transform from unit sphere to our sphere
        lightPos = lightPos * radius;
        lightPos = lightPos + center;

        // create point light
        std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
        pointLight->SetPosition(lightPos);
        pointLight->SetLightColor(lightColor);
        scene->AddLight(pointLight);
    }
}


std::shared_ptr<Camera> project::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera1 = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 49.1f * resolution.y / resolution.x);

    glm::vec3 cam_pos(-5.42333f, 1.62742f, 6.6804f);
    camera1->SetPosition(cam_pos);

    camera1->Rotate(glm::vec3(1.f, 0.f, 0.f), 81.1831f / 180.f * PI);
    camera1->Rotate(glm::vec3(0.f, 0.f, 1.f), -101.51f / 180.f * PI);


    std::shared_ptr<Camera> camera2 = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 49.1f * resolution.y / resolution.x);
    glm::vec3 cam_pos2(-6.38683f, 1.087f, 9.18057f);
    camera2->SetPosition(cam_pos2);

    camera2->Rotate(glm::vec3(1.f, 0.f, 0.f), 63.134f / 180.f * PI);
    camera2->Rotate(glm::vec3(0.f, 0.f, 1.f), -61.8181f / 180.f * PI);

    std::shared_ptr<Camera> camera3 = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 49.1f * resolution.y / resolution.x);
    glm::vec3 cam_pos3(3.78134f, 2.36243f, 7.38813f);
    camera3->SetPosition(cam_pos3);

    camera3->Rotate(glm::vec3(1.f, 0.f, 0.f), 75.5642f / 180.f * PI);
    camera3->Rotate(glm::vec3(0.f, 0.f, 1.f), -123.162f / 180.f * PI);

    std::shared_ptr<Camera> camera4 = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 49.1f * resolution.y / resolution.x);
    glm::vec3 cam_pos4(-6.99852f, 1.94818f, 6.92974f);
    camera4->SetPosition(cam_pos4);

    camera4->Rotate(glm::vec3(1.f, 0.f, 0.f), 81.1832f / 180.f * PI);
    camera4->Rotate(glm::vec3(0.f, 0.f, 1.f), -101.51f / 180.f * PI);

    float focalDistance = 7.f;
    float apertureRadius = 0.06f;
    std::shared_ptr<Camera> DOFcamera = std::make_shared<WideApertureCamera>(resolution.x / resolution.y, 49.1f * resolution.y / resolution.x, focalDistance, apertureRadius);
    DOFcamera->SetPosition(cam_pos);
    DOFcamera->Rotate(glm::vec3(1.f, 0.f, 0.f), 81.1831f / 180.f * PI);
    DOFcamera->Rotate(glm::vec3(0.f, 0.f, 1.f), -101.51f / 180.f * PI);

    return camera1;
}


// Assignment 7 Part 1 TODO: Change the '1' here.
// 0 -- Naive.
// 1 -- BVH.
// 2 -- Grid.
#define ACCELERATION_TYPE 2

std::shared_ptr<Scene> project::CreateScene() const
{
    // create scene
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    ///////////////////////////////////
    //////////// LIGHTS ///////////////
    ///////////////////////////////////

    // add a green / blue point light to balance colors
    glm::vec3 blueColor = glm::vec3(0.037f, 0.915f, 1.0f);
    glm::vec3 orangeColor = glm::vec3(0.855f, 0.167f, 0.047f);

    std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(4.09601f, 5.2227f, 5.24693f));
    pointLight->SetLightColor(orangeColor * 2.f);
    scene->AddLight(pointLight);

    // add the two candle lights using POINT lights
    glm::vec3 candleColor = glm::vec3(1.f, 0.2f, 0.05f)*1.f;

    // try adding the two sphere lights
    glm::vec3 lightPos1 = glm::vec3(4.46158f, 1.94833f, 7.22604f);
    glm::vec3 lightPos2 = glm::vec3(-1.16581f, -0.760411f, 7.22604f);
    float radius = 0.12f;
    int numPointLights = 8;
    addSphereLight(lightPos1, radius, numPointLights, candleColor, scene);
    addSphereLight(lightPos2, radius, numPointLights, candleColor, scene);

//    // add another light for cool reflections
//    std::shared_ptr<Light> refLight = std::make_shared<PointLight>();
//    refLight->SetPosition(glm::vec3(5.46892f, 6.69302f, 6.38128f));
//    refLight->SetLightColor(candleColor);
//    scene->AddLight(refLight);

//    // try it with the real sphere light implementation
//    glm::vec3 lightPos1 = glm::vec3(4.46158f, 1.94833f, 7.22604f);
//    glm::vec3 lightPos2 = glm::vec3(-1.16581f, -0.760411f, 7.22604f);
//    float radius = 0.15f;

//    std::shared_ptr<Light> sphereLight1 = std::make_shared<SphereLight>(radius);
//    std::shared_ptr<Light> sphereLight2 = std::make_shared<SphereLight>(radius);
//    sphereLight1->SetPosition(lightPos1);
//    sphereLight1->SetLightColor(candleColor);
//    scene->AddLight(sphereLight1);
//    sphereLight2->SetPosition(lightPos2);
//    sphereLight2->SetLightColor(candleColor);
//    scene->AddLight(sphereLight2);


    /////////////////////////////////////////////

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(0.f, 0.f, 0.f));
    //cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
    //cubeMaterial->SetReflectivity(0.3f);

    // Objects (loaded from a sigle .obj file)
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> allObjects = MeshLoader::LoadMesh("../../../Project/Scene6.obj", &loadedMaterials);
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


std::shared_ptr<ColorSampler> project::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(glm::ivec3(4, 4, 1)); // 4, 4

    std::shared_ptr<SimpleAdaptiveSampler> sampler = std::make_shared<SimpleAdaptiveSampler>();
    sampler->SetInternalSampler(jitter);
    sampler->SetEarlyExitParameters(100.f * SMALL_EPSILON, 16); // 16

    return sampler;
    //return jitter;
}

std::shared_ptr<class Renderer> project::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
    //std::shared_ptr<BackwardRenderer> renderer = std::make_shared<BackwardRenderer>(scene, sampler);

    std::shared_ptr<Camera> camera = CreateCamera();
    std::shared_ptr<PhotonMappingRenderer> renderer = std::make_shared<PhotonMappingRenderer>(scene, sampler);
    std::shared_ptr<PerspectiveCamera> pcamera = std::dynamic_pointer_cast<PerspectiveCamera> (camera);
    renderer->setPerspectiveCamera(pcamera);

    return renderer;
}

int project::GetSamplesPerPixel() const
{
    return 8;
}

bool project::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}

int project::GetMaxReflectionBounces() const
{
    return 4;
}

int project::GetMaxRefractionBounces() const
{
    return 4;
}

glm::vec2 project::GetImageOutputResolution() const
{
    return glm::vec2(1920.f, 1080.f) / 2.0f;
}
