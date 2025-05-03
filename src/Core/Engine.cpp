#include "pch.h"
#include "Engine.h"

#include "Scene/Camera/Camera.h"
#include "Scene/Camera/CameraComponent.h"
#include "Scene/Camera/PerspectiveCamera.h"
#include "Scene/Mesh/MeshLoader.h"
#include "Scene/Mesh/MeshComponent.h"
#include "Scene/BehaviourComponent.h"

#include <numeric>
#include <Scene/Camera/FirstPersonControllerComponent.h>
#include <Scene/SpinComponent.h>
#include <Scene/Light/PointLightComponent.h>
#include <Scene/LissajousLightsComponent.h>
#include <Scene/Texture/TextureFactory.h>

void Engine::init(const std::vector<std::string>& commandLineArguments) {
    std::string arguments = std::accumulate(commandLineArguments.begin(), commandLineArguments.end(), std::string{},
            [](const std::string& a, const std::string& b) -> std::string {
                return a.empty() ? b : a + " " + b;
            });
    LOG(INFO) << "Initializing engine. Command line arguments: " << arguments;

    AssetFactory::init();

    mLaunchArguments = commandLineArguments;

    mWindow = std::make_unique<Window>("Vulkan PBR");
    mWindow->setResizeHandler([&](uint32_t width, uint32_t height) { onWindowResize(width, height); });

    mInputSystem = std::make_unique<InputSystem>(*mWindow);
    mVulkanContext = std::make_unique<VulkanContext>(*mWindow);
    mSwapChain = std::make_unique<SwapChain>(*mVulkanContext);
    mScene = std::make_unique<Scene>();
    mFrameRenderer = std::make_unique<FrameRenderer>(*mVulkanContext, *mSwapChain, *mScene);

    initScene1();
}

void Engine::runMainLoop() {
    while (!mInputSystem->isQuitRequested()) {
        mInputSystem->pollEvents();
        if (mWindow->getWidth() == 0 || mWindow->getHeight() == 0 || mSwapChain->isOutOfDate()) {
            mInputSystem->waitForEvents();
        } else {
            prepareFrame();
            updateBehaviour();
            updateTransforms();
            renderFrame();

            if (mInputSystem->getKeyDown(Key::eF11)) {
                static bool fullscreen = false;
                fullscreen = !fullscreen;
                mWindow->setFullscreen(fullscreen);
            }
        }
    }

    mVulkanContext->getDevice().waitIdle();
}

void Engine::initScene1() {
    std::unique_ptr<Camera> mainCamera =
            std::make_unique<PerspectiveCamera>(mWindow->getWidth(), mWindow->getHeight(),
                    glm::radians(70.0f), 0.1f, 1000.0f);
    std::unique_ptr<SceneObject> mainCameraObject = std::make_unique<SceneObject>();
    std::unique_ptr<CameraComponent> cameraComponent = std::make_unique<CameraComponent>(std::move(mainCamera));
    mScene->setCamera(&*cameraComponent);
    mainCameraObject->addComponent(std::move(cameraComponent));
    mainCameraObject->addComponent(std::make_unique<FirstPersonControllerComponent>());
    mainCameraObject->getTransform().translate(glm::vec3(0.0f, 0.0f, -2.0f));
    mScene->getRoot().addChild(std::move(mainCameraObject));

    std::shared_ptr<Mesh> sphereMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("sphere3.obj"));
    std::shared_ptr<Mesh> cubeMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("cube2.obj"));

    std::string K = "2K";

    TextureFactory textureFactory(*mVulkanContext);
    std::shared_ptr<Texture2D> testAlbedo = textureFactory.load2D(
            AssetFactory::getTextureAsset("T6/ufomabmo_" + K + "_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO = textureFactory.load2D(
            AssetFactory::getTextureAsset("T6/ufomabmo_" + K + "_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness = textureFactory.load2D(
            AssetFactory::getTextureAsset("T6/ufomabmo_" + K + "_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal = textureFactory.load2D(
            AssetFactory::getTextureAsset("T6/ufomabmo_" + K + "_Normal.jpg"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_basecolor.png"), false, 3);
    std::shared_ptr<Texture2D> testMetallic2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_metallic.png"), true, 1);
    std::shared_ptr<Texture2D> testRoughness2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_roughness.png"), true, 1);
    std::shared_ptr<Texture2D> testNormal2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_normal.png"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_" + K + "_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_" + K + "_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_" + K + "_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_" + K + "_Normal.jpg"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo4 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T4/uihobckv_" + K + "_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO4 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T4/uihobckv_" + K + "_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness4 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T4/uihobckv_" + K + "_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal4 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T4/uihobckv_" + K + "_Normal.jpg"), true, 3);

    std::string wK = "2K";

    std::shared_ptr<Texture2D> testAlbedo5 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T9/uehffbmew_" + wK + "_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO5 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T9/uehffbmew_" + wK + "_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness5 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T9/uehffbmew_" + wK + "_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal5 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T9/uehffbmew_" + wK + "_Normal.jpg"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo6 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T7/vhjldck_" + wK + "_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO6 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T7/vhjldck_" + wK + "_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness6 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T7/vhjldck_" + wK + "_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal6 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T7/vhjldck_" + wK + "_Normal.jpg"), true, 3);

//    std::shared_ptr<Texture2D> testAlbedo3 = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Albedo.jpg"), false, 3);
//    std::shared_ptr<Texture2D> testAO3 = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_AO.jpg"), true, 1);
//    std::shared_ptr<Texture2D> testRoughness3 = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Roughness.jpg"), true, 1);
//    std::shared_ptr<Texture2D> testNormal3 = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Normal.jpg"), true, 3);

    std::shared_ptr<Material> testMaterial = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    testMaterial->albedoTexture = testAlbedo;
    testMaterial->aoTexture = testAO;
    testMaterial->roughnessTexture = testRoughness;
    testMaterial->normalTexture = testNormal;

    std::shared_ptr<Material> testMaterial2 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    testMaterial2->albedoTexture = testAlbedo2;
    testMaterial2->metallicTexture = testMetallic2;
    testMaterial2->roughnessTexture = testRoughness2;
    testMaterial2->normalTexture = testNormal2;

    std::shared_ptr<Material> testMaterial3 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            0.0f, 1.0f);
    testMaterial3->albedoTexture = testAlbedo3;
    testMaterial3->aoTexture = testAO3;
    testMaterial3->roughnessTexture = testRoughness3;
    testMaterial3->normalTexture = testNormal3;

    std::shared_ptr<Material> testMaterial4 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            0.0f, 1.0f);
    testMaterial4->albedoTexture = testAlbedo4;
    testMaterial4->aoTexture = testAO4;
    testMaterial4->roughnessTexture = testRoughness4;
    testMaterial4->normalTexture = testNormal4;

    std::shared_ptr<Material> testMaterial5 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            0.0f, 1.0f);
    testMaterial5->albedoTexture = testAlbedo5;
    testMaterial5->aoTexture = testAO5;
    testMaterial5->roughnessTexture = testRoughness5;
    testMaterial5->normalTexture = testNormal5;

    std::shared_ptr<Material> testMaterial6 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            0.0f, 1.0f);
    testMaterial6->albedoTexture = testAlbedo6;
    testMaterial6->aoTexture = testAO6;
    testMaterial6->roughnessTexture = testRoughness6;
    testMaterial6->normalTexture = testNormal6;

    std::unique_ptr<SceneObject> meshObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent->getMaterialSlots()[0] = testMaterial4;
    meshObject->addComponent(std::move(meshComponent));
    meshObject->getTransform().translate(glm::vec3(2.0f, 0.0f, 2.0f));
    meshObject->addComponent(std::make_unique<SpinComponent>());
    mScene->getRoot().addChild(std::move(meshObject));

    std::unique_ptr<SceneObject> meshObject2 = std::make_unique<SceneObject>();
    meshObject2->getTransform().translate(glm::vec3(2.0f, 0.0f, -2.0f));
    std::unique_ptr<MeshComponent> meshComponent2 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent2->getMaterialSlots()[0] = testMaterial3;
    meshObject2->addComponent(std::move(meshComponent2));
    mScene->getRoot().addChild(std::move(meshObject2));

    std::unique_ptr<SceneObject> meshObject3 = std::make_unique<SceneObject>();
    meshObject3->getTransform().translate(glm::vec3(-2.0f, 0.0f, -2.0f));
    std::unique_ptr<MeshComponent> meshComponent3 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent3->getMaterialSlots()[0] = testMaterial2;
    meshObject3->addComponent(std::move(meshComponent3));
    mScene->getRoot().addChild(std::move(meshObject3));

    std::unique_ptr<SceneObject> meshObject4 = std::make_unique<SceneObject>();
    meshObject4->getTransform().translate(glm::vec3(-2.0f, 0.0f, 2.0f));
    std::unique_ptr<MeshComponent> meshComponent4 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent4->getMaterialSlots()[0] = testMaterial;
    meshObject4->addComponent(std::move(meshComponent4));
    mScene->getRoot().addChild(std::move(meshObject4));

    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(0.0f, 2.0f, 0.0f));
        pointLightObject->getTransform().setScale(glm::vec3(0.1f, 0.1f, 0.1f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(35.0f * glm::vec3(3.0f, 3.0f, 6.0f)));
        std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(sphereMesh);
        meshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                80.0f * glm::vec3(3.0f, 3.0f, 6.0f), 0.0f, 0.0f);
        pointLightObject->addComponent(std::move(meshComponent));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }

    std::unique_ptr<SceneObject> lissajous = std::make_unique<SceneObject>();
    lissajous->getTransform().translate(glm::vec3(0.0f, 0.0f, 0.0f));
    lissajous->addComponent(std::make_unique<LissajousLightsComponent>(50));
    mScene->getRoot().addChild(std::move(lissajous));

    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(20.0f, 0.5f, 20.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial6;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, -10.0f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(20.0f, 0.5f, 20.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial5;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(10.0f, 0.0f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(0.5f, 20.0f, 20.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial6;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(-10.0f, 0.0f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(0.5f, 20.0f, 20.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial6;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
        cubeObject->getTransform().setScale(glm::vec3(20.0f, 20.0f, 0.5f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial6;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, 0.0f, -10.0f));
        cubeObject->getTransform().setScale(glm::vec3(20.0f, 20.0f, 0.5f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = testMaterial6;
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    //Light cubes
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, -8.75f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(2.0f, 2.0f, 2.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                20.0f * glm::vec3(0.05f, 0.05f, 1.0f), 0.0f, 0.0f);
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(-5.0f, -8.75f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(2.0f, 2.0f, 2.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                20.0f * glm::vec3(1.00f, 0.05f, 0.05f), 0.0f, 0.0f);
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, -8.75f, 5.0f));
        cubeObject->getTransform().setScale(glm::vec3(2.0f, 2.0f, 2.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                20.0f * glm::vec3(0.05f, 1.0f, 0.5f), 0.0f, 0.0f);
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(5.0f, -8.75f, 0.0f));
        cubeObject->getTransform().setScale(glm::vec3(2.0f, 2.0f, 2.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                20.0f * glm::vec3(1.0f, 1.0f, 0.05f), 0.0f, 0.0f);
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
    {
        std::unique_ptr<SceneObject> cubeObject = std::make_unique<SceneObject>();
        cubeObject->getTransform().setPosition(glm::vec3(0.0f, -8.75f, -5.0f));
        cubeObject->getTransform().setScale(glm::vec3(2.0f, 2.0f, 2.0f));
        std::unique_ptr<MeshComponent> cubeMeshComponent = std::make_unique<MeshComponent>(cubeMesh);
        cubeMeshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                20.0f * glm::vec3(1.05f, 0.05f, 1.0f), 0.0f, 0.0f);
        cubeObject->addComponent(std::move(cubeMeshComponent));
        mScene->getRoot().addChild(std::move(cubeObject));
    }
}

void Engine::initScene3() {
    std::unique_ptr<Camera> mainCamera =
            std::make_unique<PerspectiveCamera>(mWindow->getWidth(), mWindow->getHeight(),
                    glm::radians(70.0f), 0.1f, 1000.0f);
    std::unique_ptr<SceneObject> mainCameraObject = std::make_unique<SceneObject>();
    std::unique_ptr<CameraComponent> cameraComponent = std::make_unique<CameraComponent>(std::move(mainCamera));
    mScene->setCamera(&*cameraComponent);
    mainCameraObject->addComponent(std::move(cameraComponent));
    mainCameraObject->addComponent(std::make_unique<FirstPersonControllerComponent>());
    mainCameraObject->getTransform().translate(glm::vec3(0.0f, 0.0f, -2.0f));
    mScene->getRoot().addChild(std::move(mainCameraObject));

    std::shared_ptr<Mesh> sphereMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("sphere3.obj"));
    std::shared_ptr<Mesh> cubeMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("cube.obj"));

    TextureFactory textureFactory(*mVulkanContext);
    std::shared_ptr<Texture2D> testAlbedo = textureFactory.load2D(
            AssetFactory::getTextureAsset("T1/scksebop_4K_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testMetallic = textureFactory.load2D(
            AssetFactory::getTextureAsset("T1/scksebop_4K_Metalness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness = textureFactory.load2D(
            AssetFactory::getTextureAsset("T1/scksebop_4K_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal = textureFactory.load2D(
            AssetFactory::getTextureAsset("T1/scksebop_4K_Normal.jpg"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_basecolor.png"), false, 3);
    std::shared_ptr<Texture2D> testMetallic2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_metallic.png"), true, 1);
    std::shared_ptr<Texture2D> testRoughness2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_roughness.png"), true, 1);
    std::shared_ptr<Texture2D> testNormal2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("rustediron2_normal.png"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_8K_Albedo.jpg"), false, 3);
    std::shared_ptr<Texture2D> testAO3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_8K_AO.jpg"), true, 1);
    std::shared_ptr<Texture2D> testRoughness3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_8K_Roughness.jpg"), true, 1);
    std::shared_ptr<Texture2D> testNormal3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("T3/tlooadar_8K_Normal.jpg"), true, 3);

//    std::shared_ptr<Texture2D> testAlbedo = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Albedo.jpg"), false, 3);
//    std::shared_ptr<Texture2D> testAO = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_AO.jpg"), true, 1);
//    std::shared_ptr<Texture2D> testRoughness = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Roughness.jpg"), true, 1);
//    std::shared_ptr<Texture2D> testNormal = textureFactory.load2D(
//            AssetFactory::getTextureAsset("T2/smvnfc2p_8K_Normal.jpg"), true, 3);

    std::shared_ptr<Material> testMaterial = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    testMaterial->albedoTexture = testAlbedo;
    testMaterial->metallicTexture = testMetallic;
    testMaterial->roughnessTexture = testRoughness;
    testMaterial->normalTexture = testNormal;

    std::shared_ptr<Material> testMaterial2 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    testMaterial2->albedoTexture = testAlbedo2;
    testMaterial2->metallicTexture = testMetallic2;
    testMaterial2->roughnessTexture = testRoughness2;
    testMaterial2->normalTexture = testNormal2;

    std::shared_ptr<Material> testMaterial3 = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            0.0f, 1.0f);
    testMaterial3->albedoTexture = testAlbedo3;
    testMaterial3->aoTexture = testAO3;
    testMaterial3->roughnessTexture = testRoughness3;
    testMaterial3->normalTexture = testNormal3;

    std::unique_ptr<SceneObject> meshObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f), 0.0f, 0.1f);
    meshObject->addComponent(std::move(meshComponent));
    meshObject->getTransform().translate(glm::vec3(2.0f, 0.0f, 2.0f));
    meshObject->addComponent(std::make_unique<SpinComponent>());
    mScene->getRoot().addChild(std::move(meshObject));

    std::unique_ptr<SceneObject> meshObject2 = std::make_unique<SceneObject>();
    meshObject2->getTransform().translate(glm::vec3(2.0f, 0.0f, -2.0f));
    std::unique_ptr<MeshComponent> meshComponent2 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent2->getMaterialSlots()[0] = testMaterial3;
    meshObject2->addComponent(std::move(meshComponent2));
    mScene->getRoot().addChild(std::move(meshObject2));

    std::unique_ptr<SceneObject> meshObject3 = std::make_unique<SceneObject>();
    meshObject3->getTransform().translate(glm::vec3(-2.0f, 0.0f, -2.0f));
    std::unique_ptr<MeshComponent> meshComponent3 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent3->getMaterialSlots()[0] = testMaterial2;
    meshObject3->addComponent(std::move(meshComponent3));
    mScene->getRoot().addChild(std::move(meshObject3));

    std::unique_ptr<SceneObject> meshObject4 = std::make_unique<SceneObject>();
    meshObject4->getTransform().translate(glm::vec3(-2.0f, 0.0f, 2.0f));
    std::unique_ptr<MeshComponent> meshComponent4 = std::make_unique<MeshComponent>(sphereMesh);
    meshComponent4->getMaterialSlots()[0] = testMaterial;
    meshObject4->addComponent(std::move(meshComponent4));
    mScene->getRoot().addChild(std::move(meshObject4));

    std::unique_ptr<SceneObject> floorObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> floorComponent = std::make_unique<MeshComponent>(cubeMesh);
    floorComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f), 0.0f, 0.15f);
    floorObject->addComponent(std::move(floorComponent));
    floorObject->getTransform().setScale(glm::vec3(80.0f, 0.1f, 80.0f));
    floorObject->getTransform().setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    mScene->getRoot().addChild(std::move(floorObject));

    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(0.0f, 1.0f, 0.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(glm::vec3(5.0f, 5.0f, 6.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(3.0f, 2.0f, 0.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(glm::vec3(2.0f, 2.0f, 6.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(0.0f, 0.5f, 3.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(glm::vec3(7.0f, 7.0f, 2.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
}

void Engine::initScene2() {
    std::unique_ptr<Camera> mainCamera =
            std::make_unique<PerspectiveCamera>(mWindow->getWidth(), mWindow->getHeight(),
                    glm::radians(70.0f), 0.1f, 1000.0f);
    std::unique_ptr<SceneObject> mainCameraObject = std::make_unique<SceneObject>();
    std::unique_ptr<CameraComponent> cameraComponent = std::make_unique<CameraComponent>(std::move(mainCamera));
    mScene->setCamera(&*cameraComponent);
    mainCameraObject->addComponent(std::move(cameraComponent));
    mainCameraObject->addComponent(std::make_unique<FirstPersonControllerComponent>());
    mainCameraObject->getTransform().translate(glm::vec3(0.0f, 1.0f, 0.0f));
    mScene->getRoot().addChild(std::move(mainCameraObject));

    std::shared_ptr<Mesh> mesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("cube.obj"));

    std::unique_ptr<SceneObject> meshObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(mesh);
    meshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f), 0.0f, 0.05f);
    meshObject->addComponent(std::move(meshComponent));
    meshObject->getTransform().setScale(glm::vec3(80.0f, 0.1f, 80.0f));
    mScene->getRoot().addChild(std::move(meshObject));

    std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
    pointLightObject->getTransform().translate(glm::vec3(0.0f, 1.0f, 0.0f));
    pointLightObject->addComponent(std::make_unique<PointLightComponent>(glm::vec3(5.0f, 5.0f, 6.0f)));
    mScene->getRoot().addChild(std::move(pointLightObject));
}

void Engine::initScene4() {
    std::unique_ptr<Camera> mainCamera =
            std::make_unique<PerspectiveCamera>(mWindow->getWidth(), mWindow->getHeight(),
                    glm::radians(70.0f), 0.1f, 1000.0f);
    std::unique_ptr<SceneObject> mainCameraObject = std::make_unique<SceneObject>();
    std::unique_ptr<CameraComponent> cameraComponent = std::make_unique<CameraComponent>(std::move(mainCamera));
    mScene->setCamera(&*cameraComponent);
    mainCameraObject->addComponent(std::move(cameraComponent));
    mainCameraObject->addComponent(std::make_unique<FirstPersonControllerComponent>());
    mainCameraObject->getTransform().translate(glm::vec3(0.0f, 0.0f, -2.0f));
    mScene->getRoot().addChild(std::move(mainCameraObject));

    std::shared_ptr<Mesh> robotMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("CR.obj"));
    std::shared_ptr<Mesh> cubeMesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("cube.obj"));

    TextureFactory textureFactory(*mVulkanContext);
    std::shared_ptr<Texture2D> testAlbedo = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Hands_BaseColor.png"), false, 3);
    std::shared_ptr<Texture2D> testMetallic = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Hands_Metallic.png"), true, 1);
    std::shared_ptr<Texture2D> testRoughness = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Hands_Roughness.png"), true, 1);
    std::shared_ptr<Texture2D> testNormal = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Hands_Normal.png"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Legs_BaseColor.png"), false, 3);
    std::shared_ptr<Texture2D> testMetallic2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Legs_Metallic.png"), true, 1);
    std::shared_ptr<Texture2D> testRoughness2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Legs_Roughness.png"), true, 1);
    std::shared_ptr<Texture2D> testNormal2 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Legs_Normal.png"), true, 3);

    std::shared_ptr<Texture2D> testAlbedo3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Torse_BaseColor.png"), false, 3);
    std::shared_ptr<Texture2D> testMetallic3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Torse_Metallic.png"), true, 1);
    std::shared_ptr<Texture2D> testRoughness3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Torse_Roughness.png"), true, 1);
    std::shared_ptr<Texture2D> testNormal3 = textureFactory.load2D(
            AssetFactory::getTextureAsset("CR/2022_10_31_Robot_LP4_Torse_Normal.png"), true, 3);

    std::shared_ptr<Material> handsMat = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    handsMat->albedoTexture = testAlbedo;
    handsMat->metallicTexture = testMetallic;
    handsMat->roughnessTexture = testRoughness;
    handsMat->normalTexture = testNormal;

    std::shared_ptr<Material> legsMat = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    legsMat->albedoTexture = testAlbedo2;
    legsMat->metallicTexture = testMetallic2;
    legsMat->roughnessTexture = testRoughness2;
    legsMat->normalTexture = testNormal2;

    std::shared_ptr<Material> torseMat = std::make_shared<Material>(glm::vec3(1.0f),
            glm::vec3(0.0f),
            1.0f, 1.0f);
    torseMat->albedoTexture = testAlbedo3;
    torseMat->metallicTexture = testMetallic3;
    torseMat->roughnessTexture = testRoughness3;
    torseMat->normalTexture = testNormal3;

    std::unique_ptr<SceneObject> meshObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(robotMesh);
    meshComponent->getMaterialSlots().push_back(handsMat);
    meshComponent->getMaterialSlots().push_back(torseMat);
    meshComponent->getMaterialSlots().push_back(legsMat);
//    meshComponent->getMaterialSlots().push_back(Material::kDefaultMaterial);
    meshObject->addComponent(std::move(meshComponent));
    meshObject->getTransform().translate(glm::vec3(2.0f, 0.0f, 2.0f));
//    meshObject->addComponent(std::make_unique<SpinComponent>());
    mScene->getRoot().addChild(std::move(meshObject));

    std::unique_ptr<SceneObject> floorObject = std::make_unique<SceneObject>();
    std::unique_ptr<MeshComponent> floorComponent = std::make_unique<MeshComponent>(cubeMesh);
    floorComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(0.0f), 0.0f, 0.15f);
    floorObject->addComponent(std::move(floorComponent));
    floorObject->getTransform().setScale(glm::vec3(80.0f, 0.1f, 80.0f));
    floorObject->getTransform().setPosition(glm::vec3(0.0f, -0.05f, 0.0f));
    mScene->getRoot().addChild(std::move(floorObject));

    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(0.0f, 12.0f, -6.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(50.0f * glm::vec3(5.0f, 5.0f, 6.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(9.0f, 6.0f, 0.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(50.0f * glm::vec3(2.0f, 2.0f, 6.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
    {
        std::unique_ptr<SceneObject> pointLightObject = std::make_unique<SceneObject>();
        pointLightObject->getTransform().translate(glm::vec3(0.0f, 1.5f, 9.0f));
        pointLightObject->addComponent(std::make_unique<PointLightComponent>(50.0f * glm::vec3(7.0f, 7.0f, 2.0f)));
        mScene->getRoot().addChild(std::move(pointLightObject));
    }
}

void Engine::onWindowResize(uint32_t width, uint32_t height) {
    mVulkanContext->getDevice().waitIdle();

    mFrameRenderer = nullptr;
    mSwapChain = nullptr;

    if (mWindow->getWidth() != 0 && mWindow->getHeight() != 0) {
        mSwapChain = std::make_unique<SwapChain>(*mVulkanContext);
        mFrameRenderer = std::make_unique<FrameRenderer>(*mVulkanContext, *mSwapChain, *mScene);

        mScene->getCamera()->getCamera().setWidth(width);
        mScene->getCamera()->getCamera().setHeight(height);
    }
}

void Engine::prepareFrame() {
    auto newFrameStart = std::chrono::steady_clock::now();

    mDelta = std::chrono::duration<double>(newFrameStart - mFrameStart).count();
    if (mDelta > 1.0) {
        mDelta = 1.0;
    }

    mFrameStart = newFrameStart;
}

void Engine::updateTransforms() {
    mScene->getRoot().updateTransforms();
}

void Engine::updateBehaviour() {
    std::vector<std::reference_wrapper<BehaviourComponent>> components
            = mScene->getRoot().getComponentsRecursive<BehaviourComponent>();

    float delta = static_cast<float>(mDelta);

    for (BehaviourComponent& component : components) {
        component.update(delta);
    }
}

void Engine::renderFrame() {
    mFrameRenderer->renderFrame();
}
