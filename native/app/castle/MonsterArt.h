#pragma once

#include "engine/graphics/Sprite.h"

namespace app::castle::art {
inline constexpr char kAtlasPath[] = "characters/monsters-anime.png";
inline constexpr int kColumns = 3;
inline constexpr int kRows = 2;

// Original nonhuman monsters, ordered by the stable Species save IDs.
// Every cell includes transparent gutters to prevent filtering into neighbours.
inline engine::Sprite frame(const engine::Sprite& sheet, int species) {
    if (!sheet.isValid() || species < 0 || species >= kColumns * kRows) return {};
    engine::Sprite sprite = sheet;
    const float du = (sheet.u1 - sheet.u0) / kColumns;
    const float dv = (sheet.v1 - sheet.v0) / kRows;
    sprite.u0 = sheet.u0 + (species % kColumns) * du;
    sprite.v0 = sheet.v0 + (species / kColumns) * dv;
    sprite.u1 = sprite.u0 + du;
    sprite.v1 = sprite.v0 + dv;
    sprite.width = sheet.width / kColumns;
    sprite.height = sheet.height / kRows;
    return sprite;
}
}  // namespace app::castle::art
