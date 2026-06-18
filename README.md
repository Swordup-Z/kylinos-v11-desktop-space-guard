# 麒麟V11空间清理

English name: **KylinOS V11 Desktop Space Cleaner**.

麒麟V11空间清理是面向 KylinOS Desktop V11 的保守型桌面清理工具。
它会展示 Kaiming/KARE 与 ostree 的空间占用，帮助禁用浪费空间的
Kaiming 自启动行为，并通过官方 `kaiming` 命令卸载指定 Kaiming 应用。

The app is intentionally defensive:

- It never deletes `/ostree`, `/sysroot/ostree`, EFI files, GRUB config,
  loader entries, `/etc/fstab`, or partition tables.
- It does not rewrite boot configuration.
- It uses `kaiming uninstall` for installed Kaiming apps.
- It only quarantines clearly non-current Kaiming version directories when the
  user explicitly requests that action.
- System-level actions require `pkexec` and Kylin maintain mode.

## Run From Source

```bash
./gui/kylin-space-guard-gtk
```

CLI scan:

```bash
./bin/kylin-space-guard --dry-run --user "$USER"
```

## Install For Current User

```bash
./install.sh
```

Then launch **麒麟V11空间清理** from the application menu, or run:

```bash
kylin-space-guard-gtk
```

## Dependencies

Runtime dependencies are intentionally small:

- `bash`
- `python3`
- `python3-gi`
- GTK 3 introspection bindings
- `pkexec` for privileged actions
- `/opt/kaiming-tools/bin/kaiming` when Kaiming actions are used

The CLI helper works without `rg`; it falls back to `grep`.

## Common Actions

Disable Kaiming/KARE preheat and silent autostart entries for the current user:

```bash
pkexec kylin-space-guard --apply --user "$USER" --disable-kaiming-autostart
```

Uninstall a Kaiming app while keeping user data:

```bash
pkexec kylin-space-guard --apply --user "$USER" --uninstall-kaiming-app org.mozilla.net.mozilla.firefox
```

Install a weekly reporting timer:

```bash
pkexec kylin-space-guard --apply --user "$USER" --install-monitor
```

## Design

The GTK app is a user interface wrapper. The auditable helper in
`bin/kylin-space-guard` owns all checks and privileged actions.

This split keeps the desktop experience simple while making system changes easy
to review, test, and automate.

## License

MIT
