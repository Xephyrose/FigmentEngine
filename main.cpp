#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>
#include "Node.h"
#include "Vector3.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

struct AppState
{
    SDL_Window* window = nullptr;
    SDL_GPUDevice* device = nullptr;

    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    Uint32 numVertexes = 0;
    SDL_GPUBuffer* vertexBuffer = nullptr;
};

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
    SDL_GPUShader* vertexShader = LoadShader(appState->device, "OnlyPosition.vert");
    if (vertexShader == nullptr)
    {
        SDL_Log("Couldn't create vertex shader!");
        return false;
    }

    SDL_GPUShader* fragmentShader = LoadShader(appState->device, "SolidColor.frag");
    if (fragmentShader == nullptr)
    {
        SDL_Log("Couldn't create fragment shader!");
        return false;
    }

    std::array vertexBufferDescriptions{
        SDL_GPUVertexBufferDescription{
            .slot = 0,
            .pitch = sizeof(Vector3),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        },
    };

    std::array vertexAttributes{
        SDL_GPUVertexAttribute{
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
            .offset = 0 * sizeof(float),
        },
        SDL_GPUVertexAttribute{
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
            .offset = 1 * sizeof(float),
        },
        SDL_GPUVertexAttribute{
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
            .offset = 2 * sizeof(float),
        },
    };

    std::array colorTargetDescriptions{
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(appState->device, appState->window)
        }
    };

    auto pipelineCreateInfo = SDL_GPUGraphicsPipelineCreateInfo{
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
        .target_info = SDL_GPUGraphicsPipelineTargetInfo{
            .color_target_descriptions = colorTargetDescriptions.data(),
            .num_color_targets = colorTargetDescriptions.size(),
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

bool CreateVertexBuffer(AppState* myAppState, std::span<Vector3> vertexes) {
    // Allocate memory for whatever number of vertexes we need
    myAppState->numVertexes = vertexes.size();
    Uint32 vertexSize = myAppState->numVertexes * sizeof(Vector3);

    // Create buffer to store vertexes
    auto vertexBufferCreateInfo = SDL_GPUBufferCreateInfo{
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
    auto transferBufferCreateInfo = SDL_GPUTransferBufferCreateInfo{
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
    auto* transferData = static_cast<Vector3*>(SDL_MapGPUTransferBuffer(myAppState->device, transferBuffer, false));
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
    auto bufferLocation = SDL_GPUTransferBufferLocation{
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };

    // Decide the destination of our upload
    auto bufferRegion = SDL_GPUBufferRegion{
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

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_Log("Init");

    auto* appState = new AppState();
    *appstate = appState;

    appState->window = SDL_CreateWindow("Hello, SDL GPU!", 1280, 720, 0);
    if (appState->window == nullptr)
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUShaderFormat formatFlags = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
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

    if (!CreatePipeline(appState))
    {
        return SDL_APP_FAILURE;
    }

    // Must be counter-clockwise to face forward
    std::array vertexes{
        Vector3{-0.7f, -0.7f, 0.0f}, // Bottom-Left
        Vector3{0.7f, -0.7f, 0.0f}, // Bottom-Right
        Vector3{0.0f, 0.7f, 0.0f}, // Top-Middle
    };

    if (!CreateVertexBuffer(appState, vertexes))
    {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    SDL_Log("Event");
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
    SDL_Log("Iterate");
    const AppState* appState = static_cast<AppState*>(appstate);

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
        SDL_GPUColorTargetInfo colorTargetInfo = {
            .texture = swapchainTexture,
            .clear_color = SDL_FColor{0.4f, 0.6f, 0.9f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        // now we actually define the render pass, by passing &colorTargetInfo, which modified our swapchainTexture to become cleared
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);

        // Define the pipeline we'll use
        SDL_BindGPUGraphicsPipeline(renderPass, appState->pipeline);

        // Define which vertex buffers we'll be using (in this case, just the one)
        std::array vertexBuffers{
            SDL_GPUBufferBinding{
                .buffer = appState->vertexBuffer,
                .offset = 0,
            },
        };
        // Bind our vertex buffer to the render pass
        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers.data(), vertexBuffers.size());

        // Draw stuff
        SDL_DrawGPUPrimitives(renderPass, appState->numVertexes, 1, 0, 0);

        // Finally, end the render pass
        SDL_EndGPURenderPass(renderPass);
    }
    // Send the command buffer to the GPU for drawing
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    SDL_Log("Quit");
    const AppState* appState = static_cast<AppState*>(appstate);

    SDL_ReleaseWindowFromGPUDevice(appState->device, appState->window);
    // hehehe kill rog astral 5090 with hammers
    SDL_DestroyGPUDevice(appState->device);
    SDL_DestroyWindow(appState->window);
    delete appState;
}