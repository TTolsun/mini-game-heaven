# Asset credits

- Noto Sans KR by the Noto project: https://github.com/google/fonts/tree/main/ofl/notosanskr
- License: SIL Open Font License 1.1. Full text: `fonts/OFL-NotoSansKR.txt`.
- The bundled subset uses weight 600 and the glyph inventory in `fonts/srpg-glyphs.txt`. Rebuild with `tools/prepare-srpg-font.py`.
- Prototype battlefield, figures and UI are drawn procedurally in `native/app/srpg/BattleScene.cpp`; launcher geometry is defined in Android vector resources.
- Sound effects are synthesized by `native/app/Sfx.cpp`.
- No character illustrations, music or audio extracted from Dragon Ball are bundled. Story and character references are documented in `docs/srpg-lore.md`.
