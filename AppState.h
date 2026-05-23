#ifndef FIGMENTENGINE_APPSTATE_H
#define FIGMENTENGINE_APPSTATE_H
#include <vector>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "Material.h"
#include "Transform3D.h"

class Node;
class Camera3D;
struct Transform3D;

struct AppState {
    SDL_Window* window = nullptr;
    SDL_GPUDevice* device = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;

    SDL_GPUTexture* depthTexture = nullptr;
    SDL_GPUBuffer* vertexBuffer = nullptr;
    SDL_GPUBuffer* indexBuffer = nullptr;
    Uint32 numVertexes = 0;
    Uint32 numIndices = 0;

    SDL_GPUTexture* texture = nullptr;
    SDL_GPUSampler* sampler = nullptr;

    std::vector<Node*> nodes;

    mutable Camera3D* current_camera;
    Transform3D modelTransform;

    int window_width = 1920;
    int window_height = 1080;

    float sensitivity = 0.05f;

    float currentAspectRatio = static_cast<float>(window_width) / static_cast<float>(window_height);

    std::unordered_map<std::string, SDL_GPUShader*> shaders;
    std::unordered_map<std::string, SDL_GPUTexture*> textures;
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> pipelines;

    std::unordered_map<std::string, Material> materials;

    Uint64 currentTime = 0;
    Uint64 lastTime = 0;
    Uint64 delta = 0;

    bool LoadShader(const std::string& shaderFilename);
    bool LoadTexture(const std::string& texturePath);

    SDL_GPUShader* GetShader(const std::string& shaderFilename);
    SDL_GPUTexture* GetTexture(const std::string& shaderFilename);
};




#endif //FIGMENTENGINE_APPSTATE_H
