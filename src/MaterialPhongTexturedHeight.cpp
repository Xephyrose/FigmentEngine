#include "MaterialPhongTexturedHeight.h"

MaterialPhongTexturedHeight::MaterialPhongTexturedHeight(const std::string &name, const std::string &pipeline, const std::vector<BlinnPhongMaterialLayer> &initial_layers) : layers(initial_layers) {
    this->name = name;
    this->pipeline = pipeline;
    AppState::Get().materials.insert_or_assign(name, this);
}

void MaterialPhongTexturedHeight::Bind(AppState *appState, SDL_GPUCommandBuffer *commandBuffer, glm::mat4 model) {

}

void MaterialPhongTexturedHeight::setSampler(const std::string &new_sampler) {
    sampler = new_sampler;
}

void MaterialPhongTexturedHeight::setTextureAlbedo(const std::string &texture) {
}

void MaterialPhongTexturedHeight::setTextureAmbient(const std::string &texture) {
}

void MaterialPhongTexturedHeight::setTextureSpecular(const std::string &texture) {
}

void MaterialPhongTexturedHeight::setTextureNormalMap(const std::string &texture) {
}
