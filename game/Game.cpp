#include "Game.h"

#include "src/AppState.h"


Game::Game() {
    name = "root";
}

void Game::Init() {}

void Game::ImGuiDraw() {
    Node::ImGuiDraw();
}

void Game::Update() {
    Node::Update();
}

void Game::Draw(SDL_GPUCommandBuffer *commandBuffer) {
    Node::Draw(commandBuffer);
}

void Game::DrawShadow(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) {
    Node::DrawShadow(commandBuffer, renderPass);
}

void Game::Input() {
    Node::Input();
}

void Game::Event(SDL_Event &event) {
    Node::Event(event);
}
