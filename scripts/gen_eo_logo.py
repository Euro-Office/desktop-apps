#!/usr/bin/env python3
"""Generate Qt-friendly path-only Euro-Office title-bar logos."""

from __future__ import annotations

import sys
from pathlib import Path

from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen


def find_font() -> str:
    candidates = [
        # nix noto
        *Path("/nix/store").glob("*/share/fonts/noto/NotoSans.ttf"),
        *Path("/run/current-system/sw/share/fonts").glob("**/NotoSans.ttf"),
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    ]
    for c in candidates:
        p = Path(c)
        if p.is_file():
            return str(p)
    raise SystemExit("No suitable sans font found")


def doc_icon_path(x: float, y: float, size: float) -> str:
    """Simple document mark (fold + lines), path-only."""
    k = size / 24.0

    def X(v: float) -> float:
        return x + v * k

    def Y(v: float) -> float:
        return y + v * k

    r = 2 * k
    return (
        f"M{X(6):.3f},{Y(2):.3f} "
        f"H{X(14):.3f} "
        f"L{X(20):.3f},{Y(8):.3f} "
        f"V{Y(20):.3f} "
        f"A{r:.3f},{r:.3f} 0 0 1 {X(18):.3f},{Y(22):.3f} "
        f"H{X(6):.3f} "
        f"A{r:.3f},{r:.3f} 0 0 1 {X(4):.3f},{Y(20):.3f} "
        f"V{Y(4):.3f} "
        f"A{r:.3f},{r:.3f} 0 0 1 {X(6):.3f},{Y(2):.3f} Z "
        f"M{X(14):.3f},{Y(2):.3f} V{Y(8):.3f} H{X(20):.3f} "
        f"M{X(8):.3f},{Y(13):.3f} H{X(16):.3f} "
        f"M{X(8):.3f},{Y(17):.3f} H{X(14):.3f}"
    )


def wordmark_path(font_path: str, text: str, text_h: float = 12.0) -> tuple[str, float]:
    font = TTFont(font_path)
    glyph_set = font.getGlyphSet()
    cmap = font.getBestCmap()
    upem = font["head"].unitsPerEm
    scale = text_h / upem
    baseline = 14.5

    x = 0.0
    chunks: list[str] = []
    for ch in text:
        gname = cmap.get(ord(ch))
        if not gname:
            raise SystemExit(f"missing glyph for {ch!r}")
        glyph = glyph_set[gname]
        pen = SVGPathPen(glyph_set)
        tpen = TransformPen(pen, (scale, 0, 0, -scale, x, baseline))
        glyph.draw(tpen)
        d = pen.getCommands()
        if d:
            chunks.append(d)
        x += glyph.width * scale
    return " ".join(chunks), x


def write_logo(out: Path, fill: str, font_path: str) -> None:
    icon_size = 14.0
    icon_x = 1.0
    icon_y = (20.0 - icon_size) / 2.0
    gap = 4.0
    word, word_w = wordmark_path(font_path, "Euro-Office")
    text_x = icon_x + icon_size + gap
    total_w = int(text_x + word_w + 2.0 + 0.5)
    icon_d = doc_icon_path(icon_x, icon_y, icon_size)

    svg = f"""<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="{total_w}" height="20" viewBox="0 0 {total_w} 20" fill="none">
  <!-- Path-only mark: Qt QSvgRenderer does not reliably draw nested <svg> or <text>. -->
  <path fill="{fill}" fill-rule="evenodd" d="{icon_d}"/>
  <g transform="translate({text_x:.3f},0)">
    <path fill="{fill}" d="{word}"/>
  </g>
</svg>
"""
    out.write_text(svg)
    print(f"wrote {out} ({total_w}x20)")


def main() -> None:
    out_dir = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    out_dir.mkdir(parents=True, exist_ok=True)
    font_path = find_font()
    print("font:", font_path)
    # light chrome → dark ink; dark chrome → light ink
    write_logo(out_dir / "logo-light-eo.svg", "#3D4A5C", font_path)
    write_logo(out_dir / "logo-dark-eo.svg", "#E8EEF5", font_path)


if __name__ == "__main__":
    main()
