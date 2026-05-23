#include "MaterialUnlitTextured.h"

MaterialUnlitTextured::MaterialUnlitTextured(const std::string &name, const std::string &pipeline, const std::string& sampler, const std::string &textureAlbedo) : textureAlbedo(textureAlbedo) {
    this->name = name;
    this->pipeline = pipeline;
    this->sampler = sampler;
}
