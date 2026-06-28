#include <vector>
#include <SDL3/SDL.h>
#include "box2d/box2d.h"
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
#include "FreeCam3D.h"
#include "Input.h"
#include "Material.h"
#include "Sprite2D.h"
#include "Vertex.h"
#include "thirdparty/tiny_gltf.h"

#include "EditorThemeManager.h"

#ifdef __linux__
#include <dlfcn.h>
#endif

void FixedDelta(AppState* appState) {
    appState->lastTime = appState->currentTime;
    appState->currentTime = SDL_GetTicks();
    appState->delta = appState->currentTime - appState->lastTime;

    double frameTimeSeconds = static_cast<double>(appState->delta) / 1000.0;
    constexpr double MAX_FRAME_TIME = 0.25;
    if (frameTimeSeconds > MAX_FRAME_TIME) frameTimeSeconds = MAX_FRAME_TIME;

    appState->fixedTimeStepAccumulator += frameTimeSeconds;

    while (appState->fixedTimeStepAccumulator >= appState->fixedTimeStep) {
        b2World_Step(appState->worldId, static_cast<float>(appState->fixedTimeStep), 4);
        appState->fixedTimeStepAccumulator -= appState->fixedTimeStep;
    }
}

void HandleInput(AppState* appState) {
    appState->root.Input(*appState);
    if (Input::IsJustPressed(SDL_SCANCODE_X)) {
        appState->debug = !appState->debug;
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

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 9.8f};
    appState->worldId = b2CreateWorld(&worldDef);

    auto* freeCam = new FreeCam3D();
    appState->current_camera_3d = freeCam;
    freeCam->localTransform.position = glm::vec3(-43, 8, 14);
    freeCam->localTransform.setRotation(glm::vec3(-34, 0, 0));
    appState->root.addChild(std::unique_ptr<Node>(freeCam));

    auto* pointLight = new PointLight3D(appState);
    pointLight->localTransform.position.x = -43;
    pointLight->localTransform.position.y = 5;
    appState->root.addChild(std::unique_ptr<Node>(pointLight));

    // auto* camera2d = new Camera2D();
    // appState->current_camera_2d = camera2d;
    // appState->root.addChild(std::unique_ptr<Node>(camera2d));

    // auto* freeCam = new FreeCam2D();
    // appState->current_camera_2d = freeCam;
    // appState->root.addChild(std::unique_ptr<Node>(freeCam));

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
    SDL_SetWindowAspectRatio(appState->window, 1.777f, 1.777f);
    SDL_MaximizeWindow(appState->window);

    constexpr SDL_GPUShaderFormat formatFlags = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
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
    appState->CreateLightBuffers();

    appState->quadMesh = new Mesh();
    appState->quadMesh->CreateQuad(1, 1, -1);
    appState->quadMesh->UploadToGPU(*appState);

    auto* meshInstance = new MeshInstance3D();
    meshInstance->mesh = "zulu.glb";
    appState->root.addChild(std::unique_ptr<Node>(meshInstance));

    auto* meshInstance2 = new MeshInstance3D();
    meshInstance2->mesh = "lynx.glb";
    meshInstance2->localTransform.position = glm::vec3(0.35f, -0.5f, -0.25f);
    meshInstance2->localTransform.rotation = glm::vec3(0.0f, 180, 0.0f);
    freeCam->addChild(std::unique_ptr<Node>(meshInstance2));

    // auto* physicsBody = new PhysicsBody2D(*appState, b2_staticBody, 800, 100, appState->windowWidth / 2.0f, 800);
    // appState->root.addChild(std::unique_ptr<Node>(physicsBody));
    //
    // auto* editorSprite = new Sprite2D();
    // editorSprite->size.x = appState->windowWidth;
    // physicsBody->addChild(std::unique_ptr<Node>(editorSprite));
    //
    // auto* player = new Player2D(*appState, 100, 100, appState->windowWidth / 2.0f, 0);
    // appState->root.addChild(std::unique_ptr<Node>(player));

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
        ImGui::Text("Mesh Spawner");
        ImGui::InputText("Mesh", &appState->editorMesh);
        ImGui::SameLine();
        if (ImGui::Button("Spawn Mesh")) {
            auto* meshInstance = new MeshInstance3D();
            meshInstance->mesh = appState->editorMesh;
            SDL_Log("Mesh spawned: %s", appState->editorMesh.c_str());
            meshInstance->localTransform.position = appState->current_camera_3d->GetGlobalTransform().position;
            meshInstance->localTransform.rotation = appState->current_camera_3d->GetGlobalTransform().rotation * glm::vec3(0.0f, 1.0f, 0.0f);
            appState->root.addChild(std::unique_ptr<Node>(meshInstance));
        }
        ImGui::Text("Sprite Spawner");
        ImGui::InputText("Sprite", &appState->editorSprite);
        ImGui::SameLine();
        if (ImGui::Button("Spawn Sprite")) {
            auto* sprite = new Sprite2D();
            // SDL_Log("Sprite spawned: %s", appState->editorSprite.c_str());
            appState->root.addChild(std::unique_ptr<Node>(sprite));
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

        ImGui::Begin("Rendering Overrides");
        static const char* items[] = { "", "phong", "phong_textured", "blinn_phong", "blinn_phong_textured", "missing", "line", "uvs" };
        static int selected_idx = 0;
        ImGui::Combo("Override", &selected_idx, items, IM_ARRAYSIZE(items));
        appState->material_override = items[selected_idx];
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

        // gpuLights is a vector of structs that store point light data. Here we clear this list, so we can upload the latest light data to the GPU.
        appState->gpuLights.clear();
        appState->gpuLights.reserve(appState->pointLights.size());

        // repopulate gpuLights
        for (const PointLight3D* light : appState->pointLights) {
            PointLight3DGPU gpu;
            gpu.position = glm::vec4(light->GetGlobalTransform().position, 0.0f);
            gpu.color = glm::vec4(light->color, light->intensity);
            appState->gpuLights.push_back(gpu);
        }

        // transfer the light data into the light buffer
        if (void* mapped = SDL_MapGPUTransferBuffer(appState->device, appState->lightTransferBuffer, false)) {
            memcpy(mapped, appState->gpuLights.data(), appState->gpuLights.size() * sizeof(PointLight3DGPU));
            SDL_UnmapGPUTransferBuffer(appState->device, appState->lightTransferBuffer);
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

        SDL_GPUTransferBufferLocation src = {};
        src.transfer_buffer = appState->lightTransferBuffer;
        src.offset = 0;

        SDL_GPUBufferRegion dst = {};
        dst.buffer = appState->lightBuffer;
        dst.offset = 0;
        dst.size = appState->gpuLights.size() * sizeof(PointLight3DGPU);

        SDL_UploadToGPUBuffer(copyPass, &src, &dst, false);

        SDL_EndGPUCopyPass(copyPass);

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
            uiTargetInfo.clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 0.0f}; // Don't clear
            uiTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;   // Preserve the 3D scene
            uiTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
            uiTargetInfo.resolve_texture = nullptr;
            uiTargetInfo.cycle = false;

            SDL_GPURenderPass* uiPass = SDL_BeginGPURenderPass(commandBuffer, &uiTargetInfo, 1, nullptr);
            if (uiPass) {
                ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer, uiPass);
                SDL_EndGPURenderPass(uiPass);
            }
        }
    }
    // Send the command buffer to the GPU for drawing
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    Input::Update();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto* appState = static_cast<AppState*>(appstate);

    FixedDelta(appState);
    HandleInput(appState);
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

    for (const auto &sampler: appState->samplers | std::views::values) {
        if (sampler) {
            SDL_ReleaseGPUSampler(appState->device, sampler);
        }
    }
    appState->samplers.clear();

    for (const auto &pipeline: appState->pipelines | std::views::values) {
        if (pipeline) {
            SDL_ReleaseGPUGraphicsPipeline(appState->device, pipeline);
        }
    }
    appState->pipelines.clear();

    for (const auto &shader: appState->shaders | std::views::values) {
        if (shader) {
            SDL_ReleaseGPUShader(appState->device, shader);
        }
    }
    appState->shaders.clear();

    if (appState->depthTexture) {SDL_ReleaseGPUTexture(appState->device, appState->depthTexture);}

    b2DestroyWorld(appState->worldId);

    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(appState->device);
    SDL_DestroyWindow(appState->window);
    delete appState;
}