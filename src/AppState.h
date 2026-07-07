#ifndef FIGMENTENGINE_APPSTATE_H
#define FIGMENTENGINE_APPSTATE_H
#include <string>
#include <unordered_map>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "DirectionalLight3D.h"
#include "SpotLight3D.h"
#include "Node.h"
#include "PointLight3D.h"
#include "Light3DGPU.h"
#include "Transform3D.h"
#include "box2d/box2d.h"
#include "game/FightingGame/FightingGame.h"

struct Mesh;
struct Camera2D;
struct Node;
struct Camera3D;
struct Transform3D;

struct Material;

struct AppState {
    SDL_Window* window = nullptr;
    SDL_GPUDevice* device = nullptr;
    SDL_GPURenderPass *renderPass;
    SDL_GPUTexture* depthTexture = nullptr;
    SDL_GPUTexture* msaaColorTarget = nullptr;
    std::array<SDL_GPUVertexBufferDescription, 1> m_vertexBufferDescriptions;
    std::array<SDL_GPUVertexAttribute, 4> m_vertexAttributes;
    SDL_GPUVertexInputState vertexInputState;
    int msaaSamples = 3;
    bool isMouseRelative = false;

    FightingGame root;
    b2WorldId worldId;

    Mesh* quadMesh = nullptr;
    Camera2D* current_camera_2d;
    Camera3D* current_camera_3d;
    Transform3D modelTransform;

    SDL_GPUTexture* shadowMap = nullptr;
    SDL_GPUGraphicsPipeline* shadowPipeline = nullptr;

    int windowWidth = 1600;
    int windowHeight = 900;
    float sensitivity = 0.05f;
    float currentAspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    std::vector<PointLight3DGPU> pointLightGPUs;
    std::vector<DirectionalLight3DGPU> directionalLightGPUs;
    std::vector<SpotLight3DGPU> spotLightGPUs;
    std::vector<PointLight3D*> pointLights;
    std::vector<DirectionalLight3D*> directionalLights;
    std::vector<SpotLight3D*> spotLights;
    SDL_GPUBuffer* pointLightBuffer;
    SDL_GPUBuffer* directionalLightBuffer;
    SDL_GPUBuffer* spotLightBuffer;
    SDL_GPUTransferBuffer* pointLightTransferBuffer;
    SDL_GPUTransferBuffer* directionalLightTransferBuffer;
    SDL_GPUTransferBuffer* spotLightTransferBuffer;

    std::unordered_map<std::string, Mesh> meshes;
    std::unordered_map<std::string, Material*> materials;
    std::unordered_map<std::string, SDL_GPUShader*> shaders;
    std::unordered_map<std::string, SDL_GPUSampler*> samplers;
    std::unordered_map<std::string, SDL_GPUTexture*> textures;
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> pipelines;
    std::unordered_map<std::string, SDL_GPUColorTargetBlendState> blendStates;
    std::unordered_map<std::string, SDL_GPURasterizerState> rasterizerStates;
    std::unordered_map<std::string, SDL_GPUMultisampleState> multisampleStates;

    Uint64 currentTime = 0;
    Uint64 lastTime = 0;
    Uint64 delta = 0;
    double fixedTimeStepAccumulator = 0;
    double fixedTimeStep = 1.0 / 60.0;

    std::string material_override;

    bool CreatePipeline(const std::string& name, const std::string& vertShader, const std::string& fragShader, const std::string& rasterizerState, const
                        std::string &blendState, const bool &depth_test, const bool &depth_write);

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
    SDL_GPUMultisampleState GetMultisampleState(const std::string &key) const;

    void CreateVertexinputState();
    void CreateDefaultMeshes();
    void CreateDepthTexture();
    void CreateMSAAColorTarget();
    void CreatePointLightBuffer();
    void CreateDirectionalLightBuffer();
    void CreateSpotLightBuffer();
    void CreateDefaultMaterials();
    void CreateDefaultSamplers();
    void CreateDefaultTextures();
    void CreateDefaultPipelines();
    void CreateDefaultBlendStates();
    void CreateDefaultRasterizerStates();
    void CreateDefaultMultisampleStates();
    void CreateShadowMap();
    void CreateShadowPipeline();
    void RenderShadowMap(SDL_GPUCommandBuffer* cmdBuf, const glm::mat4& lightViewProj);
    glm::mat4 GetLightViewProjection() const;

    void RecreateAllMultisampleStates();
    void RecreateAllPipelines();

    // EDITOR
    bool debug = true;
    Node* editorSelected = nullptr;
    std::string editorMesh;
    std::string editorSprite;
};




#endif //FIGMENTENGINE_APPSTATE_H
