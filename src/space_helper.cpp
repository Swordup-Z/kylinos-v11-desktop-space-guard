#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStorageInfo>
#include <QTextStream>

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
    QStringLiteral("/data/usershare/kylinos-system-rollbacks/storage/kaiming-old-containers");

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
    return root.bytesTotal() > root.bytesAvailable() ? root.bytesTotal() - root.bytesAvailable() : 0;
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

static QSet<QString> currentLayerKeys()
{
    QSet<QString> keys;
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
            line.replace(QStringLiteral("/opt/kaiming/"), QStringLiteral("/var/opt/kaiming/"));
            QString kind;
            QString ref;
            QString module;
            QString version;
            if (parseLayerPath(line, &kind, &ref, &module, &version)) {
                keys.insert(ref + QLatin1Char('|') + version);
            }
        }
    }
    return keys;
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
    const QSet<QString> current = currentLayerKeys();
    for (const QString &path : findLayerDirs()) {
        QString kind;
        QString ref;
        QString module;
        QString version;
        if (!parseLayerPath(path, &kind, &ref, &module, &version)) {
            continue;
        }
        if (current.contains(ref + QLatin1Char('|') + version)) {
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
         QStringLiteral("奇安信浏览器预热"),
         QStringLiteral("QaxBrowser preheat"),
         QStringLiteral("登录后预热浏览器以换取启动速度，但会占用后台资源。"),
         QStringLiteral("Preheats the browser after login, trading background resources for faster startup."),
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
    QJsonObject root;
    QJsonObject metrics;
    metrics.insert(QStringLiteral("root_used"), QString::number(rootUsedBytes()));
    metrics.insert(QStringLiteral("kaiming"), QString::number(duBytes(kKaimingRoot)));
    metrics.insert(QStringLiteral("ostree_upper"),
                   QString::number(dirsBytesByFindName(QStringLiteral("/sysroot/ostree/pkgs"),
                                                       {QStringLiteral("*-upper"), QStringLiteral("*-tmpupper")})));
    metrics.insert(QStringLiteral("kare_upper"),
                   QString::number(directChildDirsBytes(QStringLiteral("/opt/kare-applications"),
                                                        {QStringLiteral("upper"), QStringLiteral("work")})));
    root.insert(QStringLiteral("metrics"), metrics);
    root.insert(QStringLiteral("oldContainers"), oldContainerCandidates());
    root.insert(QStringLiteral("autostarts"), autostartCandidates(user));
    root.insert(QStringLiteral("mode"), runCapture(QStringLiteral("mm-cli"), {QStringLiteral("-s")}).trimmed());
    root.insert(QStringLiteral("user"), user);
    root.insert(QStringLiteral("time"), QDateTime::currentDateTime().toString(Qt::ISODate));
    return root;
}

static int applyAutostart(const QString &user, const QStringList &ids)
{
    const QString home = homeForUser(user);
    const QJsonArray candidates = autostartCandidates(user);
    QSet<QString> valid;
    for (const QJsonValue &value : candidates) {
        valid.insert(value.toObject().value(QStringLiteral("id")).toString());
    }

    QJsonArray results;
    QDir().mkpath(home + QStringLiteral("/.config/autostart"));
    for (const QString &id : ids) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), id);
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

    QJsonObject root;
    root.insert(QStringLiteral("action"), QStringLiteral("applyAutostart"));
    root.insert(QStringLiteral("results"), results);
    QTextStream(stdout) << QJsonDocument(root).toJson(QJsonDocument::Compact) << Qt::endl;
    return 0;
}

static bool requireMaintainMode(QString *message)
{
    if (QCoreApplication::applicationPid() <= 0) {
        return false;
    }
    if (qEnvironmentVariableIntValue("EUID") != 0 && ::getuid() != 0) {
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

    const QSet<QString> current = currentLayerKeys();
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
        if (current.contains(ref + QLatin1Char('|') + version)) {
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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments();
    QString user = currentUser();
    QStringList entries;
    QStringList containers;
    bool doScan = args.contains(QStringLiteral("--scan"));
    bool doAutostart = args.contains(QStringLiteral("--apply-autostart"));
    bool doContainers = args.contains(QStringLiteral("--apply-old-containers"));

    for (int i = 1; i < args.size(); ++i) {
        if (args.at(i) == QStringLiteral("--user") && i + 1 < args.size()) {
            user = args.at(++i);
        } else if (args.at(i) == QStringLiteral("--entries") && i + 1 < args.size()) {
            entries = args.at(++i).split(QLatin1Char(','), Qt::SkipEmptyParts);
        } else if (args.at(i) == QStringLiteral("--container") && i + 1 < args.size()) {
            containers << args.at(++i);
        }
    }

    if (doAutostart) {
        return applyAutostart(user, entries);
    }
    if (doContainers) {
        return applyOldContainers(containers);
    }
    if (!doScan && !doAutostart && !doContainers) {
        doScan = true;
    }

    if (doScan) {
        QTextStream(stdout) << QJsonDocument(scan(user)).toJson(QJsonDocument::Compact) << Qt::endl;
        return 0;
    }
    return 1;
}
