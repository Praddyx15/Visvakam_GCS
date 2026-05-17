#!/usr/bin/env bash
# Generates resources/icons/visvakarn.icns from assets/visvakarn/Visvakarn_app-icon.png
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$REPO_ROOT/assets/visvakarn/Visvakarn_app-icon.png"
ICONSET="$REPO_ROOT/resources/icons/visvakarn.iconset"
OUT="$REPO_ROOT/resources/icons/visvakarn.icns"

mkdir -p "$ICONSET"

# sips cannot handle large RGBA PNGs; use Python Pillow for resizing
python3 - <<PYEOF
from PIL import Image
import os
src = "$SRC"
iconset = "$ICONSET"
img = Image.open(src).convert("RGBA")
sizes = [
    ("icon_16x16.png", 16),    ("icon_16x16@2x.png", 32),
    ("icon_32x32.png", 32),    ("icon_32x32@2x.png", 64),
    ("icon_64x64.png", 64),    ("icon_64x64@2x.png", 128),
    ("icon_128x128.png", 128), ("icon_128x128@2x.png", 256),
    ("icon_256x256.png", 256), ("icon_256x256@2x.png", 512),
    ("icon_512x512.png", 512), ("icon_512x512@2x.png", 1024),
]
for name, size in sizes:
    img.resize((size, size), Image.LANCZOS).save(os.path.join(iconset, name), "PNG")
PYEOF

iconutil -c icns "$ICONSET" -o "$OUT"
rm -rf "$ICONSET"

echo "Generated: $OUT"
