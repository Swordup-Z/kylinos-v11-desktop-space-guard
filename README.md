# 麒麟V11空间清理

[English](README.en.md)

麒麟V11空间清理是面向 KylinOS Desktop V11 的桌面空间分析与保守清理工具。它用于查看根分区、Kaiming、KARE、ostree 写入层和应用容器占用，并在用户确认后执行可回滚、可审计的清理或优化操作。

## 功能

- 展示根分区总占用，并区分 Kaiming、KARE、ostree 写入层、APP 占用和系统缓存等类别。
- 展示 Kaiming 应用容器统计和每个应用的容器明细。
- 针对 KARE/Kaiming 运行环境识别可清理候选：Kaiming 旧版本层、非活跃 KARE 写入层缓存/临时数据，并把安全候选移动到 DATA 回滚隔离区。
- 管理自启动项，支持禁用已启用项，也支持还原之前禁用过的项目。
- 显示扫描进度、执行计划、执行结果和失败日志路径。

## 安全边界

该工具采用保守策略：

- 不删除 `/ostree`、`/sysroot/ostree`、EFI 文件、GRUB 配置、loader 条目、`/etc/fstab` 或分区表。
- 不把清理行为伪装成应用卸载。
- 不静默删除当前正在使用的 Kaiming 层或正在挂载的 KARE 写入层。
- 涉及系统级变更时通过 `pkexec` 执行，并遵循 Kylin 维护模式要求。
- 清理候选会移动到回滚隔离区，而不是直接永久删除。
- Kaiming repo 对象仓库和应用状态目录目前只做占用展示，不做自动删除；这类数据必须依赖官方工具或更严格的引用校验。

## 构建

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

也可以使用项目内 Makefile：

```bash
make
```

## 安装到当前用户

```bash
./install.sh
```

或：

```bash
make install
```

安装后可以从桌面启动 **麒麟V11空间清理**，也可以直接运行：

```bash
kylin-space-cleaner
```

## 命令行

执行一次扫描：

```bash
kylin-space-cleaner-helper --scan --user "$USER"
```

查看旧版兼容 CLI：

```bash
./bin/kylin-space-guard --dry-run --user "$USER"
```

## 依赖

- CMake
- Ninja
- Qt 5 Widgets / Charts
- `pkexec`
- Kaiming 相关能力依赖系统中的 Kaiming 工具链

## 设计

桌面界面使用 C++/Qt Widgets 实现。扫描和高权限操作由本地 helper 执行，GUI 负责展示状态、候选项、选择流程、进度和结果。清理逻辑以可确认、可回滚和可验证为优先级。

## 许可证

MIT
