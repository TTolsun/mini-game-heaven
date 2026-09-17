#define LOG_TAG "Image"
#include "engine/asset/Image.h"

#include "engine/core/Log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#include "third_party/stb/stb_image.h"

namespace engine {

Image Image::load(AssetLoader& loader, const std::string& path) {
    Image image;

    const std::vector<uint8_t> bytes = loader.readFile(path);
    if (bytes.empty()) {
        LOGE("asset not found: %s", path.c_str());
        return image;
    }

    int channels = 0;
    stbi_uc* decoded = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()),
                                             &image.width, &image.height, &channels, 4);
    if (decoded == nullptr) {
        LOGE("decode failed: %s (%s)", path.c_str(), stbi_failure_reason());
        image.width = image.height = 0;
        return image;
    }

    const size_t byteCount = static_cast<size_t>(image.width) * image.height * 4;
    image.pixels.assign(decoded, decoded + byteCount);
    stbi_image_free(decoded);
    return image;
}

}  // namespace engine
