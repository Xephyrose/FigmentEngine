#include "ShooterGame.h"

#include "src/AppState.h"
#include "src/DirectionalLight3D.h"
#include "src/FreeCam3D.h"
#include "src/MeshInstance3D.h"

void ShooterGame::Init(AppState &appState) {

    auto* freeCam = new FreeCam3D();
    appState.current_camera_3d = freeCam;
    freeCam->localTransform.position = glm::vec3(-43, 8, 14);
    freeCam->localTransform.setRotation(glm::vec3(-34, 0, 0));
    addChild(std::unique_ptr<Node>(freeCam));

    // auto* pointLight = new PointLight3D(appState);
    // pointLight->localTransform.position.x = -43;
    // pointLight->localTransform.position.y = 5;
    // pointLight->localTransform.position.z = -14;
    // pointLight->color.x = 0.5f;
    // pointLight->constant = 0;
    // addChild(std::unique_ptr<Node>(pointLight));
    //
    // auto* pointLight1 = new PointLight3D(appState);
    // pointLight1->localTransform.position.x = -36;
    // pointLight1->localTransform.position.y = 5;
    // pointLight1->color.y  = 0.5f;
    // pointLight1->constant = 0;
    // addChild(std::unique_ptr<Node>(pointLight1));

    auto* directionalLight = new DirectionalLight3D(&appState);
    // directionalLight->localTransform.rotation.x = -9;
    // directionalLight->localTransform.rotation.y = 45;
    directionalLight->localTransform.position.x = -2.0f;
    directionalLight->localTransform.position.y = 4;
    directionalLight->localTransform.position.z = -2.0f;
    addChild(std::unique_ptr<Node>(directionalLight));

    // auto* spotLight = new SpotLight3D(appState);
    // spotLight->localTransform.position.x = -46;
    // spotLight->localTransform.position.y = 1;
    // spotLight->localTransform.position.z = 14;
    // spotLight->brightness = 27;
    // // spotLight->intensity = 0;
    // addChild(std::unique_ptr<Node>(spotLight));

    auto* meshInstance = new MeshInstance3D();
    meshInstance->mesh = "zulu.glb";
    addChild(std::unique_ptr<Node>(meshInstance));

    auto* meshInstance2 = new MeshInstance3D();
    meshInstance2->mesh = "lynx.glb";
    meshInstance2->localTransform.position = glm::vec3(0.35f, -0.5f, -0.25f);
    meshInstance2->localTransform.rotation = glm::vec3(0.0f, 180, 0.0f);
    freeCam->addChild(std::unique_ptr<Node>(meshInstance2));
}

