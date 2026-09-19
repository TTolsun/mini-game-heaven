cmake_minimum_required(VERSION 3.22)
project(wuten LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GameActivity (static lib + native_app_glue) comes from the AAR via Prefab.
find_package(game-activity REQUIRED CONFIG)

add_library(${PROJECT_NAME} SHARED
    # engine: platform-independent core
    engine/Engine.cpp
    engine/asset/Image.cpp
    engine/audio/Mixer.cpp
    engine/audio/Synth.cpp
    engine/graphics/Animation.cpp
    engine/graphics/Font.cpp
    engine/graphics/Particles.cpp
    engine/graphics/Shader.cpp
    engine/graphics/SpriteBatch.cpp
    engine/graphics/Texture.cpp
    engine/graphics/TextureAtlas.cpp

    # app: martial-arts SRPG learning prototype
    app/Sfx.cpp
    app/srpg/BattleModel.cpp
    app/srpg/BattleScene.cpp

    # platform/android: the only Android-aware code
    platform/android/AndroidAssetLoader.cpp
    platform/android/AndroidAudio.cpp
    platform/android/AndroidHaptics.cpp
    platform/android/AndroidMain.cpp
    platform/android/GlContext.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# stb headers are third-party; keep our warning flags off them.
set_source_files_properties(engine/asset/Image.cpp engine/graphics/Font.cpp
    PROPERTIES COMPILE_OPTIONS "-Wno-unused-function")

# The static lib defines JNI entry points that nothing references directly,
# so force the linker to keep them.
target_link_options(${PROJECT_NAME} PRIVATE
    -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    game-activity::game-activity_static
    android
    EGL
    GLESv3
    aaudio
    log
)
