# Asset credits

- Noto Sans KR by the Noto project: https://github.com/google/fonts/tree/main/ofl/notosanskr
- License: SIL Open Font License 1.1. Full text: fonts/OFL-NotoSansKR.txt.
- The bundled subset uses weight 600 and fonts/srpg-glyphs.txt. Rebuild with tools/prepare-srpg-font.py.
- art/portraits.png: newly generated with the built-in imagegen tool on 2026-09-20. The 3×3 atlas is 1254×1254 pixels. Exact prompt and SHA-256: docs/art/visual-slice-prompts.md.
- Modular pixel characters, tiles and dojo are authored in native/app/srpg/BattleArt.cpp. Runtime generation uses fixed palettes and shared body parts, not generated animation sheets.
- UI and pixel VFX are drawn in native/app/srpg/BattleScene.cpp. Launcher geometry uses Android vector resources.
- Sound effects are synthesized by native/app/Sfx.cpp. No music or audio extracted from Dragon Ball is bundled.
- Dragon Ball characters are fan-game references. Original setting facts and game inventions are separated in docs/srpg-lore.md. The portrait atlas is newly generated artwork, not an asset extracted from the anime or manga.
