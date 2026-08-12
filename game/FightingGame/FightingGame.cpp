#include "FightingGame.h"

#include "src/AppState.h"
#include "src/FreeCam2D.h"
#include "src/PhysicsBody2D.h"
#include "Player2D.h"
#include "src/Sprite2D.h"

void FightingGame::Init() {
    AppState* appState = &AppState::Get();

    // auto* camera2d = new Camera2D();
    // appState->current_camera_2d = camera2d;
    // addChild(std::unique_ptr<Node>(camera2d));

    auto* freeCam = new FreeCam2D();
    appState->current_camera_2d = freeCam;
    addChild(std::unique_ptr<Node>(freeCam));

    auto* physicsBody = new PhysicsBody2D(b2_staticBody, 800, 100, static_cast<float>(appState->windowWidth) / 2.0f, 800);
    addChild(std::unique_ptr<Node>(physicsBody));

    auto* editorSprite = new Sprite2D();
    editorSprite->size.x = static_cast<float>(appState->windowWidth);
    physicsBody->addChild(std::unique_ptr<Node>(editorSprite));

    auto* player = new Player2D(100, 100, static_cast<float>(appState->windowWidth) / 2.0f, 0);
    addChild(std::unique_ptr<Node>(player));
}
