#include <filesystem>
#include <iostream>
#include <vector>
#include <SDL3/SDL.h>

#include "Mesh.h"
#include "Node.h"
#define SDL_MAIN_USE_CALLBACKS
#include <ranges>
#include <SDL3/SDL_main.h>

#include "Camera3D.h"
#include "FreeCam.h"
#include "AppState.h"
#include "Material.h"
#include "tiny_gltf.h"
#include "Vertex.h"

void UpdateAndUploadMVP(const AppState* appState, SDL_GPUCommandBuffer* cmdBuf) {
    const glm::mat4 model = appState->modelTransform.getMatrix();
    const glm::mat4 view = appState->current_camera->GetViewMatrix();
    const glm::mat4 projection = appState->current_camera->GetProjectionMatrix(appState->currentAspectRatio);

    glm::mat4 mvp = projection * view * model;

    SDL_PushGPUVertexUniformData(cmdBuf, 0, &mvp[0][0], sizeof(glm::mat4));
}

void handle_mouse_motion(const AppState* appState, const SDL_Event* event) {
    appState->current_camera->localTransform.rotate(glm::vec3(-event->motion.yrel * appState->sensitivity, -event->motion.xrel * appState->sensitivity, 0));
}

void HandleInput(const AppState* appState) {
    for (int i = 0; i < appState->nodes.size(); i++) {
        appState->nodes[i]->Input(*appState, SDL_GetKeyboardState(nullptr));
    }
}

void HandleUpdate(const AppState* appState) {
    for (int i = 0; i < appState->nodes.size(); i++) {
        appState->nodes[i]->Update(*appState);
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    auto* appState = new AppState();
    *appstate = appState;

    auto* freeCam = new FreeCam(*appState);
    appState->nodes.push_back(freeCam);

    appState->window = SDL_CreateWindow("FigmentEngine", appState->windowWidth, appState->windowHeight, 0);
    if (appState->window == nullptr)
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowRelativeMouseMode(appState->window, true);

    constexpr SDL_GPUShaderFormat formatFlags = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
    appState->device = SDL_CreateGPUDevice(formatFlags, true, nullptr);
    if (appState->device == nullptr)
    {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_ClaimWindowForGPUDevice(appState->device, appState->window))
    {
        SDL_Log("Couldn't claim window for GPU device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const SDL_GPUTextureCreateInfo depthInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = static_cast<Uint32>(appState->windowWidth),
        .height = static_cast<Uint32>(appState->windowHeight),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1
    };
    appState->depthTexture = SDL_CreateGPUTexture(appState->device, &depthInfo);

    // Load texture before the pipeline uses it
    if (!appState->CreateDefaultTextures()) {
        SDL_Log("Couldn't load default textures.");
        return SDL_APP_FAILURE;
    }
    appState->CreateDefaultMaterials();
    appState->CreateDefaultSamplers();
    appState->CreateDefaultPipelines();

    try {
        Mesh model = Mesh::LoadGLB(*appState, "zulu.glb");

        // Upload to GPU using the Mesh's own method
        model.UploadToGPU(*appState);

        // Store the mesh in appState (you'll need to add this member)
        appState->meshes.push_back(std::move(model));

    } catch (const std::exception& e) {
        SDL_Log("Failed to load model: %s", e.what());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto* appState = static_cast<AppState*>(appstate);

    switch (event->type)
    {
        case SDL_EVENT_MOUSE_MOTION:
            handle_mouse_motion(appState, event);
            return SDL_APP_CONTINUE;
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto* appState = static_cast<AppState*>(appstate);

    appState->lastTime = appState->currentTime;
    appState->currentTime = SDL_GetTicks();
    appState->delta = appState->currentTime - appState->lastTime;

    HandleInput(appState);
    HandleUpdate(appState);

    // Here's where we store the commands that will be eventually sent to the GPU
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(appState->device);
    if (commandBuffer == nullptr)
    {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // here we create the texture that will be drawn to the screen once it's done
    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appState->window, &swapchainTexture, nullptr, nullptr))
    {
        SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (swapchainTexture != nullptr) {
        // Here we create the first render pass, which just clears the screen
        const SDL_GPUColorTargetInfo colorTargetInfo = {
            .texture = swapchainTexture,
            .clear_color = SDL_FColor{0.4f, 0.6f, 0.9f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        const SDL_GPUDepthStencilTargetInfo depthTarget = {
            .texture = appState->depthTexture,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
            .stencil_load_op = SDL_GPU_LOADOP_CLEAR,
            .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
            .cycle = true
        };

        // now we actually define the render pass, by passing &colorTargetInfo, which modified our swapchainTexture to become cleared
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTarget);

        UpdateAndUploadMVP(appState, commandBuffer);

        // Draw all meshes
        for (const auto& mesh : appState->meshes) {
            if (!mesh.isOnGPU) continue;

            // Bind mesh's vertex buffer
            SDL_GPUBufferBinding vertexBinding = { .buffer = mesh.vertexBuffer, .offset = 0 };
            SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);

            // Bind mesh's index buffer if it has indices
            if (!mesh.indices.empty()) {
                SDL_GPUBufferBinding indexBinding = { .buffer = mesh.indexBuffer, .offset = 0 };
                SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
            }

            // Draw each submesh with its material
            for (const auto& submesh : mesh.submeshes) {
                if (appState->GetMaterial(submesh.material) != nullptr) {
                    // Bind the material's pipeline and textures
                    appState->GetMaterial(submesh.material)->Bind(appState);

                    // Draw the submesh
                    if (!mesh.indices.empty()) {
                        SDL_DrawGPUIndexedPrimitives(
                            renderPass,
                            submesh.indexCount,
                            1,
                            submesh.startIndex,
                            submesh.startVertex,
                            0
                        );
                    } else {
                        SDL_DrawGPUPrimitives(
                            renderPass,
                            submesh.vertexCount,
                            1,
                            submesh.startVertex,
                            0
                        );
                    }
                }
            }
        }

        SDL_EndGPURenderPass(renderPass);
    }
    // Send the command buffer to the GPU for drawing
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    const auto appState = static_cast<AppState*>(appstate);

    for (const Node* node : appState->nodes) delete node;
    appState->nodes.clear();
    appState->meshes.clear();

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

    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(appState->device);
    SDL_DestroyWindow(appState->window);
    delete appState;
}