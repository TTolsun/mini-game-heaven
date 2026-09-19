# 초상화 제작 기록

2026-09-20. 도구: 내장 imagegen. 새 이미지 한 장 생성, 참조 이미지 없음, 후처리 없음. 생성 원본을 그대로 app/src/main/assets/art/portraits.png로 복사했습니다. 실제 크기 1254×1254 RGBA가 아니라 원본 RGB PNG이며 런타임에서 RGBA로 읽습니다. 셀당 418×418입니다. 이미지 파일의 SHA-256은 아래에 기록합니다.

## 사용한 프롬프트

Create one professional illustrated portrait atlas for a retro martial arts tactical RPG about young Master Roshi before Dragon Ball. Output a single square 1536x1536 image arranged as exactly a 3 by 3 regular grid of equal 512x512 cells, no gutters, no borders, no writing, no text. Each cell is a separate centered bust portrait with identical dark warm ink-blue background and generous space around the whole head. Style: beautifully expressive hand-painted 1990s East Asian animation character portraits, confident dark ink contours, restrained warm cel shading, painterly paper grain, coherent amber/jade/indigo palette, readable faces. NOT pixel art, NOT a screenshot, NOT a sprite sheet. Row 1 left: youthful Roshi as a 12-year-old human boy, messy short black hair, open friendly courageous eyes, orange martial arts gi with muted teal collar, no glasses, no beard. Row 1 center: young Master Shen as a 13-year-old human boy, narrow sharp eyes, sleek dark hair tied high with a small topknot, ivory and dark teal martial arts clothes, confident competitive expression. Row 1 right: Master Mutaito, elderly but robust martial arts master, bald upper head, prominent white eyebrows and swept white mustache, deep purple gi, calm dignified gaze. Row 2 left: original young female healer Lanlan, dark bobbed hair with small jade ribbons, blue and cream robes, kind focused expression. Row 2 center: rugged adult bandit brawler, brown headband, squared jaw, earth-red vest, clenched grin. Row 2 right: stocky armored adult bandit, shaved head with dark stubble, dark teal lamellar vest, stern eyes. Row 3 left: robed enemy ki adept, crimson scarf, dark tied hair, cunning eyes. Row 3 center: agile enemy raider, straw hat tilted back, ochre tunic, angular face. Row 3 right: older Roshi at the end of his journey, bald head, thick white beard, round sunglasses, tropical orange shirt, compassionate smile. Keep every portrait entirely inside its own cell, matched scale chest-up, shoulders stopping at cell bottom, lighting from upper left. This is production portrait artwork, highly finished and charming.

## 인게임 원본

- 초상화: app/src/main/assets/art/portraits.png입니다.
- 픽셀 유닛·타일·도장: native/app/srpg/BattleArt.cpp의 팔레트와 부품 드로잉 코드입니다. 생성 이미지에서 잘라 오거나 AI 프레임을 보간하지 않습니다.
- 픽셀 VFX: native/app/srpg/BattleScene.cpp의 effects 함수입니다.
- 검증용 장면: native/tests/VisualFixtures.cpp입니다. 테스트용 에뮬레이터에서만 저장을 교체하며 사용자 실기기의 캠페인에는 주입하지 않습니다.

SHA-256: D3CB44C28E54DF11361AA7B8FD49DE5C089A8D0483966241AA2851DC82B8979C
