#include <vector>
#include <SDL3/SDL.h>

#include "box2d/box2d.h"
#include "box3d/box3d.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_sdl3.h"
#include "thirdparty/imgui/imgui_impl_sdlgpu3.h"
#include "thirdparty/imgui/imgui_stdlib.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "Mesh.h"
#include "MeshInstance3D.h"
#define SDL_MAIN_USE_CALLBACKS
#include <ranges>
#include <SDL3/SDL_main.h>

#include "AppState.h"
#include "Camera3D.h"
#include "DirectionalLight3D.h"
#include "EditorThemeManager.h"
#include "Input.h"
#include "Material.h"
#include "Sprite2D.h"
#include "Vertex.h"
#include "thirdparty/tiny_gltf.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

void HandleInput(AppState* appState) {
    appState->root.Input(*appState);
    if (Input::IsJustPressed(SDL_SCANCODE_X)) {
        appState->debug = !appState->debug;
    }
}

void FixedDelta(AppState* appState) {
    appState->lastTime = appState->currentTime;
    appState->currentTime = SDL_GetTicks();
    appState->delta = appState->currentTime - appState->lastTime;

    double frameTimeSeconds = static_cast<double>(appState->delta) / 1000.0;
    constexpr double MAX_FRAME_TIME = 0.25;
    if (frameTimeSeconds > MAX_FRAME_TIME) frameTimeSeconds = MAX_FRAME_TIME;

    appState->fixedTimeStepAccumulator += frameTimeSeconds;

    while (appState->fixedTimeStepAccumulator >= appState->fixedTimeStep) {
        HandleInput(appState);
        appState->root.FixedUpdate(*appState);
        b2World_Step(appState->worldId2, static_cast<float>(appState->fixedTimeStep), 4);
        b3World_Step(appState->worldId3, static_cast<float>(appState->fixedTimeStep), 4);
        appState->fixedTimeStepAccumulator -= appState->fixedTimeStep;
        Input::UpdateInputs();
    }
}

void HandleUpdate(AppState* appState) {
    appState->root.Update(*appState);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    #ifdef __linux__
        if (dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD) != nullptr) {
            SDL_Log("librenderdoc.so loaded. Forcing X11.");
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
        }
    #endif
    auto* appState = new AppState();
    *appstate = appState;

    b2WorldDef worldDef2 = b2DefaultWorldDef();
    worldDef2.gravity = (b2Vec2){0.0f, 19.6f};
    appState->worldId2 = b2CreateWorld(&worldDef2);

    b3WorldDef worldDef3 = b3DefaultWorldDef();
    worldDef3.gravity = (b3Vec3){0.0f, -39.2, 0.0f};
    appState->worldId3 = b3CreateWorld(&worldDef3);

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    if (main_scale < 1.0f) {
        main_scale = 1.0f;
    }
    if (main_scale == 0.0f) {
        SDL_Log("SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()) returned 0.0f: %s", SDL_GetError());
    }
    constexpr SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    appState->window = SDL_CreateWindow("FigmentEngine", appState->windowWidth, appState->windowHeight, window_flags);
    if (appState->window == nullptr)
    {
        SDL_Log("Error: SDL_CreateWindow(): %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowAspectRatio(appState->window, 16.0f / 9, 16.0f / 9);
    SDL_MaximizeWindow(appState->window);

    constexpr SDL_GPUShaderFormat formatFlags = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL;
    appState->device = SDL_CreateGPUDevice(formatFlags, true, nullptr);
    if (appState->device == nullptr)
    {
        SDL_Log("Error: SDL_CreateGPUDevice(): %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(appState->device, appState->window))
    {
        SDL_Log("Error: SDL_ClaimWindowForGPUDevice(): %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetGPUSwapchainParameters(appState->device, appState->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);

    appState->CreateVertexinputState();
    appState->CreateDefaultBlendStates();
    appState->CreateDefaultMultisampleStates();
    appState->CreateDefaultMeshes();
    appState->CreateDefaultTextures();
    appState->CreateDefaultMaterials();
    appState->CreateDefaultSamplers();
    appState->CreateDefaultRasterizerStates();
    appState->CreateDefaultPipelines();
    appState->CreateMSAAColorTarget();
    appState->CreateDepthTexture();
    appState->CreatePointLightBuffer();
    appState->CreateDirectionalLightBuffer();
    appState->CreateSpotLightBuffer();
    appState->CreateShadowPipeline();
    appState->CreateShadowMap();

    appState->quadMesh = new Mesh();
    appState->quadMesh->CreateQuad(1, 1, -1);
    appState->quadMesh->UploadToGPU(*appState);

    // IMGUI_CHECKVERSION(); // Crashes on ImGui docking branch (?)
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

    EditorThemeManager::ApplyImGuiTheme();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForSDLGPU(appState->window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = appState->device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(appState->device, appState->window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;  // Only used in multi-viewports mode.
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    ImGui_ImplSDLGPU3_Init(&init_info);

    appState->root.Init(*appState);

    return SDL_APP_CONTINUE;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto* appState = static_cast<AppState*>(appstate);
    if (appState->debug) {
        ImGui_ImplSDL3_ProcessEvent(event);
    }

    appState->root.Event(*appState, *event);

    switch (event->type)
    {
        case SDL_EVENT_WINDOW_RESIZED:
            appState->windowWidth = event->window.data1;
            appState->windowHeight = event->window.data2;
            appState->CreateMSAAColorTarget();
            appState->CreateDepthTexture();
            appState->RecreateAllPipelines();
            return SDL_APP_CONTINUE;
        case SDL_EVENT_MOUSE_MOTION:
            return SDL_APP_CONTINUE;
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            return SDL_APP_CONTINUE;
    }
}

void DrawNodeTree(AppState* appState, Node* node) {
    ImGui::PushID(node);

    int flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    if (node == appState->editorSelected)
        flags |= ImGuiTreeNodeFlags_Selected;
    if (node->children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;
    if (ImGui::TreeNodeEx(node->name.c_str(), flags)) {
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            appState->editorSelected = node;
        }
        for (auto& child : node->children) {
            DrawNodeTree(appState, child.get());
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void PreparePointLightBuffer(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    // gpuLights is a vector of structs that store point light data. Here we clear this list, so we can upload the latest light data to the GPU.
    appState->pointLightGPUs.clear();
    appState->pointLightGPUs.reserve(appState->pointLights.size());

    // repopulate gpuLights
    for (const PointLight3D* light : appState->pointLights) {
        PointLight3DGPU gpu;
        gpu.color = glm::vec4(light->color, light->brightness);
        gpu.position = glm::vec4(light->GetGlobalTransform().position, 0);
        gpu.params = glm::vec4(light->constant, light->linear, light->quadratic, 0);
        appState->pointLightGPUs.push_back(gpu);
    }

    // transfer the light data into the light buffer
    if (void* mapped = SDL_MapGPUTransferBuffer(appState->device, appState->pointLightTransferBuffer, false)) {
        memcpy(mapped, appState->pointLightGPUs.data(), appState->pointLightGPUs.size() * sizeof(PointLight3DGPU));
        SDL_UnmapGPUTransferBuffer(appState->device, appState->pointLightTransferBuffer);
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation src = {};
    src.transfer_buffer = appState->pointLightTransferBuffer;
    src.offset = 0;

    SDL_GPUBufferRegion dst = {};
    dst.buffer = appState->pointLightBuffer;
    dst.offset = 0;
    dst.size = appState->pointLightGPUs.size() * sizeof(PointLight3DGPU);

    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    SDL_EndGPUCopyPass(copyPass);
}

void PrepareDirectionalLightBuffer(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    appState->directionalLightGPUs.clear();
    appState->directionalLightGPUs.reserve(appState->directionalLights.size());

    for (const DirectionalLight3D* light : appState->directionalLights) {
        DirectionalLight3DGPU gpu;
        gpu.color = glm::vec4(light->color, light->brightness);
        gpu.direction = glm::vec4(light->GetGlobalTransform().getForward(), 0);
        appState->directionalLightGPUs.push_back(gpu);
    }

    if (void* mapped = SDL_MapGPUTransferBuffer(appState->device, appState->directionalLightTransferBuffer, false)) {
        memcpy(mapped, appState->directionalLightGPUs.data(), appState->directionalLightGPUs.size() * sizeof(DirectionalLight3DGPU));
        SDL_UnmapGPUTransferBuffer(appState->device, appState->directionalLightTransferBuffer);
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation src = {};
    src.transfer_buffer = appState->directionalLightTransferBuffer;
    src.offset = 0;

    SDL_GPUBufferRegion dst = {};
    dst.buffer = appState->directionalLightBuffer;
    dst.offset = 0;
    dst.size = appState->directionalLightGPUs.size() * sizeof(DirectionalLight3DGPU);

    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    SDL_EndGPUCopyPass(copyPass);
}

void PrepareSpotLightBuffer(AppState *appState, SDL_GPUCommandBuffer *commandBuffer) {
    appState->spotLightGPUs.clear();
    appState->spotLightGPUs.reserve(appState->spotLights.size());

    for (const SpotLight3D* light : appState->spotLights) {
        SpotLight3DGPU gpu;
        gpu.color = glm::vec4(light->color, light->brightness);
        gpu.position = glm::vec4(light->GetGlobalTransform().position, glm::cos(glm::radians(light->cutoff)));
        gpu.direction = glm::vec4(light->GetGlobalTransform().getForward(), glm::cos(glm::radians(light->outerCutoff)));
        gpu.params = glm::vec4(light->constant, light->linear, light->quadratic, 0);
        appState->spotLightGPUs.push_back(gpu);
    }

    if (void* mapped = SDL_MapGPUTransferBuffer(appState->device, appState->spotLightTransferBuffer, false)) {
        memcpy(mapped, appState->spotLightGPUs.data(), appState->spotLightGPUs.size() * sizeof(SpotLight3DGPU));
        SDL_UnmapGPUTransferBuffer(appState->device, appState->spotLightTransferBuffer);
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation src = {};
    src.transfer_buffer = appState->spotLightTransferBuffer;
    src.offset = 0;

    SDL_GPUBufferRegion dst = {};
    dst.buffer = appState->spotLightBuffer;
    dst.offset = 0;
    dst.size = appState->spotLightGPUs.size() * sizeof(SpotLight3DGPU);

    SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

    SDL_EndGPUCopyPass(copyPass);
}

SDL_AppResult RenderFrame(AppState* appState) {
    if (appState->debug) {
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Node Heirarchy");
        DrawNodeTree(appState, &appState->root);
        ImGui::End();

        ImGui::Begin("Debug");

        ImGui::Text("Material Override");
        static const char* mat_items[] = { "", "pbr_orm", "pbr", "phong", "phong_textured", "blinn_phong", "blinn_phong_textured", "missing", "line" };
        static int mat_selected_idx = 0;
        ImGui::Combo("##Override", &mat_selected_idx, mat_items, IM_ARRAYSIZE(mat_items));
        appState->material_override = mat_items[mat_selected_idx];

        ImGui::Text("Mesh Spawner");
        std::vector<const char*> mesh_items;
        mesh_items.reserve(appState->meshes.size() + 1);
        mesh_items.push_back("");
        for (const auto &key: appState->meshes | std::views::keys)
            mesh_items.push_back(key.c_str());
        static int mesh_index = 0;
        if (ImGui::Combo("##Mesh", &mesh_index, mesh_items.data(), static_cast<int>(mesh_items.size()))) {
            appState->editorMesh = mesh_items[mesh_index];
        }

        if (ImGui::Button("Spawn Mesh") && appState->meshes.contains(appState->editorMesh)) {
            auto* meshInstance = new MeshInstance3D();
            meshInstance->mesh = appState->editorMesh;
            SDL_Log("Mesh spawned: %s", appState->editorMesh.c_str());
            meshInstance->localTransform.position = appState->current_camera_3d->GetGlobalTransform().position;
            meshInstance->localTransform.rotation = appState->current_camera_3d->GetGlobalTransform().rotation * glm::vec3(0.0f, 1.0f, 0.0f);
            appState->root.addChild(std::unique_ptr<Node>(meshInstance));
        }

        ImGui::Text("Sprite Spawner");

        std::vector<const char*> sprite_items;
        sprite_items.reserve(appState->textures.size() + 1);
        sprite_items.push_back("");
        for (const auto &key: appState->textures | std::views::keys)
            sprite_items.push_back(key.c_str());
        static int sprite_index = 0;
        if (ImGui::Combo("##Sprite", &sprite_index, sprite_items.data(), static_cast<int>(sprite_items.size()))) {
            appState->editorSprite = sprite_items[sprite_index];
        }

        if (ImGui::Button("Spawn Sprite") && appState->textures.contains(appState->editorSprite)) {
            auto* sprite = new Sprite2D();
            SDL_Log("Sprite spawned: %s", appState->editorSprite.c_str());
            appState->root.addChild(std::unique_ptr<Node>(sprite));
        }

        if (ImGui::Button("Spawn 100 Lights")) {
            for (int i = 0; i < 100; i++) {
                auto* pointLight = new PointLight3D(appState);
                pointLight->localTransform.position.x = 50 - static_cast<float>(rand() % 100); // NOLINT(*-msc50-cpp)
                pointLight->localTransform.position.y = rand() % 10; // NOLINT(*-narrowing-conversions, *-msc50-cpp)
                pointLight->localTransform.position.z = 50 - static_cast<float>(rand() % 100); // NOLINT(*-msc50-cpp)

                pointLight->color.r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // NOLINT(*-msc50-cpp)
                pointLight->color.g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // NOLINT(*-msc50-cpp)
                pointLight->color.b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // NOLINT(*-msc50-cpp)
                pointLight->constant = 0.01f;
                appState->root.addChild(std::unique_ptr<Node>(pointLight));
            }
        }

        ImGui::End();
        if (appState->editorSelected != nullptr) {
            ImGui::Begin("Inspector");
            appState->editorSelected->ImGuiDraw();
            ImGui::End();
        }

        ImGui::Begin("Project Settings");
        if (ImGui::InputInt("MSAA Samples", &appState->msaaSamples)) {
            appState->CreateMSAAColorTarget();
            appState->CreateDepthTexture();
            appState->RecreateAllPipelines();
        }
        ImGui::End();

        ImGui::Render();
    }
    ImDrawData* draw_data = ImGui::GetDrawData();

    // Here's where we store the commands that will be eventually sent to the GPU
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(appState->device);
    if (commandBuffer == nullptr)
    {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!appState->directionalLights.empty()) {
        appState->RenderShadowMap(commandBuffer, appState->GetOffsetLightViewProjection());
    }

    if (appState->debug) {
        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, commandBuffer);
    }
    // Here we create the texture that will be drawn to the screen once it's done
    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appState->window, &swapchainTexture, nullptr, nullptr))
    {
        SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != nullptr) {
        SDL_GPUColorTargetInfo colorTargetInfo = {};
        colorTargetInfo.mip_level = 0;
        colorTargetInfo.layer_or_depth_plane = 0;
        colorTargetInfo.clear_color = SDL_FColor{0.4f, 0.6f, 0.9f, 1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.resolve_mip_level = 0;
        colorTargetInfo.resolve_layer = 0;
        colorTargetInfo.cycle = false;
        colorTargetInfo.cycle_resolve_texture = false;

        if (appState->msaaSamples == 0) {
            // NO MSAA: Render directly to swapchain
            colorTargetInfo.texture = swapchainTexture;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            colorTargetInfo.resolve_texture = nullptr;
        } else {
            // MSAA: Render to MSAA target, resolve to swapchain
            colorTargetInfo.texture = appState->msaaColorTarget;
            colorTargetInfo.store_op = SDL_GPU_STOREOP_RESOLVE;
            colorTargetInfo.resolve_texture = swapchainTexture;
        }

        const SDL_GPUDepthStencilTargetInfo depthTarget = {
            .texture = appState->depthTexture,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
            .stencil_load_op = SDL_GPU_LOADOP_CLEAR,
            .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
            .cycle = true
        };

        PreparePointLightBuffer(appState, commandBuffer);
        PrepareDirectionalLightBuffer(appState, commandBuffer);
        PrepareSpotLightBuffer(appState, commandBuffer);

        // Now we actually define the render pass, by passing &colorTargetInfo, which modified our swapchainTexture to become cleared
        appState->renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTarget);
        if (!appState->renderPass) {
            SDL_Log("Couldn't begin render pass: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }

        appState->root.Draw(*appState, commandBuffer);

        SDL_EndGPURenderPass(appState->renderPass);
        appState->renderPass = nullptr;

        if (appState->debug) {
            // Start a new render pass targeting the swapchain directly
            SDL_GPUColorTargetInfo uiTargetInfo = {};
            uiTargetInfo.texture = swapchainTexture;
            uiTargetInfo.mip_level = 0;
            uiTargetInfo.layer_or_depth_plane = 0;
            uiTargetInfo.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f};
            uiTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
            uiTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            uiTargetInfo.resolve_texture = nullptr;
            uiTargetInfo.cycle = false;

            if (SDL_GPURenderPass* uiPass = SDL_BeginGPURenderPass(commandBuffer, &uiTargetInfo, 1, nullptr)) {
                ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer, uiPass);
                SDL_EndGPURenderPass(uiPass);
            }
        }
    }
    // Send the command buffer to the GPU for drawing
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto* appState = static_cast<AppState*>(appstate);

    FixedDelta(appState);
    HandleUpdate(appState);
    return RenderFrame(appState);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    const auto appState = static_cast<AppState*>(appstate);

    SDL_WaitForGPUIdle(appState->device);

    for (const auto &material: appState->materials | std::views::values) {delete material;}
    appState->materials.clear();

    for (const auto &texture: appState->textures | std::views::values) {
        if (texture) {
            SDL_ReleaseGPUTexture(appState->device, texture);
        }
    }
    appState->textures.clear();
    SDL_ReleaseGPUTexture(appState->device, appState->depthTexture);
    SDL_ReleaseGPUTexture(appState->device, appState->shadowMap);
    SDL_ReleaseGPUTexture(appState->device, appState->msaaColorTarget);

    for (const auto &sampler: appState->samplers | std::views::values) {
        if (sampler) {
            SDL_ReleaseGPUSampler(appState->device, sampler);
        }
    }
    appState->samplers.clear();

    for (auto &mesh: appState->meshes | std::views::values) {
        mesh.ReleaseGPUResources(appState);
    }
    appState->meshes.clear();
    appState->quadMesh->ReleaseGPUResources(appState);

    for (const auto &pipeline: appState->pipelines | std::views::values) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(appState->device, pipeline);
        }
    }
    appState->pipelines.clear();
    SDL_ReleaseGPUGraphicsPipeline(appState->device, appState->shadowPipeline);

    for (const auto &shader: appState->shaders | std::views::values) {
        if (shader) {
            SDL_ReleaseGPUShader(appState->device, shader);
        }
    }
    appState->shaders.clear();

    SDL_ReleaseGPUBuffer(appState->device, appState->pointLightBuffer);
    SDL_ReleaseGPUBuffer(appState->device, appState->directionalLightBuffer);
    SDL_ReleaseGPUBuffer(appState->device, appState->spotLightBuffer);

    SDL_ReleaseGPUTransferBuffer(appState->device, appState->pointLightTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(appState->device, appState->directionalLightTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(appState->device, appState->spotLightTransferBuffer);

    b2DestroyWorld(appState->worldId2);
    b3DestroyWorld(appState->worldId3);

    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(appState->device);
    SDL_DestroyWindow(appState->window);
    delete appState;
}