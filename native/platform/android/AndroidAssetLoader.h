#pragma once

#include "engine/asset/AssetLoader.h"

struct AAssetManager;

namespace platform::android {

// Reads files packed into the APK's assets/ directory.
class AndroidAssetLoader final : public engine::AssetLoader {
public:
    explicit AndroidAssetLoader(AAssetManager* manager) : manager_(manager) {}

    std::vector<uint8_t> readFile(const std::string& path) override;

private:
    AAssetManager* manager_;
};

}  // namespace platform::android
