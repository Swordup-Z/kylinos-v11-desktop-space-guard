#!/usr/bin/env bash
set -Eeuo pipefail

PREFIX=${PREFIX:-"$HOME/.local/share/kylin-space-guard"}
BIN_DIR=${BIN_DIR:-"$HOME/.local/bin"}
APP_DIR=${APP_DIR:-"$HOME/.local/share/applications"}
ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

install -d "$PREFIX/bin" "$PREFIX/gui" "$BIN_DIR" "$APP_DIR"
install -m 0755 "$ROOT_DIR/bin/kylin-space-guard" "$PREFIX/bin/kylin-space-guard"
install -m 0755 "$ROOT_DIR/gui/kylin-space-guard-gtk" "$PREFIX/gui/kylin-space-guard-gtk"

ln -sfn "$PREFIX/bin/kylin-space-guard" "$BIN_DIR/kylin-space-guard"
ln -sfn "$PREFIX/gui/kylin-space-guard-gtk" "$BIN_DIR/kylin-space-guard-gtk"

cat >"$APP_DIR/kylin-space-guard.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=麒麟V11空间清理
Name[zh_CN]=麒麟V11空间清理
Name[en_US]=KylinOS V11 Desktop Space Cleaner
Comment=清理和抑制 Kaiming/KARE 与 ostree 的异常空间占用
Comment[zh_CN]=清理和抑制 Kaiming/KARE 与 ostree 的异常空间占用
Comment[en_US]=Clean and control Kaiming/KARE and ostree space usage on KylinOS Desktop V11
Exec=$PREFIX/gui/kylin-space-guard-gtk
Icon=preferences-system
Terminal=false
Categories=System;Utility;
Keywords=kylin;kylinos;v11;desktop;kaiming;kare;ostree;storage;cleanup;空间清理;麒麟;
EOF

echo "Installed 麒麟V11空间清理 / KylinOS V11 Desktop Space Cleaner"
echo "GUI: $PREFIX/gui/kylin-space-guard-gtk"
echo "CLI: $PREFIX/bin/kylin-space-guard"
echo "Desktop entry: $APP_DIR/kylin-space-guard.desktop"
