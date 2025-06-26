#include "wrapper.h"

#include "logging.h"
#include "core.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/imagedecoder.h>

std::string get_text_from_file(std::string path) {
    Renderer& renderer = ecs.get_system<Renderer>();
    AAsset* asset = AAssetManager_open(renderer.app->activity->assetManager, path.c_str(), 0);

    const void* a = AAsset_getBuffer(asset);
    uint32_t length = AAsset_getLength(asset);

    if(a == nullptr) LOGD("failed to open file: %s", path.c_str());

    std::vector<uint8_t> bytes;
    bytes.resize(length);
    memcpy(bytes.data(), a, length);
    std::string s(bytes.begin(), bytes.end());

    AAsset_close(asset);

    return s;
}

std::vector<uint8_t> get_texture(std::string path, ivec2& size) {
    Renderer& renderer = ecs.get_system<Renderer>();
    const char* file = path.c_str();
    AAsset* asset = AAssetManager_open(renderer.app->activity->assetManager, file, AASSET_MODE_STREAMING);
    AImageDecoder* decoder;
    int result = AImageDecoder_createFromAAsset(asset, &decoder);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        LOGD("ERROR: Texture failed to load. %s", path.c_str());
        return {};
    }

    const AImageDecoderHeaderInfo* info = AImageDecoder_getHeaderInfo(decoder);
    size.x = AImageDecoderHeaderInfo_getWidth(info);
    size.y = AImageDecoderHeaderInfo_getHeight(info);
    size_t stride = AImageDecoder_getMinimumStride(decoder);
    size_t bytes_size = size.y * stride;

    std::vector<uint8_t> data(bytes_size);

    result = AImageDecoder_decodeImage(decoder, data.data(), stride, bytes_size);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        LOGD("ERROR: Texture failed to load. %s", path.c_str());
        return {};
    }

    AImageDecoder_delete(decoder);
    AAsset_close(asset);

    return data;
}


Shader::Shader(std::string vspath, std::string fspath) {
    from_source(vspath, fspath);
}


void Shader::from_source(std::string vspath, std::string fspath) {
    std::string vertex_shader_src = get_text_from_file(vspath);
    std::string fragment_shader_src = get_text_from_file(fspath);
    const char* v_ptr = vertex_shader_src.data();
    const char* f_ptr = fragment_shader_src.data();

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &v_ptr, 0);
    glCompileShader(vertex_shader);


    // check if vertex shader compiled correctly
    GLint compile_check;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_check);
    if(compile_check == GL_FALSE) {
        int log_size = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<char> error_log(log_size);
        glGetShaderInfoLog(vertex_shader, log_size, &log_size, &error_log[0]);
        LOGD("ERROR: Vertex shader failed to compile: %s", error_log.data());

        return;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &f_ptr, 0);
    glCompileShader(fragment_shader);

    // check if fragment shader compiled correctly
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_check);
    if(compile_check == GL_FALSE) {
        int log_size = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<char> error_log(log_size);
        glGetShaderInfoLog(fragment_shader, log_size, &log_size, &error_log[0]);
        LOGD("ERROR: Fragment shader failed to compile: %s", error_log.data());

        glDeleteShader(vertex_shader);
        return;
    }

    // attach and link vertex/fragment shaders
    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &compile_check);
    if(compile_check == GL_FALSE) {
        int log_size = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<char> error_log(log_size);
        glGetProgramInfoLog(id, log_size, &log_size, &error_log[0]);
        LOGD("ERROR: Program failed to link: %s", error_log.data());

        glDeleteShader(id);
        return;
    }

    // delete vertex/fragment shaders after compilation
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::use() {
    glUseProgram(id);
}

void Vertices::init() {
    glGenBuffers(1, &vertex_buffer);
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    initialized = true;
}

void Vertices::vertex_buffer_data(void* ptr, uint32_t num_vertices_, uint32_t vertex_size, uint32_t usage) {
    bind();
    num_vertices = num_vertices_;
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * vertex_size, ptr, usage);
}

void Vertices::index_buffer_data(void* ptr, uint32_t num_indices_, GLenum index_type_, uint32_t index_size, uint32_t usage) {
    bind();
    num_indices = num_indices_;
    index_type = index_type_;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * index_size, ptr, usage);
}

void Vertices::add_vertex_attribute(uint32_t index, uint32_t size, GLenum type, GLenum normalized, uint32_t stride, uint32_t offset) {
    glBindVertexArray(vertex_array);
    if(type == GL_INT || type == GL_UNSIGNED_INT) glVertexAttribIPointer(index, size, type, stride, (void*)offset);
    else glVertexAttribPointer(index, size, type, normalized, stride, (void*)offset);
    glEnableVertexAttribArray(index);
}

void Vertices::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindVertexArray(vertex_array);
}

void Vertices::draw_vertices(GLenum mode) {
    bind();
    glDrawArrays(mode, 0, num_vertices);
}

void Vertices::draw_indices(GLenum mode) {
    bind();
    glDrawElements(mode, num_indices, index_type, (void*)0);
}

Vertices::Vertices(Vertices&& a) {
    vertex_buffer = a.vertex_buffer;
    a.vertex_buffer = 0;

    vertex_array = a.vertex_array;
    a.vertex_array = 0;

    index_buffer = a.index_buffer;
    a.index_buffer = 0;

    num_indices = a.num_indices;
    a.num_indices = 0;

    num_vertices = a.num_vertices;
    a.num_vertices = 0;

    initialized = a.initialized;
    a.initialized = false;

    index_type = a.index_type;
}

Vertices& Vertices::operator=(Vertices&& a) {
    vertex_buffer = a.vertex_buffer;
    a.vertex_buffer = 0;

    vertex_array = a.vertex_array;
    a.vertex_array = 0;

    index_buffer = a.index_buffer;
    a.index_buffer = 0;

    num_indices = a.num_indices;
    a.num_indices = 0;

    num_vertices = a.num_vertices;
    a.num_vertices = 0;

    initialized = a.initialized;
    a.initialized = false;

    index_type = a.index_type;

    return *this;
}

Vertices::~Vertices() {
    if(initialized) {
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(1, &index_buffer);
        glDeleteBuffers(1, &vertex_array);
    }
}

Texture::Texture(std::string path, Format format, int mip_levels) {
    Renderer& renderer = ecs.get_system<Renderer>();

    type = GL_TEXTURE_2D;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    this->format = format;

    const char* file = path.c_str();
    AAsset* asset = AAssetManager_open(renderer.app->activity->assetManager, file, AASSET_MODE_STREAMING);
    AImageDecoder* decoder;
    int result = AImageDecoder_createFromAAsset(asset, &decoder);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        LOGD("ERROR: Texture failed to load. %s", path.c_str());
        return;
    }

    const AImageDecoderHeaderInfo* info = AImageDecoder_getHeaderInfo(decoder);
    size.x = AImageDecoderHeaderInfo_getWidth(info);
    size.y = AImageDecoderHeaderInfo_getHeight(info);
    size_t stride = AImageDecoder_getMinimumStride(decoder);
    size_t bytes_size = size.y * stride;

    std::vector<uint8_t> data(bytes_size);

    result = AImageDecoder_decodeImage(decoder, data.data(), stride, bytes_size);
    if (result != ANDROID_IMAGE_DECODER_SUCCESS) {
        LOGD("ERROR: Texture failed to load. %s", path.c_str());
        return;
    }


    AImageDecoder_delete(decoder);
    AAsset_close(asset);


    std::vector<uint8_t> flip(bytes_size);
    for(int y = 0; y < size.y; ++y) {
        uint32_t flipped_y = size.y - y - 1;

        for(int x = 0; x < stride; ++x) {
            size_t flipped_i = flipped_y * stride + x;
            size_t i = y * stride + x;

            flip[i] = data[flipped_i];
        }
    }


    glTexStorage2D(GL_TEXTURE_2D, mip_levels + 1, format.format_bits, size.x, size.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format.format, format.bits, flip.data());


    if(mip_levels > 0) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, mip_levels);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return;
}

bool Texture::load(std::string path, Format format, int mip_levels) {
    type = GL_TEXTURE_2D;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    this->format = format;

    ivec2 size;
    std::vector<uint8_t> data = get_texture(path, size);
    float stride = data.size() / size.y;

    std::vector<uint8_t> flip(data.size());
    for(int y = 0; y < size.y; ++y) {
        uint32_t flipped_y = size.y - y - 1;

        for(int x = 0; x < stride; ++x) {
            size_t flipped_i = flipped_y * stride + x;
            size_t i = y * stride + x;

            flip[i] = data[flipped_i];
        }
    }

    glTexStorage2D(GL_TEXTURE_2D, mip_levels + 1, format.format_bits, size.x, size.y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format.format, format.bits, flip.data());


    if(mip_levels > 0) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, mip_levels);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    return true;
}

bool Texture::load(glm::uvec2 size, Format format) {
    type = GL_TEXTURE_2D;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    this->format = format;

    glTexStorage2D(GL_TEXTURE_2D, 1, format.format_bits, size.x, size.y);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return true;
}

bool Texture::load(glm::uvec3 size, Format format) {
    type = GL_TEXTURE_3D;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);
    this->format = format;

    glTexStorage3D(GL_TEXTURE_3D, 1, format.format_bits, size.x, size.y, size.z);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return true;
}

Texture::Texture(uint8_t* data, glm::uvec3 size_, GLenum type_, Format format) {
    type = type_;
    size = size_;
    this->format = format;

    glGenTextures(1, &id);
    glBindTexture(type, id);

    if(!data) {
        LOGD("ERROR: Texture failed to load. (raw data texture)");
        return;
    } else {

        if(this->type == GL_TEXTURE_2D) {
            glTexStorage2D(GL_TEXTURE_2D, 1, format.format_bits, size.x, size.y);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, format.format, format.bits, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else if(this->type == GL_TEXTURE_3D) {
            glTexStorage3D(GL_TEXTURE_3D, 1, format.format_bits, size.x, size.y, size.z);
            glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, size.x, size.y, size.z, format.format, format.bits, data);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        }

        return;
    }
}

Texture::Texture(glm::uvec3 size, GLenum type, Format format) {
    this->type = type;
    this->size = size;
    this->format = format;

    glGenTextures(1, &id);
    glBindTexture(type, id);

    if(type == GL_TEXTURE_2D) {
        glTexStorage2D(GL_TEXTURE_2D, 1, format.format_bits, size.x, size.y);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else if(type == GL_TEXTURE_3D) {
        glTexStorage3D(GL_TEXTURE_3D, 1, format.format_bits, size.x, size.y, size.z);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    }
}

void Texture::bind(int binding) {
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(type, id);
}

void Texture::bind() {
    glBindTexture(type, id);
}

Texture::Texture(Texture&& t) noexcept {
    id = t.id;
    size = t.size;
    num_channels = t.num_channels;
    format = t.format;
    t.id = 0;
    type = t.type;
}

Texture& Texture::operator=(Texture&& t) noexcept {
    id = t.id;
    size = t.size;
    num_channels = t.num_channels;
    format = t.format;
    t.id = 0;
    type = t.type;

    return *this;
}

void Texture::delete_texture() {
    glDeleteTextures(1, &id);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}
