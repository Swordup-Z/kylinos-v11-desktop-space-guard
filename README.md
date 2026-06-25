# 麒麟V11空间清理

English name: **KylinOS V11 Desktop Space Cleaner**.

麒麟V11空间清理是面向 KylinOS Desktop V11 的保守型桌面清理工具。
它会展示 Kaiming/KARE 与 ostree 的空间占用，识别 Kaiming 更新后遗留的
旧版本应用容器，并在用户确认后把安全候选移动到 DATA 隔离区。
它也可以帮助禁用浪费空间的 Kaiming 自启动行为。

The app is intentionally defensive:

- It never deletes `/ostree`, `/sysroot/ostree`, EFI files, GRUB config,
  loader entries, `/etc/fstab`, or partition tables.
- It does not rewrite boot configuration.
- It does not uninstall applications as a cleanup action.
- It only quarantines clearly non-current Kaiming application container version
  directories when the user explicitly requests that action.
- System-level actions require `pkexec` and Kylin maintain mode.

## Run From Source

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/kylin-space-cleaner
```

Convenience wrapper:

```bash
make
./build/kylin-space-cleaner
```

CLI scan:

```bash
./bin/kylin-space-guard --dry-run --user "$USER"
```

## Install For Current User

```bash
./install.sh
```

or:

```bash
make install
```

Then launch **麒麟V11空间清理** from a Chinese desktop session, or
**KylinOS V11 Desktop Space Cleaner** from an English desktop session. You can
also run:

```bash
kylin-space-cleaner
```

## Dependencies

Runtime dependencies are intentionally small:

- `bash`
- `cmake`
- `ninja-build`
- Qt 5 development/runtime packages
- `pkexec` for privileged actions
- `/opt/kaiming-tools/bin/kaiming` when Kaiming actions are used

The CLI helper works without `rg`; it falls back to `grep`.

## Common Actions

Move old Kaiming application container versions to a DATA rollback quarantine:

```bash
pkexec kylin-space-guard --apply --user "$USER" --clean-old-app-containers
```

Disable Kaiming/KARE silent autostart entries for the current user:

```bash
pkexec kylin-space-guard --apply --user "$USER" --disable-kaiming-autostart
```

Install a weekly reporting timer:

```bash
pkexec kylin-space-guard --apply --user "$USER" --install-monitor
```

## Design

The default desktop app is now a C++/Qt application. The auditable C++ helper
owns scans and privileged actions, while the UI presents a plan, selectable
items, current progress, and a result summary. The original Bash/GTK prototype
is kept for rule comparison and fallback testing.

## License

MIT
