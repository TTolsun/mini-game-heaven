#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "engine/graphics/Sprite.h"
#include "engine/math/Color.h"
#include "engine/math/Vec2.h"

namespace engine {

class AssetLoader;
class SpriteBatch;
class TextureAtlas;

enum class TextAlign { Left, Center, Right };

// Bitmap font baked from a TTF with stb_truetype into the shared atlas.
// Glyphs are baked once at kBakePixelHeight and scaled when drawn, which is
// fine for the sizes a phone HUD uses.
class Font {
public:
    static constexpr float kBakePixelHeight = 72.0f;

    // Bakes ASCII plus the UTF-8 extraGlyphs into the shared atlas.
    bool load(AssetLoader& assets, TextureAtlas& atlas, const std::string& name,
              const std::string& ttfPath, std::string_view extraGlyphs = {});

    // `size` is the line height in world units.
    void draw(SpriteBatch& batch, std::string_view text, Vec2 position, float size,
              Color color = Color::white(), TextAlign align = TextAlign::Left) const;

    // Width in world units of `text` at `size`.
    float measure(std::string_view text, float size) const;

    bool isLoaded() const { return sprite_.isValid(); }

private:
    struct Glyph {
        float x0, y0, x1, y1;  // pixel rect inside the baked bitmap
        float xoff, yoff;      // offset from the pen position to the rect's top-left
        float xadvance;
    };

    const Glyph* glyph(int codepoint) const;
    std::vector<int> codepoints_;

    Sprite sprite_;  // the whole baked bitmap in the atlas
    int bitmapWidth_ = 0;
    int bitmapHeight_ = 0;
    float ascent_ = 0.0f;  // pixels above the baseline at bake size
    std::vector<Glyph> glyphs_;
};

}  // namespace engine
