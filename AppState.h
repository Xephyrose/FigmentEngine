#ifndef FIGMENTENGINE_APPSTATE_H
#define FIGMENTENGINE_APPSTATE_H
#include <string>
#include <unordered_map>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "Node.h"
#include "Transform3D.h"

struct Node;
struct Camera3D;
struct Transform3D;

struct Material;
struct Mesh;

struct AppState {
    SDL_Window* window = nullptr;
    SDL_GPUDevice* device = nullptr;
    SDL_GPURenderPass *renderPass;
    SDL_GPUTexture* depthTexture = nullptr;
    bool isMouseRelative = false;

    Node root = Node("root");

    bool debug = true;
    Node* editorSelected = nullptr;
    std::string mesh;

    Camera3D* current_camera;
    Transform3D modelTransform;

    int windowWidth = 1920;
    int windowHeight = 1080;
    float sensitivity = 0.05f;
    float currentAspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    std::unordered_map<std::string, Mesh> meshes;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, SDL_GPUShader*> shaders;
    std::unordered_map<std::string, SDL_GPUSampler*> samplers;
    std::unordered_map<std::string, SDL_GPUTexture*> textures;
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> pipelines;
    std::unordered_map<std::string, SDL_GPUColorTargetBlendState> blendStates;
    std::unordered_map<std::string, SDL_GPURasterizerState> rasterizerStates;

    Uint64 currentTime = 0;
    Uint64 lastTime = 0;
    Uint64 delta = 0;

    std::string material_override;

    bool CreatePipeline(const std::string& name, const std::string& vertShader, const std::string& fragShader, const std::string& rasterizerState, const
                        std::string &blendState);

    bool LoadMesh(const std::string& path);
    bool LoadShader(const std::string& path);
    bool LoadTexture(const std::string& path);

    Mesh* GetMesh(const std::string& path);
    Material* GetMaterial(const std::string& key) const;
    SDL_GPUShader* GetShader(const std::string& path);
    SDL_GPUSampler* GetSampler(const std::string& key) const;
    SDL_GPUTexture* GetTexture(const std::string& path);
    SDL_GPUGraphicsPipeline* GetPipeline(const std::string& key) const;
    SDL_GPUColorTargetBlendState GetBlendState(const std::string &key) const;
    SDL_GPURasterizerState GetRasterizerState(const std::string &key) const;

    void CreateDefaultMeshes();
    void CreateDefaultMaterials();
    void CreateDefaultSamplers();
    void CreateDefaultTextures();
    void CreateDefaultPipelines();
    void CreateDefaultBlendStates();
    void CreateDefaultRasterizerStates();
};




#endif //FIGMENTENGINE_APPSTATE_H
