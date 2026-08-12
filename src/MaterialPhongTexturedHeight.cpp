#include "MaterialPhongTexturedHeight.h"
#include <SDL3/SDL_log.h>

#include "Camera3D.h"

MaterialPhongTexturedHeight::MaterialPhongTexturedHeight(AppState *appState, const std::string &name, const std::string &pipeline, const std::vector<BlinnPhongMaterialLayer> &initial_layers) : layers(initial_layers) {
    this->name = name;
    this->pipeline = pipeline;
    appState->materials.insert_or_assign(name, this);
}

void MaterialPhongTexturedHeight::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {

}

void MaterialPhongTexturedHeight::setSampler(AppState *appState, const std::string &new_sampler) {
    sampler = new_sampler;
}
