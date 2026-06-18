#!/usr/bin/env bash
set -Eeuo pipefail

PREFIX=${PREFIX:-"$HOME/.local/share/kylin-space-guard"}
BIN_DIR=${BIN_DIR:-"$HOME/.local/bin"}
APP_DIR=${APP_DIR:-"$HOME/.local/share/applications"}
ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BUILD_DIR=${BUILD_DIR:-"$ROOT_DIR/build"}

if command -v cmake >/dev/null 2>&1 && [ -f "$ROOT_DIR/CMakeLists.txt" ]; then
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja
  cmake --build "$BUILD_DIR"
fi

install -d "$PREFIX/bin" "$PREFIX/gui" "$PREFIX/libexec" "$BIN_DIR" "$APP_DIR"
install -m 0755 "$ROOT_DIR/bin/kylin-space-guard" "$PREFIX/bin/kylin-space-guard"
install -m 0755 "$ROOT_DIR/gui/kylin-space-guard-gtk" "$PREFIX/gui/kylin-space-guard-gtk"
if [ -x "$BUILD_DIR/kylin-space-cleaner" ] && [ -x "$BUILD_DIR/kylin-space-cleaner-helper" ]; then
  install -m 0755 "$BUILD_DIR/kylin-space-cleaner" "$PREFIX/bin/kylin-space-cleaner"
  install -m 0755 "$BUILD_DIR/kylin-space-cleaner-helper" "$PREFIX/bin/kylin-space-cleaner-helper"
  install -m 0755 "$BUILD_DIR/kylin-space-cleaner-helper" "$PREFIX/libexec/kylin-space-cleaner-helper"
fi

ln -sfn "$PREFIX/bin/kylin-space-guard" "$BIN_DIR/kylin-space-guard"
ln -sfn "$PREFIX/gui/kylin-space-guard-gtk" "$BIN_DIR/kylin-space-guard-gtk"
if [ -x "$PREFIX/bin/kylin-space-cleaner" ]; then
  ln -sfn "$PREFIX/bin/kylin-space-cleaner" "$BIN_DIR/kylin-space-cleaner"
fi

cat >"$APP_DIR/kylin-space-guard.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=麒麟V11空间清理
Name[zh]=麒麟V11空间清理
Name[zh_CN]=麒麟V11空间清理
Name[en_US]=KylinOS V11 Desktop Space Cleaner
GenericName=KylinOS V11 Desktop Space Cleaner
GenericName[zh_CN]=KylinOS V11 Desktop Space Cleaner
GenericName[en_US]=KylinOS V11 Desktop Space Cleaner
Comment=清理和抑制 Kaiming/KARE 与 ostree 的异常空间占用
Comment[zh_CN]=清理和抑制 Kaiming/KARE 与 ostree 的异常空间占用
Comment[en_US]=Clean and control Kaiming/KARE and ostree space usage on KylinOS Desktop V11
Exec=$PREFIX/bin/kylin-space-cleaner
Icon=preferences-system
Terminal=false
Categories=System;
Keywords=kylin;kylinos;v11;desktop;kaiming;kare;ostree;storage;cleanup;空间清理;麒麟;
EOF

echo "Installed 麒麟V11空间清理 / KylinOS V11 Desktop Space Cleaner"
echo "GUI: $PREFIX/bin/kylin-space-cleaner"
echo "Helper: $PREFIX/bin/kylin-space-cleaner-helper"
echo "CLI: $PREFIX/bin/kylin-space-guard"
echo "Desktop entry: $APP_DIR/kylin-space-guard.desktop"
