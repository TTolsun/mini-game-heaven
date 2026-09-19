import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';

// Display the first atlas cell without modifying the original game artwork.
// Embed the sheet so the exported single-file SDD also works offline.
export function addMascot(html, png) {
  const marker = '<div id="sdd-mascot"></div>';
  if (html.split(marker).length !== 2) throw new Error('Expected exactly one SDD mascot placeholder');
  const image = png.toString('base64');
  return html.replace(marker, `<figure aria-label="대표 몬스터: 물방울 슬라임" style="margin:12px 0 24px;width:240px;max-width:100%">
<div style="position:relative;width:100%;aspect-ratio:1;overflow:hidden">
<img src="data:image/png;base64,${image}" alt="청록색 몸과 금빛 눈을 가진 물방울 슬라임" width="1536" height="1024" style="position:absolute;left:0;top:0;width:300%;height:200%;max-width:none">
</div><figcaption style="text-align:center;color:var(--muted);font-size:13px">마물 정원 · 물방울 슬라임</figcaption></figure>`);
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  try {
    const [output] = process.argv.slice(2);
    if (!output) throw new Error('Usage: docs-mascot.mjs HTML_PATH');
    const atlas = new URL('../app/src/main/assets/characters/monsters-anime.png', import.meta.url);
    fs.writeFileSync(output, addMascot(fs.readFileSync(output, 'utf8'), fs.readFileSync(atlas)));
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
