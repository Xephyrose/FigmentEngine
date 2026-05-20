#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Mesh.h"
#include <tiny_gltf.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>
#include <SDL3/SDL_log.h>

static std::vector<float> ReadAttributeData(const tinygltf::Model& model, int accessorIdx) {
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
        const float* floatData = reinterpret_cast<const float*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = floatData[i];
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const uint16_t* shortData = reinterpret_cast<const uint16_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(shortData[i]);
        }
    }
    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        const uint8_t* byteData = reinterpret_cast<const uint8_t*>(dataPtr);
        for (size_t i = 0; i < vertexCount * numComponents; i++) {
            result[i] = static_cast<float>(byteData[i]) / 255.0f;
        }
    }

    return result;
}

Mesh Mesh::LoadGLB(const std::string& filepath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Actually load the file
    bool success = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);

    // Check if there's warnings with the file / how TinyGLTF reads it
    if (!warn.empty()) {
        SDL_Log("GLTF Error: %s", warn.c_str());
    }
    if (!err.empty()) {
        SDL_Log("GLTF Warning: %s", err.c_str());
    }
    if (!success) {
        SDL_Log("Failed to load GLB file: %s", filepath.c_str());
    }

    Mesh result;

    // Process all meshes
    for (const auto& gltfMesh : model.meshes) {
        for (const auto& primitive : gltfMesh.primitives) {
            size_t startVertex = result.vertices.size();

            // Read POSITION attribute
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt != primitive.attributes.end()) {
                std::vector<float> positions;
                positions = ReadAttributeData(model, posIt->second);

                size_t vertexCount = positions.size() / 3;
                result.vertices.resize(startVertex + vertexCount);

                // Convert positions to Vector3
                for (size_t i = 0; i < vertexCount; i++) {
                    result.vertices[startVertex + i].position = Vector3(
                        positions[i * 3],
                        positions[i * 3 + 1],
                        positions[i * 3 + 2]
                    );
                    // Set UVs
                    result.vertices[startVertex + i].uv = Vector2(0.0f, 0.0f);
                }
            }

            // 2. Read TEXCOORD_0 attribute (UVs)
            auto uvIt = primitive.attributes.find("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                std::vector<float> uvs = ReadAttributeData(model, uvIt->second);

                size_t vertexCount = uvs.size() / 2;
                for (size_t i = 0; i < vertexCount && i < (result.vertices.size() - startVertex); i++) {
                    result.vertices[startVertex + i].uv = Vector2(uvs[i * 2], uvs[i * 2 + 1]);
                }
            }

            // 3. Read indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + indexAccessor.byteOffset;
                size_t indexCount = indexAccessor.count;

                result.indices.reserve(result.indices.size() + indexCount);

                // Handle different index types
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices16[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        // Check for overflow (uint16_t max = 65535)
                        if (indices32[i] + startVertex > 65535) {
                            throw std::runtime_error("Model has more than 65535 vertices. Consider using uint32_t for indices.");
                        }
                        result.indices.push_back(static_cast<uint16_t>(indices32[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    const uint8_t* indices8 = reinterpret_cast<const uint8_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices8[i] + startVertex));
                    }
                }
            }
            else {
                // No index buffer - generate sequential indices
                size_t vertexCount = result.vertices.size() - startVertex;
                for (size_t i = 0; i < vertexCount; i++) {
                    result.indices.push_back(static_cast<uint16_t>(startVertex + i));
                }
            }
        }
    }

    // SDL_Log("Mesh has %zu vertices", result.vertices.size());
    // for (size_t i = 0; i < std::min(result.vertices.size(), static_cast<size_t>(5)); i++) {
    //     SDL_Log("Vertex %zu: UV=(%f, %f)", i, result.vertices[i].uv.x, result.vertices[i].uv.y);
    // }

    return result;
}

Mesh Mesh::LoadGLBFromMemory(const std::vector<uint8_t>& data) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Load binary GLB from memory
    bool success = loader.LoadBinaryFromMemory(&model, &err, &warn, data.data(), data.size());

    if (!warn.empty()) {
        SDL_Log("GLTF Error: %s", warn.c_str());
    }
    if (!err.empty()) {
        SDL_Log("GLTF Warning: %s", err.c_str());
    }
    if (!success) {
        SDL_Log("Failed to load GLB from memory");
    }

    // Reuse the same processing logic
    Mesh result;

    for (const auto& gltfMesh : model.meshes) {
        for (const auto& primitive : gltfMesh.primitives) {
            size_t startVertex = result.vertices.size();

            // Read POSITION
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt != primitive.attributes.end()) {
                std::vector<float> positions;
                positions = ReadAttributeData(model, posIt->second);

                size_t vertexCount = positions.size() / 3;
                result.vertices.resize(startVertex + vertexCount);

                for (size_t i = 0; i < vertexCount; i++) {
                    result.vertices[startVertex + i].position = Vector3(
                        positions[i * 3],
                        positions[i * 3 + 1],
                        positions[i * 3 + 2]
                    );
                    result.vertices[startVertex + i].uv = Vector2(0.0f, 0.0f);
                }
            }

            // Read TEXCOORD_0
            auto uvIt = primitive.attributes.find("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                std::vector<float> uvs = ReadAttributeData(model, uvIt->second);

                size_t vertexCount = uvs.size() / 2;
                for (size_t i = 0; i < vertexCount && i < (result.vertices.size() - startVertex); i++) {
                    result.vertices[startVertex + i].uv = Vector2(uvs[i * 2], uvs[i * 2 + 1]);
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
                    const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices16[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        if (indices32[i] + startVertex > 65535) {
                            throw std::runtime_error("Model has more than 65535 vertices.");
                        }
                        result.indices.push_back(static_cast<uint16_t>(indices32[i] + startVertex));
                    }
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    const uint8_t* indices8 = reinterpret_cast<const uint8_t*>(dataPtr);
                    for (size_t i = 0; i < indexCount; i++) {
                        result.indices.push_back(static_cast<uint16_t>(indices8[i] + startVertex));
                    }
                }
            }
        }
    }

    return result;
}

Mesh Mesh::CreateQuad(const float width, const float height) {
    Mesh mesh;

    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    mesh.vertices = {
        Vertex(Vector3(-halfW,  halfH, 0.0f), Vector2(0.0f, 0.0f)),
        Vertex(Vector3( halfW,  halfH, 0.0f), Vector2(1.0f, 0.0f)),
        Vertex(Vector3( halfW, -halfH, 0.0f), Vector2(1.0f, 1.0f)),
        Vertex(Vector3(-halfW, -halfH, 0.0f), Vector2(0.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2, 0, 2, 3};

    return mesh;
}

Mesh Mesh::CreateTriangle(const float size) {
    Mesh mesh;

    float halfSize = size * 0.5f;

    mesh.vertices = {
        Vertex(Vector3(0.0f,  halfSize, 0.0f), Vector2(0.5f, 0.0f)),
        Vertex(Vector3(-halfSize, -halfSize, 0.0f), Vector2(0.0f, 1.0f)),
        Vertex(Vector3( halfSize, -halfSize, 0.0f), Vector2(1.0f, 1.0f))
    };

    mesh.indices = {0, 1, 2};

    return mesh;
}