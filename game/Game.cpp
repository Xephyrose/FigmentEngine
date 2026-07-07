#include "Game.h"

#include "src/AppState.h"


Game::Game() {
    name = "root";
}

void Game::Init(AppState& appState) {}

void Game::ImGuiDraw() {
    Node::ImGuiDraw();
}

void Game::Update(AppState &appState) {
    Node::Update(appState);
}

void Game::Draw(AppState &appState, SDL_GPUCommandBuffer *commandBuffer) {
    Node::Draw(appState, commandBuffer);
}

void Game::DrawShadow(AppState &appState, SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) {
    Node::DrawShadow(appState, commandBuffer, renderPass);
}

void Game::Input(AppState &appState) {
    Node::Input(appState);
}

void Game::Event(AppState &appState, SDL_Event &event) {
    Node::Event(appState, event);
}
