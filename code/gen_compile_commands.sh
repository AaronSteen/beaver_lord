#!/bin/bash
# Generates compile_commands.json at the project root for clangd.
# Run this whenever build flags change, or whenever a new file is added
# that's #include'd into beaver.cpp as part of the unity build (rather
# than compiled as its own translation unit) -- it needs its own entry
# here, with -include flags replaying whatever beaver.cpp includes
# ahead of it, or clangd can't resolve types/symbols when you open it
# directly.
# Bear cannot be used here because the actual build runs Windows binaries via cmd.exe.
# Requires mingw-w64 for Windows headers: sudo apt install mingw-w64

SCRIPT_DIR=$(dirname "$(realpath "$0")")
ROOT=$(dirname "$SCRIPT_DIR")
BUILD="$ROOT/build"

FLAGS="-target x86_64-w64-mingw32 -isystem /usr/x86_64-w64-mingw32/include -std=c++17 -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-missing-field-initializers -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1"

cat > "$ROOT/compile_commands.json" << EOF
[
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS $SCRIPT_DIR/beaver.cpp",
    "file": "$SCRIPT_DIR/beaver.cpp"
},
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS $SCRIPT_DIR/win32_handmade.cpp",
    "file": "$SCRIPT_DIR/win32_handmade.cpp"
},
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS -include $SCRIPT_DIR/beaver.h $SCRIPT_DIR/tile.cpp",
    "file": "$SCRIPT_DIR/tile.cpp"
},
  {
    "directory": "$BUILD",
    "command": "clang $FLAGS -include $SCRIPT_DIR/beaver.h -include $SCRIPT_DIR/tile.cpp $SCRIPT_DIR/procedural.cpp",
    "file": "$SCRIPT_DIR/procedural.cpp"
}
]
EOF

echo "compile_commands.json written to $ROOT"
