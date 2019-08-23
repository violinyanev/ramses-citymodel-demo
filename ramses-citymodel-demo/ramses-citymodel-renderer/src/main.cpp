//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-citymodel/CitymodelUtils.h"
#include "ramses-citymodel/SceneToText.h"
#include "CitymodelRendererArguments.h"
#include "DisplayManager.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-utils.h"
#include <sstream>

int main(int argc, char* argv[])
{
    CitymodelRendererArguments arguments;
    if (!arguments.parse(argc, argv))
    {
        return 0;
    }

    ramses::RamsesFramework framework(*CitymodelUtils::CreateFrameworkConfig(arguments));

    ramses::RendererConfig rendererConfig;
    rendererConfig.enableSystemCompositorControl();
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    renderer.setSkippingOfUnmodifiedBuffers(false);

    Citymodel client(arguments, framework);
    framework.connect();

    SceneToText stt(client.getRamsesScene(), client.getRamsesClient(), true);
    std::ostringstream stream;
    stt.printToStream(stream);
    std::cout << stream.str();

    return 0;

    DisplayManager displayManager(renderer, framework, false, &client);

    ramses::DisplayConfig displayConfig;
    const float aspect = static_cast<float>(arguments.m_windowWidth) / static_cast<float>(arguments.m_windowHeight);
    const float nearPlane = 0.1f;
    const float farPlane = 1500.0f;
    displayConfig.setPerspectiveProjection(client.getFovy(), aspect, nearPlane, farPlane);
    displayConfig.setWindowRectangle(0, 0, arguments.m_windowWidth, arguments.m_windowHeight);
    displayConfig.setWaylandIviSurfaceID(arguments.m_waylandIviSurfaceID);
    displayConfig.setWaylandIviLayerID(arguments.m_waylandIviLayerID);
    displayConfig.setWindowIviVisible();

    const ramses::displayId_t displayId = displayManager.createDisplay(displayConfig);

    ramses::ResourceFileDescriptionSet resourceFileInformation;
    //resourceFileInformation.add(ramses::ResourceFileDescription("/home/ramses/Downloads/bmwx5_gltf/Scene.ramres"));
    //ramses::Scene* carScene = client.getRamsesClient().loadSceneFromFile("/home/ramses/Downloads/bmwx5_gltf/Scene.ramses", resourceFileInformation);
    resourceFileInformation.add(ramses::ResourceFileDescription("res/Scene.ramres"));
    ramses::Scene* carScene = client.getRamsesClient().loadSceneFromFile("res/Scene.ramses", resourceFileInformation);

    /*SceneToText stt(*carScene, client.getRamsesClient(), true);
    std::ostringstream stream;
    stt.printToStream(stream);
    std::cout << stream.str();
    return 0;*/
    
    ramses::RamsesObject* carCameraObject = carScene->findObjectByName("Camera[Evaluated for ViewLayer: \"View Layer\"]");
    ramses::PerspectiveCamera* carCamera = ramses::RamsesUtils::TryConvert<ramses::PerspectiveCamera>(*carCameraObject);
    ramses::RamsesObject* carObject = carScene->findObjectByName("RAMSES Root");
    ramses::Node* car = ramses::RamsesUtils::TryConvert<ramses::Node>(*carObject);

    carScene->createTransformationDataConsumer(*car, 222u);

    ramses::SceneGraphIterator meshIterator(*car, ramses::ETreeTraversalStyle_DepthFirst, ramses::ERamsesObjectType_MeshNode);
    ramses::RamsesObject* meshAsObject = meshIterator.getNext();
    while (nullptr != meshAsObject)
    {
        ramses::MeshNode * mesh = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*meshAsObject);
        ramses::Appearance* carAppearance = mesh->getAppearance();
        carAppearance->setDepthFunction(ramses::EDepthFunc_Always);
        meshAsObject = meshIterator.getNext();
    }

    carCamera->setViewport(0, 0, arguments.m_windowWidth, arguments.m_windowHeight);
    carCamera->setFrustum(arguments.m_fovy, aspect, nearPlane, farPlane);

    carScene->publish();
    carScene->flush();

    ramses::sceneId_t mapSceneId = client.getSceneId();
    ramses::sceneId_t carSceneId = carScene->getSceneId();

    constexpr uint32_t renderOrder = 0;
    displayManager.showSceneOnDisplay(mapSceneId, displayId, renderOrder);
    displayManager.showSceneOnDisplay(carSceneId, displayId, renderOrder + 1);

    renderer.linkData(mapSceneId, 111u, carSceneId, 222u);

    Timer frameTime;
    while (!client.shouldExit() && displayManager.isRunning())
    {
        const float dt = frameTime.getTime();
        frameTime.reset();
        client.doFrame(dt);
        renderer.doOneLoop();
        displayManager.dispatchAndFlush();
    }

    renderer.hideScene(mapSceneId);
    renderer.unmapScene(mapSceneId);
    renderer.hideScene(carSceneId);
    renderer.unmapScene(carSceneId);
    renderer.destroyDisplay(displayId);
    renderer.flush();

    return 0;
}
