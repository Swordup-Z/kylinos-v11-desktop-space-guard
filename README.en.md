# KylinOS V11 Desktop Space Cleaner

[中文](README.md)

KylinOS V11 Desktop Space Cleaner is a desktop space analysis and conservative cleanup tool for KylinOS Desktop V11. It shows root filesystem usage, Kaiming, KARE, ostree writable layers, and application container usage, then applies only user-confirmed cleanup or optimization actions.

## Features

- Show root filesystem usage with Kaiming, KARE, ostree writable layers, app usage, system cache, and other categories.
- Show Kaiming application container statistics and per-application container details.
- Detect old Kaiming container versions and move safe candidates to a DATA rollback quarantine.
- Manage autostart entries, including disabling enabled entries and restoring entries previously disabled by the tool.
- Show scan progress, execution plans, operation results, and failure log paths.

## Safety Boundaries

The tool is intentionally conservative:

- It does not delete `/ostree`, `/sysroot/ostree`, EFI files, GRUB configuration, loader entries, `/etc/fstab`, or partition tables.
- It does not present cleanup as application uninstall.
- It does not silently remove currently used container layers.
- System-level changes are executed through `pkexec` and follow Kylin maintain-mode requirements.
- Old-container cleanup moves candidates into a rollback quarantine instead of permanently deleting them directly.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

You can also use the project Makefile:

```bash
make
```

## Install For Current User

```bash
./install.sh
```

or:

```bash
make install
```

After installation, launch **KylinOS V11 Desktop Space Cleaner** from the desktop, or run:

```bash
kylin-space-cleaner
```

## Command Line

Run one scan:

```bash
kylin-space-cleaner-helper --scan --user "$USER"
```

Inspect the legacy compatibility CLI:

```bash
./bin/kylin-space-guard --dry-run --user "$USER"
```

## Dependencies

- CMake
- Ninja
- Qt 5 Widgets / Charts
- `pkexec`
- Kaiming features depend on the system Kaiming toolchain

## Design

The desktop UI is implemented with C++/Qt Widgets. Scans and privileged operations are handled by a local helper, while the GUI presents status, candidates, selection flows, progress, and results. The cleanup model prioritizes confirmation, rollback, and verification.

## License

MIT
