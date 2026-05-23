#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Mesh.h"
#include <tiny_gltf.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>

#include "Material.h"

static std::vector<float> ReadAttributeData(const tinygltf::Model& model, const int accessorIdx) {
    std::vector<float> result;

    if (accessorIdx < 0) return result;

    const tinygltf::Accessor& accessor = model.accessors[accessorIdx];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    size_t vertexCount = accessor.count;
    int numComponents = tinygltf::GetNumComponentsInType(accessor.type);

    result.resize(vertexCount * numComponents);

    // Handle different component types
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        const auto* floatData = reinterpret_cast<const float*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = floatData[i];
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const auto* shortData = reinterpret_cast<const uint16_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(shortData[i]);
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        const auto* byteData = reinterpret_cast<const uint8_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(byteData[i]) / 255.0f;
        }
    }

    return result;
}

void Mesh::UploadToGPU(const AppState& appState) {
    if (isOnGPU) return;

    // Create vertex buffer
    const SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };
    vertexBuffer = SDL_CreateGPUBuffer(appState.device, &vertexBufferInfo);

    // Create index buffer
    const SDL_GPUBufferCreateInfo indexBufferInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = static_cast<Uint32>(indices.size() * sizeof(uint16_t))
    };
    indexBuffer = SDL_CreateGPUBuffer(appState.device, &indexBufferInfo);

    // Upload data
    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(appState.device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);

    // Upload vertices
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };
    SDL_GPUTransferBuffer* vertexTransfer = SDL_CreateGPUTransferBuffer(appState.device, &transferBufferInfo);
    void* vertexData = SDL_MapGPUTransferBuffer(appState.device, vertexTransfer, false);
    memcpy(vertexData, vertices.data(), vertices.size() * sizeof(Vertex));
    SDL_UnmapGPUTransferBuffer(appState.device, vertexTransfer);

    SDL_GPUTransferBufferLocation transferLocation = {
        .transfer_buffer = vertexTransfer,
        .offset = 0,
    };

    SDL_GPUBufferRegion bufferRegion = {
        .buffer = vertexBuffer,
        .offset = 0,
        .size = static_cast<Uint32>(vertices.size() * sizeof(Vertex))
    };

    SDL_UploadToGPUBuffer(copyPass, &transferLocation, &bufferRegion, false);

    transferBufferInfo.size = static_cast<Uint32>(indices.size() * sizeof(uint16_t));

    // Upload indices similarly
    SDL_GPUTransferBuffer* indexTransfer = SDL_CreateGPUTransferBuffer(appState.device, &transferBufferInfo);
    void* indexData = SDL_MapGPUTransferBuffer(appState.device, indexTransfer, false);
    memcpy(indexData, indices.data(), indices.size() * sizeof(uint16_t));
    SDL_UnmapGPUTransferBuffer(appState.device, indexTransfer);

    SDL_GPUTransferBufferLocation indexTransferLocation = {
        .transfer_buffer = indexTransfer,
        .offset = 0,
    };

    SDL_GPUBufferRegion indexBufferRegion = {
        .buffer = indexBuffer,
        .offset = 0,
        .size = static_cast<Uint32>(indices.size() * sizeof(uint16_t))  // Cast to Uint32
    };

    SDL_UploadToGPUBuffer(copyPass, &indexTransferLocation, &indexBufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmd);

    // Cleanup transfer buffers
    SDL_ReleaseGPUTransferBuffer(appState.device, vertexTransfer);
    SDL_ReleaseGPUTransferBuffer(appState.device, indexTransfer);

    // Set GPU buffer pointers for submeshes
    for (auto& submesh : submeshes) {
        submesh.vertexBuffer = vertexBuffer;
        submesh.indexBuffer = indexBuffer;
    }

    isOnGPU = true;
}

void Mesh::ReleaseGPUResources(const AppState* appState) {
    if (appState && appState->device) {
        if (vertexBuffer) {
            SDL_ReleaseGPUBuffer(appState->device, vertexBuffer);
            vertexBuffer = nullptr;
        }
        if (indexBuffer) {
            SDL_ReleaseGPUBuffer(appState->device, indexBuffer);
            indexBuffer = nullptr;
        }
    }
    isOnGPU = false;
}

void Mesh::DrawSubmesh(AppState* appState, const Submesh &submesh) const {
    if (!isOnGPU) return;

    // Bind vertex buffer (same for all submeshes)
    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(appState->renderPass, 0, &vertexBinding, 1);

    // Bind index buffer if indices exist
    if (!indices.empty()) {
        SDL_GPUBufferBinding indexBinding = {
            .buffer = indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(appState->renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);  // Fixed: _16BIT
    }

    // Apply material (sets pipeline and textures)
    if (submesh.material.data()) {
        appState->GetMaterial(submesh.material)->Bind(appState);
    }

    // Draw
    if (!indices.empty()) {
        SDL_DrawGPUIndexedPrimitives(
            appState->renderPass,
            submesh.indexCount,
            1,
            submesh.startIndex,
            submesh.startVertex,
            0
        );
    } else {
        SDL_DrawGPUPrimitives(
            appState->renderPass,
            submesh.vertexCount,
            1,
            submesh.startVertex,
            0
        );
    }
}

void Mesh::DrawAllSubmeshes(AppState* appState) const {
    if (!isOnGPU) return;

    // Sort submeshes by material to minimize state changes
    std::vector<const Submesh*> sortedSubmeshes;
    for (const auto& submesh : submeshes) {
        sortedSubmeshes.push_back(&submesh);
    }
    std::ranges::sort(sortedSubmeshes,
                      [](const Submesh* a, const Submesh* b) {
                          return a->material < b->material;
                      });

    std::string currentMaterial;

    SDL_GPUBufferBinding vertexBinding = {
        .buffer = vertexBuffer,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(appState->renderPass, 0, &vertexBinding, 1);

    if (!indices.empty()) {
        SDL_GPUBufferBinding indexBinding = {
            .buffer = indexBuffer,
            .offset = 0
        };
        SDL_BindGPUIndexBuffer(appState->renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    }

    for (const auto* submesh : sortedSubmeshes) {
        if (submesh->material != currentMaterial) {
            if (!submesh->material.empty()) {
                if (const Material* material = appState->GetMaterial(submesh->material)) {
                    material->Bind(appState);
                }
            }
            currentMaterial = submesh->material;
        }

        if (!indices.empty()) {
            SDL_DrawGPUIndexedPrimitives(
                appState->renderPass,
                static_cast<Uint32>(submesh->indexCount),
                1,
                static_cast<Uint32>(submesh->startIndex),
                static_cast<Uint32>(submesh->startVertex),
                0
            );
        } else {
            SDL_DrawGPUPrimitives(
                appState->renderPass,
                static_cast<Uint32>(submesh->vertexCount),
                1,
                static_cast<Uint32>(submesh->startVertex),
                0
            );
        }
    }
}

const Submesh *Mesh::GetSubmesh(const std::string &name) const {
    for (const auto& submesh : submeshes) {
        if (submesh.name == name) {
            return &submesh;
        }
    }
    return nullptr;
}

Mesh Mesh::LoadGLB(const AppState& appState, std::string filepath) {
    filepath = std::filesystem::path(SDL_GetBasePath()) / "assets" / "models" / filepath;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool success = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

    if (!warn.empty()) {
        SDL_Log("GLTF Warning: %s", warn.c_str());
    }
    if (!err.empty()) {
        SDL_Log("GLTF Error: %s", err.c_str());
    }
    if (!success) {
        SDL_Log("Failed to load GLB file: %s", filepath.c_str());
        return {};
    }

    Mesh result;

    // NEW: Build node-to-mesh mapping
    std::vector<std::pair<int, std::string>> meshToNodeName; // meshIndex -> (nodeIndex, nodeName)

    for (const auto & node : model.nodes) {
        if (node.mesh >= 0) {
            meshToNodeName.emplace_back(node.mesh, node.name);
            // SDL_Log("Node[%zu]: name='%s' -> mesh=%d", nodeIdx, node.name.c_str(), node.mesh);
        }
    }

    // Process all meshes
    for (int meshIdx = 0; meshIdx < static_cast<int>(model.meshes.size()); meshIdx++) {
        const auto& gltfMesh = model.meshes[meshIdx];

        // Find which node(s) use this mesh
        std::string objectName = "Unnamed";
        for (const auto& mapping : meshToNodeName) {
            if (mapping.first == meshIdx) {
                objectName = mapping.second;
                break;
            }
        }

        // SDL_Log("Processing mesh %d: name='%s' (object: '%s')", meshIdx, gltfMesh.name.c_str(), objectName.c_str());

        for (const auto& primitive : gltfMesh.primitives) {
            Submesh submesh;
            submesh.name = objectName;
            submesh.meshName = gltfMesh.name;
            submesh.startVertex = result.vertices.size();
            submesh.startIndex = result.indices.size();

            size_t startVertex = result.vertices.size();

            // Read POSITION attribute
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt != primitive.attributes.end()) {
                std::vector<float> positions = ReadAttributeData(model, posIt->second);
                size_t vertexCount = positions.size() / 3;
                result.vertices.resize(startVertex + vertexCount);

                for (size_t i = 0; i < vertexCount; i++) {
                    result.vertices[startVertex + i].position = glm::vec3(
                        positions[i * 3],
                        positions[i * 3 + 1],
                        positions[i * 3 + 2]
                    );
                    result.vertices[startVertex + i].uv = glm::vec2(0.0f, 0.0f);
                }
                submesh.vertexCount = vertexCount;
            }

            // Read NORMAL attribute (add this for better lighting)
            auto normIt = primitive.attributes.find("NORMAL");
            if (normIt != primitive.attributes.end()) {
                std::vector<float> normals = ReadAttributeData(model, normIt->second);
                size_t vertexCount = normals.size() / 3;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    // You'll need to add a 'normal' member to your Vertex struct
                    // result.vertices[startVertex + i].normal = glm::vec3(
                    //     normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]
                    // );
                }
            }

            // Read TEXCOORD_0 attribute (UVs)
            auto uvIt = primitive.attributes.find("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                std::vector<float> uvs = ReadAttributeData(model, uvIt->second);
                size_t vertexCount = uvs.size() / 2;
                for (size_t i = 0; i < vertexCount && i < submesh.vertexCount; i++) {
                    result.vertices[startVertex + i].uv = glm::vec2(uvs[i * 2], uvs[i * 2 + 1]);
                }
            }

            // Read indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
                size_t indexCount = indexAccessor.count;

                result.indices.reserve(result.indices.size() + indexCount);

                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const auto* indices16 = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices16[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const auto* indices32 = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        if (indices32[i] + startVertex > 65535) {
                            throw std::runtime_error("Model has more than 65535 vertices. Consider using uint32_t for indices.");
                        }
                        result.indices.push_back(static_cast<uint16_t>(indices32[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    const auto* indices8 = reinterpret_cast<const uint8_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices8[i] + startVertex));
                    }
                }

                submesh.indexCount = indexCount;
            }

            submesh.material = model.materials[primitive.material].name;

            result.submeshes.push_back(submesh);
            // SDL_Log("  Added submesh '%s' - vertices: %zu, indices: %zu", submesh.name.c_str(), submesh.vertexCount, submesh.indexCount);
        }
    }

    // SDL_Log("=== Mesh Loading Complete ===");
    // SDL_Log("Total vertices: %zu", result.vertices.size());
    // SDL_Log("Total indices: %zu", result.indices.size());
    // SDL_Log("Submeshes found: %zu", result.submeshes.size());

    // for (const auto& submesh : result.submeshes) {
    //     SDL_Log("  - Object: '%s' (Mesh: '%s') vtx[%zu-%zu] idx[%zu-%zu]",
    //             submesh.name.c_str(),
    //             submesh.meshName.c_str(),
    //             submesh.startVertex,
    //             submesh.startVertex + submesh.vertexCount - 1,
    //             submesh.startIndex,
    //             submesh.startIndex + submesh.indexCount - 1);
    // }

    return result;
}

Mesh Mesh::CreateQuad(const float width, const float height) {
    Mesh mesh;

    const float halfW = width * 0.5f;
    const float halfH = height * 0.5f;

    mesh.vertices = {
        Vertex(glm::vec3(-halfW,  halfH, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3( halfW,  halfH, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3( halfW, -halfH, 0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec3(-halfW, -halfH, 0.0f), glm::vec2(0.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2, 0, 2, 3};

    return mesh;
}

Mesh Mesh::CreateTriangle(const float size) {
    Mesh mesh;

    float halfSize = size * 0.5f;

    mesh.vertices = {
        Vertex(glm::vec3(0.0f,  halfSize, 0.0f), glm::vec2(0.5f, 0.0f)),
        Vertex(glm::vec3(-halfSize, -halfSize, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3( halfSize, -halfSize, 0.0f), glm::vec2(1.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2};

    return mesh;
}