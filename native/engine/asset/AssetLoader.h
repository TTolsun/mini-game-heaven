#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace engine {

// Platform-neutral file access. The Android implementation reads from the
// APK's AAssetManager; a desktop build could read from disk.
class AssetLoader {
public:
    virtual ~AssetLoader() = default;

    // Returns the whole file, or an empty vector if it does not exist.
    virtual std::vector<uint8_t> readFile(const std::string& path) = 0;
};

}  // namespace engine
