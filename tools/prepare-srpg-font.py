"""Build a readable, compact Korean font and its glyph inventory from game sources.
Usage: uv run --with fonttools python tools/prepare-srpg-font.py <variable.ttf>
The upstream font must be the OFL-licensed Noto Sans KR from google/fonts.
"""
from pathlib import Path
import sys
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont
from fontTools import subset
root = Path(__file__).resolve().parent.parent
assets = root/'app/src/main/assets/fonts'
all_chars = set()
for module in ('srpg',):
    text = ''.join(p.read_text(encoding='utf-8-sig') for p in (root/'native/app'/module).glob('*') if p.suffix in ('.cpp', '.h'))
    chars = ''.join(sorted({c for c in text if ord(c) > 127 and c != '\ufeff'}))
    (assets/f'{module}-glyphs.txt').write_text(chars, encoding='utf-8')
    all_chars.update(chars)
chars = ''.join(sorted(all_chars))
font = TTFont(sys.argv[1])
font = instantiateVariableFont(font, {'wght': 600}, inplace=True)
options = subset.Options()
subsetter = subset.Subsetter(options=options)
subsetter.populate(text=chars+''.join(chr(c) for c in range(32,127)))
subsetter.subset(font)
font.save(assets/'NotoSansKR.ttf')
print(f'Prepared {len(chars)} extra glyphs; {(assets/"NotoSansKR.ttf").stat().st_size} bytes')
