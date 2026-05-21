#include <filesystem>
#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "Mesh.h"
#include "Node.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Camera3D.h"
#include "Vertex.h"

struct AppState
{
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

    Camera3D camera;
    Transform3D modelTransform;

    float currentAspectRatio = 720.0f / 720.0f;
};

void UpdateAndUploadMVP(const AppState* appState, SDL_GPUCommandBuffer* cmdBuf) {
    const glm::mat4 model = appState->modelTransform.GetModelMatrix();
    const glm::mat4 view = appState->camera.GetViewMatrix();
    const glm::mat4 projection = appState->camera.GetProjectionMatrix(appState->currentAspectRatio);

    glm::mat4 mvp = projection * view * model;

    SDL_PushGPUVertexUniformData(cmdBuf, 0, &mvp[0][0], sizeof(glm::mat4));
}

bool LoadTextureFromFile(AppState* appState, const std::string& texturePath) {
    if (appState->texture) {
        SDL_ReleaseGPUTexture(appState->device, appState->texture);
        appState->texture = nullptr;
    }
    if (appState->sampler) {
        SDL_ReleaseGPUSampler(appState->device, appState->sampler);
        appState->sampler = nullptr;
    }

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(appState->device);
    if (!uploadCmdBuf) {
        SDL_Log("Couldn't acquire command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    // Load image
    appState->texture = IMG_LoadGPUTexture(appState->device, copyPass, texturePath.c_str(), nullptr, nullptr);

    // End the copy pass
    SDL_EndGPUCopyPass(copyPass);
    // Submit the command buffer
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    if (!appState->texture) {
        return false;
    }

    SDL_GPUSamplerCreateInfo samplerInfo = {
        .min_filter = SDL_GPU_FILTER_LINEAR,
        .mag_filter = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
    };
    appState->sampler = SDL_CreateGPUSampler(appState->device, &samplerInfo);

    return true;
}

SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const std::string& shaderFilename) {
    SDL_GPUShaderStage stage;
    if (shaderFilename.contains(".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (shaderFilename.contains(".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Couldn't deduce shader stage from file name: %s", shaderFilename.c_str());
        return nullptr;
    }

    std::filesystem::path fullPath = std::filesystem::path(SDL_GetBasePath()) / "shaders";
    // Starts as invalid so we don't assume
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    // Different shaer formats have different entrypoint names
    const char* entrypoint;

    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        fullPath /= shaderFilename + ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL)
    {
        fullPath /= shaderFilename + ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    }
    else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        fullPath /= shaderFilename + ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else
    {
        SDL_Log("Couldn't find a supported shader format for backend %s!", SDL_GetGPUDeviceDriver(device));
        return nullptr;
    }

    // Store the size of the data we're loading, to be reused later
    size_t fileSize;
    void* code = SDL_LoadFile(fullPath.string().c_str(), &fileSize);
    if (code == nullptr)
    {
        SDL_Log("Couldn't load shader file from disk!\n\t%s", SDL_GetError());
        return nullptr;
    }

    const auto shaderInfo = SDL_GPUShaderCreateInfo{
        .code_size = fileSize,
        .code = static_cast<Uint8*>(code),
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,

        .num_samplers =
            stage == SDL_GPU_SHADERSTAGE_FRAGMENT ? 1u : 0u,

        .num_storage_textures = 0u,
        .num_storage_buffers = 0u,
        .num_uniform_buffers = stage == SDL_GPU_SHADERSTAGE_VERTEX ? 1u : 0u,
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == nullptr)
    {
        SDL_Log("Couldn't create shader from file %s: %s", fullPath.c_str(), SDL_GetError());
        SDL_free(code);
        return nullptr;
    }
    return shader;
}

bool CreatePipeline(AppState* appState) {
    SDL_GPUShader* vertexShader = LoadShader(appState->device, "Textured.vert");
    if (vertexShader == nullptr)
    {
        SDL_Log("Couldn't create vertex shader!");
        return false;
    }

    SDL_GPUShader* fragmentShader = LoadShader(appState->device, "Textured.frag");
    // SDL_GPUShader* fragmentShader = LoadShader(appState->device, "UVs.frag");
    if (fragmentShader == nullptr)
    {
        SDL_Log("Couldn't create fragment shader!");
        return false;
    }

    constexpr std::array vertexBufferDescriptions{
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        },
    };

    constexpr std::array vertexAttributes{
        SDL_GPUVertexAttribute{
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = offsetof(Vertex, position),
        },
        SDL_GPUVertexAttribute{
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = offsetof(Vertex, uv),
        }
    };

    const std::array colorTargetDescriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(appState->device, appState->window)
        }
    };

    const auto pipelineCreateInfo = SDL_GPUGraphicsPipelineCreateInfo{
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = SDL_GPUVertexInputState{
            .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
            .num_vertex_buffers = vertexBufferDescriptions.size(),
            .vertex_attributes = vertexAttributes.data(),
            .num_vertex_attributes = vertexAttributes.size(),
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = SDL_GPURasterizerState{
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .depth_stencil_state = SDL_GPUDepthStencilState{
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
        },
        .target_info = SDL_GPUGraphicsPipelineTargetInfo{
            .color_target_descriptions = colorTargetDescriptions.data(),
            .num_color_targets = colorTargetDescriptions.size(),
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
            .has_depth_stencil_target = true
        },
    };
    appState->pipeline = SDL_CreateGPUGraphicsPipeline(appState->device, &pipelineCreateInfo);
    if (appState->pipeline == nullptr)
    {
        SDL_Log("Couldn't create graphics pipeline! %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(appState->device, vertexShader);
    SDL_ReleaseGPUShader(appState->device, fragmentShader);
    return true;
}

bool CreateVertexBuffer(AppState* myAppState, const std::span<const Vertex> vertexes) {
    // Allocate memory for whatever number of vertexes we need
    myAppState->numVertexes = vertexes.size();
    const Uint32 vertexSize = myAppState->numVertexes * sizeof(Vertex);

    // Create buffer to store vertexes
    const auto vertexBufferCreateInfo = SDL_GPUBufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = vertexSize,
    };
    myAppState->vertexBuffer = SDL_CreateGPUBuffer(myAppState->device, &vertexBufferCreateInfo);
    if (myAppState->vertexBuffer == nullptr)
    {
        SDL_Log("Couldn't create vertex buffer: %s", SDL_GetError());
        return false;
    }

    // Create a buffer that will upload the data to our GPU
    const auto transferBufferCreateInfo = SDL_GPUTransferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = vertexSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(myAppState->device, &transferBufferCreateInfo);
    if (transferBuffer == nullptr)
    {
        SDL_Log("Couldn't create transfer buffer: %s", SDL_GetError());
        return false;
    }

    // Use SDL_MapGPUTransferBuffer to find the first area in our memory that is large enough to store this data
    auto* transferData = static_cast<Vertex*>(SDL_MapGPUTransferBuffer(myAppState->device, transferBuffer, false));
    if (transferData == nullptr)
    {
        SDL_Log("Couldn't map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(myAppState->device, transferBuffer);
        return false;
    }

    // Copy the vertexes into memory
    SDL_memcpy(transferData, vertexes.data(), vertexSize);
    // unmap the transfer buffer, as we've used it for its purpose
    SDL_UnmapGPUTransferBuffer(myAppState->device, transferBuffer);

    // Now, we need to use the transfer buffer to upload data to the vertex buffer.
    // I think this will allow us to store our vertexes in memory directly without needing to transfer data all wonky-like
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(myAppState->device);
    if (uploadCmdBuf == nullptr)
    {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    // Decide the source of our upload
    const auto bufferLocation = SDL_GPUTransferBufferLocation{
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };

    // Decide the destination of our upload
    const auto bufferRegion = SDL_GPUBufferRegion{
        .buffer = myAppState->vertexBuffer,
        .offset = 0,
        .size = vertexSize,
    };

    // Create the instruction that will upload our data
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);

    // End the copy pass, as it is done, and then submit the command
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf))
    {
        SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
        return false;
    }

    // Release the transfer buffer, as we're done with it too
    SDL_ReleaseGPUTransferBuffer(myAppState->device, transferBuffer);
    return true;
}

bool CreateIndexBuffer(AppState* myAppState, const std::span<const uint16_t> indices) {
    myAppState->numIndices = indices.size();
    const Uint32 indexSize = indices.size() * sizeof(uint16_t);

    const auto indexBufferCreateInfo = SDL_GPUBufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = indexSize,
    };
    myAppState->indexBuffer = SDL_CreateGPUBuffer(myAppState->device, &indexBufferCreateInfo);
    if (myAppState->indexBuffer == nullptr) {
        SDL_Log("Couldn't create index buffer: %s", SDL_GetError());
        return false;
    }

    // Create transfer buffer
    const auto transferBufferCreateInfo = SDL_GPUTransferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = indexSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(myAppState->device, &transferBufferCreateInfo);
    if (transferBuffer == nullptr) {
        SDL_Log("Couldn't create transfer buffer: %s", SDL_GetError());
        return false;
    }

    // Map and copy
    auto* transferData = static_cast<uint16_t*>(SDL_MapGPUTransferBuffer(myAppState->device, transferBuffer, false));
    if (transferData == nullptr) {
        SDL_Log("Couldn't map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(myAppState->device, transferBuffer);
        return false;
    }

    SDL_memcpy(transferData, indices.data(), indexSize);
    SDL_UnmapGPUTransferBuffer(myAppState->device, transferBuffer);

    // Upload to index buffer
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(myAppState->device);
    if (uploadCmdBuf == nullptr) {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    const auto bufferLocation = SDL_GPUTransferBufferLocation{
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };

    const auto bufferRegion = SDL_GPUBufferRegion{
        .buffer = myAppState->indexBuffer,
        .offset = 0,
        .size = indexSize,
    };

    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);

    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(myAppState->device, transferBuffer);
    return true;
}

bool LoadMeshToGPU(AppState* appState, const Mesh& mesh) {
    // Release old buffers if they exist
    if (appState->vertexBuffer) {
        SDL_ReleaseGPUBuffer(appState->device, appState->vertexBuffer);
    }
    if (appState->indexBuffer) {
        SDL_ReleaseGPUBuffer(appState->device, appState->indexBuffer);
    }

    // Create new buffers
    if (!CreateVertexBuffer(appState, mesh.vertices)) {
        return false;
    }

    if (!CreateIndexBuffer(appState, mesh.indices)) {
        return false;
    }

    return true;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    auto* appState = new AppState();
    *appstate = appState;

    appState->window = SDL_CreateWindow("FigmentEngine", 720, 720, 0);
    if (appState->window == nullptr)
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

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

    SDL_GPUTextureCreateInfo depthInfo = {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = 720,
        .height = 720,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1
    };
    appState->depthTexture = SDL_CreateGPUTexture(appState->device, &depthInfo);

    // Load texture before the pipeline uses it
    std::filesystem::path texturePath = std::filesystem::path(SDL_GetBasePath()) / "textures" / "dev.png";
    if (!LoadTextureFromFile(appState, texturePath.string())) {
        SDL_Log("Couldn't load texture.");
    }

    if (!appState->texture || !appState->sampler) {
        SDL_Log("Texture or sampler is null after loading!");
    }

    if (!CreatePipeline(appState))
    {
        return SDL_APP_FAILURE;
    }

    try {
        Mesh model = Mesh::LoadGLB(std::filesystem::path(SDL_GetBasePath()) / "models/placeholder.glb");

        if (!LoadMeshToGPU(appState, model)) {
            return SDL_APP_FAILURE;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        return SDL_APP_FAILURE;
    }

    appState->camera.transform.position = glm::vec3(0.0f, 0.0f, -1.5f);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:

            return SDL_APP_CONTINUE;
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        default:
            return SDL_APP_CONTINUE;
    }
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    AppState* appState = static_cast<AppState*>(appstate);

    //appState->camera.transform.position += glm::vec3(0.0f, 0.0f, -0.1f);
    appState->modelTransform.rotation *= glm::angleAxis(0.01f, glm::vec3(0.25f, 0.5f, 0.1f));

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

        // Define the pipeline we'll use
        SDL_BindGPUGraphicsPipeline(renderPass, appState->pipeline);

        // Define which vertex buffers we'll be using (in this case, just the one)
        const std::array vertexBuffers{
            SDL_GPUBufferBinding{
                .buffer = appState->vertexBuffer,
                .offset = 0,
            },
        };
        // Bind our vertex buffer to the render pass
        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers.data(), vertexBuffers.size());

        // Index, too
        const SDL_GPUBufferBinding indexBinding = {
            .buffer = appState->indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        UpdateAndUploadMVP(appState, commandBuffer);

        // Bind texture and sampler (if they exist)
        SDL_GPUTextureSamplerBinding textureBinding = {
            .texture = appState->texture,
            .sampler = appState->sampler
        };
        SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);

        // Draw stuff
        SDL_DrawGPUIndexedPrimitives(renderPass, appState->numIndices, 1, 0, 0, 0);

        // Finally, end the render pass
        SDL_EndGPURenderPass(renderPass);
    }
    // Send the command buffer to the GPU for drawing
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    const AppState* appState = static_cast<AppState*>(appstate);

    if (appState->sampler) SDL_ReleaseGPUSampler(appState->device, appState->sampler);
    if (appState->texture) SDL_ReleaseGPUTexture(appState->device, appState->texture);
    if (appState->depthTexture) SDL_ReleaseGPUTexture(appState->device, appState->depthTexture);
    if (appState->indexBuffer) SDL_ReleaseGPUBuffer(appState->device, appState->indexBuffer);
    if (appState->vertexBuffer) SDL_ReleaseGPUBuffer(appState->device, appState->vertexBuffer);
    if (appState->pipeline) SDL_ReleaseGPUGraphicsPipeline(appState->device, appState->pipeline);

    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(appState->device);
    SDL_DestroyWindow(appState->window);
    delete appState;
}