#define LOG_TAG "AndroidAssetLoader"
#include "platform/android/AndroidAssetLoader.h"

#include <android/asset_manager.h>

#include "engine/core/Log.h"

namespace platform::android {

std::vector<uint8_t> AndroidAssetLoader::readFile(const std::string& path) {
    std::vector<uint8_t> bytes;

    AAsset* asset = AAssetManager_open(manager_, path.c_str(), AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        LOGW("missing asset: %s", path.c_str());
        return bytes;
    }

    const off_t length = AAsset_getLength(asset);
    bytes.resize(static_cast<size_t>(length));
    const int read = AAsset_read(asset, bytes.data(), bytes.size());
    AAsset_close(asset);

    if (read < 0 || static_cast<size_t>(read) != bytes.size()) {
        LOGE("short read: %s (%d of %ld)", path.c_str(), read, static_cast<long>(length));
        bytes.clear();
    }
    return bytes;
}

}  // namespace platform::android
