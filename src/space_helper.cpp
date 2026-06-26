#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStorageInfo>
#include <QTextStream>

#include <future>

#include <unistd.h>

struct AutostartEntry {
    QString id;
    QString nameZh;
    QString nameEn;
    QString descriptionZh;
    QString descriptionEn;
    QStringList sources;
};

static const QString kKaimingRoot = QStringLiteral("/var/opt/kaiming");
static const QString kQuarantineRoot =
    QStringLiteral("/data/usershare/kylinos-system-rollbacks/storage/kylin-space-guard-cleanup");

static QString runCapture(const QString &program, const QStringList &args, int *exitCode = nullptr)
{
    QProcess process;
    process.start(program, args);
    process.waitForFinished(-1);
    if (exitCode) {
        *exitCode = process.exitCode();
    }
    return QString::fromLocal8Bit(process.readAllStandardOutput())
        + QString::fromLocal8Bit(process.readAllStandardError());
}

static qint64 duBytes(const QString &path)
{
    const QString output = runCapture(QStringLiteral("du"), {QStringLiteral("-sb"), path});
    qint64 lastValue = 0;
    const QRegularExpression numberLine(QStringLiteral("^\\s*(\\d+)\\s+"));
    for (const QString &line : output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QRegularExpressionMatch match = numberLine.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        bool ok = false;
        const qint64 value = match.captured(1).toLongLong(&ok);
        if (ok) {
            lastValue = value;
        }
    }
    return lastValue;
}

static qint64 rootUsedBytes()
{
    const QStorageInfo root(QStringLiteral("/"));
    return root.bytesTotal() > root.bytesFree() ? root.bytesTotal() - root.bytesFree() : 0;
}

static QString homeForUser(const QString &user)
{
    const QString line = runCapture(QStringLiteral("getent"), {QStringLiteral("passwd"), user}).trimmed();
    const QStringList parts = line.split(QLatin1Char(':'));
    if (parts.size() >= 6 && !parts.at(5).isEmpty()) {
        return parts.at(5);
    }
    return QDir::homePath();
}

static QString currentUser()
{
    const QByteArray sudoUser = qgetenv("SUDO_USER");
    if (!sudoUser.isEmpty() && sudoUser != "root") {
        return QString::fromLocal8Bit(sudoUser);
    }
    const QByteArray user = qgetenv("USER");
    if (!user.isEmpty()) {
        return QString::fromLocal8Bit(user);
    }
    return QStringLiteral("zengjianqi");
}

static QStringList findLayerDirs()
{
    int code = 0;
    const QString output = runCapture(
        QStringLiteral("find"),
        {QStringLiteral("/var/opt/kaiming/layers/stable"), QStringLiteral("-mindepth"), QStringLiteral("5"),
         QStringLiteral("-maxdepth"), QStringLiteral("5"), QStringLiteral("-type"), QStringLiteral("d"),
         QStringLiteral("-print")},
        &code);
    QStringList result;
    if (code == 0) {
        for (const QString &line : output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            result << line.trimmed();
        }
    }
    result.sort();
    return result;
}

static bool parseLayerPath(const QString &path, QString *kind, QString *ref, QString *module, QString *version)
{
    const QString prefix = QStringLiteral("/var/opt/kaiming/layers/stable/");
    if (!path.startsWith(prefix)) {
        return false;
    }
    const QStringList parts = path.mid(prefix.size()).split(QLatin1Char('/'));
    if (parts.size() != 5) {
        return false;
    }
    if (kind) {
        *kind = parts.at(1);
    }
    if (ref) {
        *ref = parts.at(2);
    }
    if (module) {
        *module = parts.at(3);
    }
    if (version) {
        *version = parts.at(4);
    }
    return !parts.at(1).isEmpty() && !parts.at(2).isEmpty() && !parts.at(4).isEmpty();
}

static QString normalizeKaimingLayerPath(QString path)
{
    if (path.startsWith(QStringLiteral("/opt/kaiming/"))) {
        path.replace(QStringLiteral("/opt/kaiming/"), QStringLiteral("/var/opt/kaiming/"));
    }
    return path;
}

static QString normalizeRuntimePath(QString path)
{
    if (path.startsWith(QStringLiteral("/opt/kare-applications/"))) {
        path.replace(QStringLiteral("/opt/kare-applications/"), QStringLiteral("/var/opt/kare-applications/"));
    }
    if (path.startsWith(QStringLiteral("/opt/kaiming/"))) {
        path.replace(QStringLiteral("/opt/kaiming/"), QStringLiteral("/var/opt/kaiming/"));
    }
    return path;
}

static bool pathUnder(const QString &path, const QString &root)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QString cleanRoot = QDir::cleanPath(root);
    return cleanPath == cleanRoot || cleanPath.startsWith(cleanRoot + QLatin1Char('/'));
}

static QSet<QString> currentLayerPaths()
{
    QSet<QString> paths;
    const QDir infoDir(QStringLiteral("/var/opt/kaiming/info"));
    const QStringList files = infoDir.entryList({QStringLiteral("*.list")}, QDir::Files);
    for (const QString &fileName : files) {
        QFile file(infoDir.absoluteFilePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        while (!file.atEnd()) {
            QString line = QString::fromLocal8Bit(file.readLine()).trimmed();
            if (!line.startsWith(QStringLiteral("/opt/kaiming/layers/stable/"))) {
                continue;
            }
            line = normalizeKaimingLayerPath(line);
            QString kind;
            QString ref;
            QString module;
            QString version;
            if (parseLayerPath(line, &kind, &ref, &module, &version)) {
                paths.insert(line);
            }
        }
    }
    return paths;
}

static bool pathInUse(const QString &path)
{
    const QDir proc(QStringLiteral("/proc"));
    const QStringList pids = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &pid : pids) {
        bool ok = false;
        pid.toInt(&ok);
        if (!ok) {
            continue;
        }
        QFile mountInfo(QStringLiteral("/proc/%1/mountinfo").arg(pid));
        if (!mountInfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        if (QString::fromLocal8Bit(mountInfo.readAll()).contains(path)) {
            return true;
        }
    }
    return false;
}

static QJsonArray oldContainerCandidates()
{
    QJsonArray array;
    const QSet<QString> current = currentLayerPaths();
    for (const QString &path : findLayerDirs()) {
        QString kind;
        QString ref;
        QString module;
        QString version;
        if (!parseLayerPath(path, &kind, &ref, &module, &version)) {
            continue;
        }
        if (current.contains(path)) {
            continue;
        }
        QJsonObject item;
        item.insert(QStringLiteral("path"), path);
        item.insert(QStringLiteral("kind"), kind);
        item.insert(QStringLiteral("ref"), ref);
        item.insert(QStringLiteral("module"), module);
        item.insert(QStringLiteral("version"), version);
        item.insert(QStringLiteral("bytes"), QString::number(duBytes(path)));
        item.insert(QStringLiteral("inUse"), pathInUse(path));
        array.append(item);
    }
    return array;
}

static QStringList activeOverlayPaths()
{
    int code = 0;
    const QString output = runCapture(QStringLiteral("findmnt"),
                                      {QStringLiteral("-t"), QStringLiteral("overlay"),
                                       QStringLiteral("-o"), QStringLiteral("OPTIONS"), QStringLiteral("-n")},
                                      &code);
    QStringList paths;
    const QRegularExpression optionRe(QStringLiteral("(?:upperdir|workdir)=([^,]+)"));
    auto it = optionRe.globalMatch(output);
    while (it.hasNext()) {
        paths << normalizeRuntimePath(it.next().captured(1).trimmed());
    }
    paths.removeDuplicates();
    return paths;
}

static bool runtimePathActive(const QString &path, const QStringList &activePaths)
{
    const QString normalized = normalizeRuntimePath(path);
    for (const QString &active : activePaths) {
        if (pathUnder(normalized, active) || pathUnder(active, normalized)) {
            return true;
        }
    }
    return false;
}

static QString cleanupKindZh(const QString &kind)
{
    if (kind == QStringLiteral("kaiming-old-layer")) {
        return QStringLiteral("Kaiming 旧版本文件");
    }
    if (kind == QStringLiteral("kare-cache")) {
        return QStringLiteral("KARE 缓存文件");
    }
    if (kind == QStringLiteral("kare-temp")) {
        return QStringLiteral("KARE 临时文件");
    }
    if (kind == QStringLiteral("kare-log")) {
        return QStringLiteral("KARE 日志文件");
    }
    return kind;
}

static QString cleanupKindEn(const QString &kind)
{
    if (kind == QStringLiteral("kaiming-old-layer")) {
        return QStringLiteral("Kaiming old version files");
    }
    if (kind == QStringLiteral("kare-cache")) {
        return QStringLiteral("KARE cache files");
    }
    if (kind == QStringLiteral("kare-temp")) {
        return QStringLiteral("KARE temporary files");
    }
    if (kind == QStringLiteral("kare-log")) {
        return QStringLiteral("KARE log files");
    }
    return kind;
}

static QJsonObject makeCleanupCandidate(const QString &id,
                                        const QString &subsystem,
                                        const QString &kind,
                                        const QString &scope,
                                        const QString &path,
                                        qint64 bytes,
                                        bool cleanable,
                                        bool inUse,
                                        const QString &reasonZh,
                                        const QString &reasonEn)
{
    QJsonObject item;
    item.insert(QStringLiteral("id"), id);
    item.insert(QStringLiteral("subsystem"), subsystem);
    item.insert(QStringLiteral("kind"), kind);
    item.insert(QStringLiteral("kindZh"), cleanupKindZh(kind));
    item.insert(QStringLiteral("kindEn"), cleanupKindEn(kind));
    item.insert(QStringLiteral("scope"), scope);
    item.insert(QStringLiteral("path"), path);
    item.insert(QStringLiteral("bytes"), QString::number(bytes));
    item.insert(QStringLiteral("cleanable"), cleanable);
    item.insert(QStringLiteral("inUse"), inUse);
    item.insert(QStringLiteral("reasonZh"), reasonZh);
    item.insert(QStringLiteral("reasonEn"), reasonEn);
    return item;
}

static QJsonArray cleanupCandidates()
{
    QJsonArray result;
    const QJsonArray oldLayers = oldContainerCandidates();
    for (const QJsonValue &value : oldLayers) {
        const QJsonObject old = value.toObject();
        const bool inUse = old.value(QStringLiteral("inUse")).toBool();
        const QString ref = old.value(QStringLiteral("ref")).toString();
        const QString module = old.value(QStringLiteral("module")).toString();
        const QString version = old.value(QStringLiteral("version")).toString();
        result.append(makeCleanupCandidate(old.value(QStringLiteral("path")).toString(),
                                           QStringLiteral("kaiming"),
                                           QStringLiteral("kaiming-old-layer"),
                                           QStringLiteral("%1 / %2 / %3").arg(ref, module, version),
                                           old.value(QStringLiteral("path")).toString(),
                                           old.value(QStringLiteral("bytes")).toString().toLongLong(),
                                           !inUse,
                                           inUse,
                                           inUse
                                               ? QStringLiteral("旧版本层仍被挂载或进程使用，暂不自动清理。")
                                               : QStringLiteral("不在当前 Kaiming 版本清单中，可移动到 DATA 回滚隔离区。"),
                                           inUse
                                               ? QStringLiteral("The old layer is still mounted or used by a process.")
                                               : QStringLiteral("Not listed by the current Kaiming version set; can be moved to rollback quarantine.")));
    }

    const QStringList activePaths = activeOverlayPaths();
    const QDir kareRoot(QStringLiteral("/var/opt/kare-applications"));
    const QStringList profiles = kareRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    const QList<QPair<QString, QString>> relativeTargets{
        {QStringLiteral("upper/var/cache"), QStringLiteral("kare-cache")},
        {QStringLiteral("upper/var/tmp"), QStringLiteral("kare-temp")},
        {QStringLiteral("upper/tmp"), QStringLiteral("kare-temp")},
        {QStringLiteral("upper/root/.cache"), QStringLiteral("kare-cache")},
        {QStringLiteral("upper/var/log"), QStringLiteral("kare-log")}
    };
    for (const QString &profile : profiles) {
        const QString profileRoot = kareRoot.absoluteFilePath(profile);
        if (!QFileInfo(profileRoot + QStringLiteral("/upper")).isDir()) {
            continue;
        }
        const bool active = runtimePathActive(profileRoot + QStringLiteral("/upper"), activePaths)
            || runtimePathActive(profileRoot + QStringLiteral("/work"), activePaths);
        for (const auto &target : relativeTargets) {
            const QString path = profileRoot + QLatin1Char('/') + target.first;
            if (!QFileInfo(path).isDir()) {
                continue;
            }
            const qint64 bytes = duBytes(path);
            if (bytes <= 0) {
                continue;
            }
            const bool cleanable = !active;
            result.append(makeCleanupCandidate(path,
                                               QStringLiteral("kare"),
                                               target.second,
                                               profile,
                                               path,
                                               bytes,
                                               cleanable,
                                               active,
                                               active
                                                   ? QStringLiteral("该 KARE 写入层正在挂载使用，只统计占用，暂不自动清理。")
                                                   : QStringLiteral("该 KARE 写入层当前未挂载，缓存/临时数据可移动到 DATA 回滚隔离区。"),
                                               active
                                                   ? QStringLiteral("This KARE writable layer is currently mounted; it is shown for visibility only.")
                                                   : QStringLiteral("This KARE writable layer is not mounted; cache or temporary data can be moved to rollback quarantine.")));
        }
    }
    return result;
}

static QJsonArray applicationContainers()
{
    struct AppInfo {
        QString ref;
        QString kind;
        qint64 bytes = 0;
        QJsonArray layers;
    };

    QMap<QString, AppInfo> apps;
    const QSet<QString> current = currentLayerPaths();
    for (const QString &path : findLayerDirs()) {
        QString kind;
        QString ref;
        QString module;
        QString version;
        if (!parseLayerPath(path, &kind, &ref, &module, &version)) {
            continue;
        }
        const qint64 bytes = duBytes(path);
        const QString key = kind + QLatin1Char('|') + ref;
        if (!apps.contains(key)) {
            AppInfo info;
            info.ref = ref;
            info.kind = kind;
            apps.insert(key, info);
        }
        QJsonObject layer;
        layer.insert(QStringLiteral("path"), path);
        layer.insert(QStringLiteral("kind"), kind);
        layer.insert(QStringLiteral("ref"), ref);
        layer.insert(QStringLiteral("module"), module);
        layer.insert(QStringLiteral("version"), version);
        layer.insert(QStringLiteral("bytes"), QString::number(bytes));
        layer.insert(QStringLiteral("current"), current.contains(path));
        layer.insert(QStringLiteral("inUse"), pathInUse(path));
        apps[key].bytes += bytes;
        apps[key].layers.append(layer);
    }

    QJsonArray array;
    for (const AppInfo &app : apps) {
        QJsonObject item;
        item.insert(QStringLiteral("ref"), app.ref);
        item.insert(QStringLiteral("kind"), app.kind);
        item.insert(QStringLiteral("bytes"), QString::number(app.bytes));
        item.insert(QStringLiteral("containers"), app.layers);
        item.insert(QStringLiteral("containerCount"), app.layers.size());
        array.append(item);
    }
    return array;
}

static QList<AutostartEntry> knownAutostarts()
{
    return {
        {QStringLiteral("kylin-note_autoStart.desktop"),
         QStringLiteral("麒麟便签静默启动"),
         QStringLiteral("Kylin Note silent start"),
         QStringLiteral("登录后静默拉起 Kaiming 便签进程。禁用后不会卸载便签，只是不再自动启动。"),
         QStringLiteral("Starts Kylin Note silently after login. Disabling does not uninstall the app."),
         {QStringLiteral("/etc/xdg/autostart/kylin-note_autoStart.desktop"),
          QStringLiteral("/usr/etc/xdg/autostart/kylin-note_autoStart.desktop")}},
        {QStringLiteral("qaxbrowser-safe-preheat.desktop"),
         QStringLiteral("奇安信浏览器自启动项"),
         QStringLiteral("QaxBrowser autostart entry"),
         QStringLiteral("登录后自动启动浏览器相关后台进程。禁用后不会卸载浏览器，只是不再自动启动。"),
         QStringLiteral("Starts browser-related background processes after login. Disabling does not uninstall the browser."),
         {QStringLiteral("/etc/xdg/autostart/qaxbrowser-safe-preheat.desktop"),
          QStringLiteral("/usr/etc/xdg/autostart/qaxbrowser-safe-preheat.desktop")}},
        {QStringLiteral("dbus-daemon-proxy.desktop"),
         QStringLiteral("KARE D-Bus 代理"),
         QStringLiteral("KARE D-Bus proxy"),
         QStringLiteral("为 KARE 兼容环境启动会话 D-Bus 代理；禁用前应确认不依赖旧版 KARE 应用。"),
         QStringLiteral("Starts a session D-Bus proxy for KARE compatibility. Keep enabled if legacy KARE apps need it."),
         {QStringLiteral("/etc/xdg/autostart/dbus-daemon-proxy.desktop"),
          QStringLiteral("/usr/etc/xdg/autostart/dbus-daemon-proxy.desktop")}},
    };
}

static bool fileContainsHiddenTrue(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QString text = QString::fromLocal8Bit(file.readAll());
    return text.contains(QRegularExpression(QStringLiteral("(?im)^\\s*Hidden\\s*=\\s*true\\s*$")));
}

static QJsonArray autostartCandidates(const QString &user)
{
    QJsonArray array;
    const QString home = homeForUser(user);
    for (const AutostartEntry &entry : knownAutostarts()) {
        QString source;
        for (const QString &candidate : entry.sources) {
            if (QFileInfo::exists(candidate)) {
                source = candidate;
                break;
            }
        }
        if (source.isEmpty()) {
            continue;
        }
        const QString target = home + QStringLiteral("/.config/autostart/") + entry.id;
        QJsonObject item;
        item.insert(QStringLiteral("id"), entry.id);
        item.insert(QStringLiteral("nameZh"), entry.nameZh);
        item.insert(QStringLiteral("nameEn"), entry.nameEn);
        item.insert(QStringLiteral("descriptionZh"), entry.descriptionZh);
        item.insert(QStringLiteral("descriptionEn"), entry.descriptionEn);
        item.insert(QStringLiteral("source"), source);
        item.insert(QStringLiteral("target"), target);
        item.insert(QStringLiteral("disabled"), fileContainsHiddenTrue(target));
        array.append(item);
    }
    return array;
}

static qint64 dirsBytesByFindName(const QString &root, const QStringList &names)
{
    qint64 total = 0;
    int code = 0;
    QStringList args{root, QStringLiteral("-type"), QStringLiteral("d"), QStringLiteral("(")};
    for (int i = 0; i < names.size(); ++i) {
        if (i > 0) {
            args << QStringLiteral("-o");
        }
        args << QStringLiteral("-name") << names.at(i);
    }
    args << QStringLiteral(")") << QStringLiteral("-print");
    const QString output = runCapture(QStringLiteral("find"), args, &code);
    if (code != 0) {
        // find can return 1 on protected Kylin directories while still printing
        // readable matches. Keep parsing stdout/stderr lines and ignore errors.
    }
    for (const QString &line : output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QString path = line.trimmed();
        if (path.startsWith(QStringLiteral("find:"))) {
            continue;
        }
        total += duBytes(path);
    }
    return total;
}

static qint64 directChildDirsBytes(const QString &root, const QStringList &names)
{
    qint64 total = 0;
    const QDir dir(root);
    const QStringList children = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &child : children) {
        const QString childRoot = dir.absoluteFilePath(child);
        for (const QString &name : names) {
            const QString path = childRoot + QLatin1Char('/') + name;
            if (QFileInfo(path).isDir()) {
                total += duBytes(path);
            }
        }
    }
    return total;
}

static QJsonObject scan(const QString &user)
{
    auto rootUsedFuture = std::async(std::launch::async, rootUsedBytes);
    auto kaimingFuture = std::async(std::launch::async, [] {
        return duBytes(kKaimingRoot);
    });
    auto ostreeFuture = std::async(std::launch::async, [] {
        return dirsBytesByFindName(QStringLiteral("/sysroot/ostree/pkgs"),
                                   {QStringLiteral("*-upper"), QStringLiteral("*-tmpupper")});
    });
    auto kareFuture = std::async(std::launch::async, [] {
        return directChildDirsBytes(QStringLiteral("/opt/kare-applications"),
                                    {QStringLiteral("upper"), QStringLiteral("work")});
    });
    auto kareBaseFuture = std::async(std::launch::async, [] {
        return duBytes(QStringLiteral("/var/opt/kare-applications/base"));
    });
    auto appPayloadFuture = std::async(std::launch::async, [] {
        return duBytes(QStringLiteral("/var/opt/kingsoft"))
            + duBytes(QStringLiteral("/var/opt/apps"))
            + duBytes(QStringLiteral("/var/opt/wechat"));
    });
    auto oldContainersFuture = std::async(std::launch::async, oldContainerCandidates);
    auto autostartsFuture = std::async(std::launch::async, [user] {
        return autostartCandidates(user);
    });

    QJsonObject root;
    QJsonObject metrics;
    const qint64 rootUsed = rootUsedFuture.get();
    const qint64 kaiming = kaimingFuture.get();
    const qint64 ostreeUpper = ostreeFuture.get();
    const qint64 kareUpper = kareFuture.get();
    const qint64 kareBase = kareBaseFuture.get();
    const qint64 appPayload = appPayloadFuture.get();
    const qint64 systemOther = qMax<qint64>(0, rootUsed - kaiming - ostreeUpper - kareUpper - kareBase - appPayload);
    metrics.insert(QStringLiteral("root_used"), QString::number(rootUsed));
    metrics.insert(QStringLiteral("kaiming"), QString::number(kaiming));
    metrics.insert(QStringLiteral("ostree_upper"), QString::number(ostreeUpper));
    metrics.insert(QStringLiteral("kare_upper"), QString::number(kareUpper));
    metrics.insert(QStringLiteral("kare_base"), QString::number(kareBase));
    metrics.insert(QStringLiteral("app_payload"), QString::number(appPayload));
    metrics.insert(QStringLiteral("system_other"), QString::number(systemOther));
    root.insert(QStringLiteral("metrics"), metrics);
    root.insert(QStringLiteral("oldContainers"), oldContainersFuture.get());
    root.insert(QStringLiteral("cleanupCandidates"), cleanupCandidates());
    root.insert(QStringLiteral("autostarts"), autostartsFuture.get());
    root.insert(QStringLiteral("applications"), applicationContainers());
    root.insert(QStringLiteral("mode"), runCapture(QStringLiteral("mm-cli"), {QStringLiteral("-s")}).trimmed());
    root.insert(QStringLiteral("user"), user);
    root.insert(QStringLiteral("time"), QDateTime::currentDateTime().toString(Qt::ISODate));
    return root;
}

static int manageAutostart(const QString &user, const QStringList &disableIds, const QStringList &enableIds)
{
    const QString home = homeForUser(user);
    const QJsonArray candidates = autostartCandidates(user);
    QSet<QString> valid;
    for (const QJsonValue &value : candidates) {
        valid.insert(value.toObject().value(QStringLiteral("id")).toString());
    }

    QJsonArray results;
    QDir().mkpath(home + QStringLiteral("/.config/autostart"));
    for (const QString &id : disableIds) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), id);
        item.insert(QStringLiteral("targetState"), QStringLiteral("disabled"));
        if (!valid.contains(id)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("not a known active autostart entry"));
            results.append(item);
            continue;
        }
        const QString target = home + QStringLiteral("/.config/autostart/") + id;
        QFile file(target);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), file.errorString());
            results.append(item);
            continue;
        }
        QTextStream out(&file);
        out << "[Desktop Entry]\n";
        out << "Type=Application\n";
        out << "Name=" << id << "\n";
        out << "Hidden=true\n";
        file.close();
        runCapture(QStringLiteral("chown"), {user + QLatin1Char(':') + user, target});
        item.insert(QStringLiteral("ok"), true);
        item.insert(QStringLiteral("message"), QStringLiteral("Hidden=true override written"));
        item.insert(QStringLiteral("target"), target);
        results.append(item);
    }
    for (const QString &id : enableIds) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), id);
        item.insert(QStringLiteral("targetState"), QStringLiteral("enabled"));
        if (!valid.contains(id)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("not a known autostart entry"));
            results.append(item);
            continue;
        }
        const QString target = home + QStringLiteral("/.config/autostart/") + id;
        QFile file(target);
        bool ok = true;
        QString message = QStringLiteral("already inherited from system autostart");
        if (QFileInfo::exists(target)) {
            ok = file.remove();
            message = ok ? QStringLiteral("user Hidden=true override removed") : file.errorString();
        }
        item.insert(QStringLiteral("ok"), ok);
        item.insert(QStringLiteral("message"), message);
        item.insert(QStringLiteral("target"), target);
        results.append(item);
    }

    QJsonObject root;
    root.insert(QStringLiteral("action"), QStringLiteral("manageAutostart"));
    root.insert(QStringLiteral("results"), results);
    QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
    return 0;
}

static bool requireMaintainMode(QString *message)
{
    if (QCoreApplication::applicationPid() <= 0) {
        return false;
    }
    if (::getuid() != 0) {
        *message = QStringLiteral("root privileges required");
        return false;
    }
    const QString mode = runCapture(QStringLiteral("mm-cli"), {QStringLiteral("-s")}).trimmed();
    if (!mode.contains(QStringLiteral("Maintain"), Qt::CaseInsensitive)) {
        *message = QStringLiteral("system is not in maintain mode");
        return false;
    }
    return true;
}

static int applyOldContainers(const QStringList &paths)
{
    QString message;
    if (!requireMaintainMode(&message)) {
        QJsonObject root;
        root.insert(QStringLiteral("ok"), false);
        root.insert(QStringLiteral("message"), message);
        QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
        return 2;
    }

    const QSet<QString> current = currentLayerPaths();
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss"));
    const QString qroot = kQuarantineRoot + QLatin1Char('/') + stamp;
    QJsonArray results;
    for (const QString &path : paths) {
        QJsonObject item;
        item.insert(QStringLiteral("path"), path);
        QString kind;
        QString ref;
        QString module;
        QString version;
        if (!parseLayerPath(path, &kind, &ref, &module, &version)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("invalid Kaiming layer path"));
            results.append(item);
            continue;
        }
        if (current.contains(path)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("path is still listed as current"));
            results.append(item);
            continue;
        }
        if (pathInUse(path)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("path is mounted or in use"));
            results.append(item);
            continue;
        }
        const qint64 bytes = duBytes(path);
        const QString rel = path.mid(QStringLiteral("/var/opt/kaiming/layers/").size());
        const QString dest = qroot + QLatin1Char('/') + rel;
        QDir().mkpath(QFileInfo(dest).absolutePath());
        int code = 0;
        const QString output = runCapture(QStringLiteral("mv"), {path, dest}, &code);
        item.insert(QStringLiteral("ok"), code == 0);
        item.insert(QStringLiteral("bytes"), QString::number(code == 0 ? bytes : 0));
        item.insert(QStringLiteral("dest"), dest);
        item.insert(QStringLiteral("message"), code == 0 ? QStringLiteral("moved to rollback quarantine") : output);
        results.append(item);
    }
    QJsonObject root;
    root.insert(QStringLiteral("action"), QStringLiteral("applyOldContainers"));
    root.insert(QStringLiteral("results"), results);
    QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
    return 0;
}

static QString quarantinePathFor(const QString &stamp, const QString &sourcePath)
{
    QString rootName = QStringLiteral("system");
    QString rel = QDir::cleanPath(sourcePath);
    if (rel.startsWith(QStringLiteral("/var/opt/kaiming/"))) {
        rootName = QStringLiteral("kaiming");
        rel = rel.mid(QStringLiteral("/var/opt/kaiming/").size());
    } else if (rel.startsWith(QStringLiteral("/var/opt/kare-applications/"))) {
        rootName = QStringLiteral("kare");
        rel = rel.mid(QStringLiteral("/var/opt/kare-applications/").size());
    } else if (rel.startsWith(QLatin1Char('/'))) {
        rel = rel.mid(1);
    }
    return kQuarantineRoot + QLatin1Char('/') + stamp + QLatin1Char('/') + rootName + QLatin1Char('/') + rel;
}

static QMap<QString, QJsonObject> cleanupCandidateMap()
{
    QMap<QString, QJsonObject> map;
    const QJsonArray candidates = cleanupCandidates();
    for (const QJsonValue &value : candidates) {
        const QJsonObject item = value.toObject();
        map.insert(item.value(QStringLiteral("id")).toString(), item);
    }
    return map;
}

static int applyCleanupCandidates(const QStringList &ids)
{
    QString message;
    if (!requireMaintainMode(&message)) {
        QJsonObject root;
        root.insert(QStringLiteral("ok"), false);
        root.insert(QStringLiteral("message"), message);
        QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
        return 2;
    }

    const QMap<QString, QJsonObject> candidates = cleanupCandidateMap();
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss"));
    QJsonArray results;
    for (const QString &id : ids) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), id);
        if (!candidates.contains(id)) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("cleanup candidate is no longer available"));
            results.append(item);
            continue;
        }

        const QJsonObject candidate = candidates.value(id);
        const QString path = candidate.value(QStringLiteral("path")).toString();
        item.insert(QStringLiteral("path"), path);
        item.insert(QStringLiteral("kind"), candidate.value(QStringLiteral("kind")).toString());
        if (!candidate.value(QStringLiteral("cleanable")).toBool()) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("candidate is not cleanable in current scan"));
            results.append(item);
            continue;
        }
        if (!QFileInfo(path).exists()) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("path no longer exists"));
            results.append(item);
            continue;
        }
        if (pathInUse(path) || runtimePathActive(path, activeOverlayPaths())) {
            item.insert(QStringLiteral("ok"), false);
            item.insert(QStringLiteral("message"), QStringLiteral("path is mounted or in use"));
            results.append(item);
            continue;
        }

        const qint64 bytes = duBytes(path);
        const QString dest = quarantinePathFor(stamp, path);
        QDir().mkpath(QFileInfo(dest).absolutePath());
        int code = 0;
        const QString output = runCapture(QStringLiteral("mv"), {path, dest}, &code);
        item.insert(QStringLiteral("ok"), code == 0);
        item.insert(QStringLiteral("bytes"), QString::number(code == 0 ? bytes : 0));
        item.insert(QStringLiteral("dest"), dest);
        item.insert(QStringLiteral("message"), code == 0 ? QStringLiteral("moved to rollback quarantine") : output);
        results.append(item);
    }
    QJsonObject root;
    root.insert(QStringLiteral("action"), QStringLiteral("applyCleanupCandidates"));
    root.insert(QStringLiteral("results"), results);
    QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
    return 0;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    QString user = currentUser();
    QStringList entries;
    QStringList disableEntries;
    QStringList enableEntries;
    QStringList containers;
    QStringList cleanupIds;
    bool doScan = args.contains(QStringLiteral("--scan"));
    bool doAutostart = args.contains(QStringLiteral("--apply-autostart"));
    bool doManageAutostart = args.contains(QStringLiteral("--manage-autostart"));
    bool doContainers = args.contains(QStringLiteral("--apply-old-containers"));
    bool doCleanupCandidates = args.contains(QStringLiteral("--apply-cleanup-candidates"));

    for (int i = 1; i < args.size(); ++i) {
        if (args.at(i) == QStringLiteral("--user") && i + 1 < args.size()) {
            user = args.at(++i);
        } else if (args.at(i) == QStringLiteral("--entries") && i + 1 < args.size()) {
            entries = args.at(++i).split(QLatin1Char(','), Qt::SkipEmptyParts);
        } else if (args.at(i) == QStringLiteral("--disable-entries") && i + 1 < args.size()) {
            disableEntries = args.at(++i).split(QLatin1Char(','), Qt::SkipEmptyParts);
        } else if (args.at(i) == QStringLiteral("--enable-entries") && i + 1 < args.size()) {
            enableEntries = args.at(++i).split(QLatin1Char(','), Qt::SkipEmptyParts);
        } else if (args.at(i) == QStringLiteral("--container") && i + 1 < args.size()) {
            containers << args.at(++i);
        } else if (args.at(i) == QStringLiteral("--cleanup-id") && i + 1 < args.size()) {
            cleanupIds << args.at(++i);
        }
    }

    if (doAutostart) {
        return manageAutostart(user, entries, {});
    }
    if (doManageAutostart) {
        return manageAutostart(user, disableEntries, enableEntries);
    }
    if (doContainers) {
        return applyOldContainers(containers);
    }
    if (doCleanupCandidates) {
        return applyCleanupCandidates(cleanupIds);
    }
    if (!doScan && !doAutostart && !doManageAutostart && !doContainers) {
        doScan = true;
    }

    if (doScan) {
        QTextStream(stdout) << QJsonDocument(scan(user)).toJson(QJsonDocument::Compact) << Qt::endl;
        return 0;
    }
    return 1;
}
