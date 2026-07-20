#include "ShooterGame.h"

#include "QuakePlayer3D.h"
#include "src/AppState.h"
#include "src/BoxBody3D.h"
#include "src/DirectionalLight3D.h"
#include "src/MeshInstance3D.h"

void ShooterGame::Init(AppState &appState) {

    auto* box = new BoxBody3D(appState, b3_staticBody, 0, -0.738988f, 0, 100, 0.01f, 100);
    addChild(std::unique_ptr<Node>(box));

    auto* player = new QuakePlayer3D(appState, 17, 5, 23, 0.5, 2);
    addChild(std::unique_ptr<Node>(player));

    // auto* pointLight = new PointLight3D(&appState);
    // pointLight->localTransform.position.x = -43;
    // pointLight->localTransform.position.y = 5;
    // pointLight->localTransform.position.z = -14;
    // pointLight->color.x = 0.5f;
    // addChild(std::unique_ptr<Node>(pointLight));

    // auto* pointLight1 = new PointLight3D(&appState);
    // pointLight1->localTransform.position.x = -36;
    // pointLight1->localTransform.position.y = 5;
    // pointLight1->color.y  = 0.5f;
    // addChild(std::unique_ptr<Node>(pointLight1));

    auto* pointLight2 = new PointLight3D(&appState);
    pointLight2->localTransform.position.x = 17;
    pointLight2->localTransform.position.y = 5;
    pointLight2->localTransform.position.z = 6;
    pointLight2->brightness = 50;
    addChild(std::unique_ptr<Node>(pointLight2));

    auto* directionalLight = new DirectionalLight3D(&appState);
    directionalLight->localTransform.rotation.x = -135;
    directionalLight->localTransform.rotation.y = -135;
    directionalLight->localTransform.position.x = 50;
    directionalLight->localTransform.position.z = 50;
    directionalLight->brightness = 5;
    addChild(std::unique_ptr<Node>(directionalLight));

    auto* spotLight = new SpotLight3D(&appState);
    spotLight->localTransform.position.x = 17;
    spotLight->localTransform.position.y = 1;
    spotLight->localTransform.position.z = 5;
    addChild(std::unique_ptr<Node>(spotLight));

    auto* meshInstance = new MeshInstance3D();
    meshInstance->mesh = "zoo.glb";
    addChild(std::unique_ptr<Node>(meshInstance));
}

