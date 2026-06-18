#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTimer>

#include <cmath>

class SpaceVisual : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal phase READ phase WRITE setPhase)

public:
    explicit SpaceVisual(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedSize(184, 118);
        auto *animation = new QPropertyAnimation(this, "phase", this);
        animation->setDuration(3600);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setLoopCount(-1);
        animation->setEasingCurve(QEasingCurve::InOutSine);
        animation->start();
    }

    qreal phase() const { return phase_; }

    void setPhase(qreal phase)
    {
        phase_ = phase;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF card(1, 1, width() - 2, height() - 2);
        QLinearGradient bg(card.topLeft(), card.bottomRight());
        bg.setColorAt(0.00, QColor(255, 255, 255, 72));
        bg.setColorAt(0.48, QColor(255, 255, 255, 30));
        bg.setColorAt(1.00, QColor(255, 255, 255, 18));
        painter.setPen(QPen(QColor(255, 255, 255, 110), 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(card, 8, 8);

        QLinearGradient sheen(card.topLeft(), QPointF(card.right(), card.top()));
        sheen.setColorAt(0.00, QColor(255, 255, 255, 0));
        sheen.setColorAt(0.46 + 0.14 * std::sin(phase_ * 6.28318530718), QColor(255, 255, 255, 54));
        sheen.setColorAt(1.00, QColor(255, 255, 255, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(sheen);
        painter.drawRoundedRect(card.adjusted(1, 1, -1, -1), 8, 8);

        const QPointF center(62, 59);
        const QRectF outer(center.x() - 36, center.y() - 36, 72, 72);
        const QRectF inner(center.x() - 25, center.y() - 25, 50, 50);
        const qreal wave = std::sin(phase_ * 6.28318530718);
        const qreal sweep = 118 + 28 * wave;

        painter.setPen(QPen(QColor(255, 255, 255, 58), 8, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(outer, 35 * 16, 285 * 16);
        painter.setPen(QPen(QColor(103, 232, 208), 8, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(outer,
                        static_cast<int>((64 + phase_ * 360) * 16),
                        static_cast<int>(sweep * 16));
        painter.setPen(QPen(QColor(255, 206, 112, 190), 3, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(inner,
                        static_cast<int>((250 - phase_ * 270) * 16),
                        static_cast<int>(92 * 16));

        painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
        painter.drawEllipse(center, 18, 18);
        painter.setBrush(QColor(255, 255, 255, 235));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, 5, 5);

        painter.setBrush(QColor(255, 255, 255, 190));
        for (int i = 0; i < 4; ++i) {
            const qreal angle = phase_ * 6.28318530718 + i * 1.57079632679;
            const qreal radius = 42 + 3 * std::sin(phase_ * 6.28318530718 + i);
            painter.drawEllipse(QPointF(center.x() + std::cos(angle) * radius,
                                        center.y() + std::sin(angle) * radius),
                                i == 0 ? 2.6 : 1.8,
                                i == 0 ? 2.6 : 1.8);
        }

        const int bars[5] = {32, 56, 42, 70, 48};
        for (int i = 0; i < 5; ++i) {
            const QRectF slot(118 + i * 10, 27, 6, 64);
            painter.setBrush(QColor(255, 255, 255, 42));
            painter.drawRoundedRect(slot, 3, 3);
            const qreal pulse = 0.76 + 0.24 * std::sin(phase_ * 6.28318530718 + i * 0.62);
            const QRectF fill(slot.left(), slot.bottom() - bars[i] * pulse, slot.width(), bars[i] * pulse);
            painter.setBrush(i % 2 ? QColor(255, 205, 112, 222) : QColor(103, 232, 208, 222));
            painter.drawRoundedRect(fill, 3, 3);
        }

        painter.setPen(QPen(QColor(255, 255, 255, 115), 1));
        const qreal x = 12 + std::fmod(phase_ * 200, 160.0);
        painter.drawLine(QPointF(x, 13), QPointF(x + 28, 13));
        painter.setPen(QPen(QColor(255, 255, 255, 56), 1));
        painter.drawLine(QPointF(112, 96), QPointF(166, 96));
    }

private:
    qreal phase_ = 0.0;
};

class CleanerWindow : public QWidget {
    Q_OBJECT

public:
    CleanerWindow()
    {
        user_ = qEnvironmentVariable("USER");
        if (user_.isEmpty()) {
            user_ = QStringLiteral("zengjianqi");
        }
        helper_ = QApplication::applicationDirPath() + QStringLiteral("/kylin-space-cleaner-helper");
        if (!QFileInfo::exists(helper_)) {
            helper_ = QApplication::applicationDirPath() + QStringLiteral("/../libexec/kylin-space-cleaner-helper");
        }

        setMinimumSize(1040, 720);
        buildUi();
        applyLanguage();
        QTimer::singleShot(100, this, &CleanerWindow::scanManual);
        refreshTimer_ = new QTimer(this);
        refreshTimer_->setInterval(1000);
        connect(refreshTimer_, &QTimer::timeout, this, &CleanerWindow::refreshScan);
        refreshTimer_->start();
        debounceTimer_ = new QTimer(this);
        debounceTimer_->setSingleShot(true);
        debounceTimer_->setInterval(1000);
        connect(debounceTimer_, &QTimer::timeout, this, &CleanerWindow::refreshScan);
        setupWatchers();
    }

private:
    struct Text {
        QString title;
        QString language;
        QString user;
        QString intro;
        QString scan;
        QString cleanOld;
        QString disableAutostart;
        QString installMonitor;
        QString metric;
        QString before;
        QString released;
        QString current;
        QString rootUsed;
        QString kaiming;
        QString ostree;
        QString kare;
        QString stage;
        QString status;
        QString detail;
        QString details;
        QString ready;
        QString running;
        QString done;
        QString failed;
        QString planned;
        QString selectAutostarts;
        QString selectContainers;
        QString noAutostarts;
        QString noContainers;
        QString rawLog;
        QString live;
        QString updating;
        QString updated;
        QString autoRefresh;
    };

    static Text zh()
    {
        return {
            QStringLiteral("麒麟V11空间清理"),
            QStringLiteral("语言："),
            QStringLiteral("用户："),
            QStringLiteral("先扫描，再选择要执行的清理或抑制动作。执行前会展示计划，执行后会展示结果。"),
            QStringLiteral("扫描"),
            QStringLiteral("清理旧容器"),
            QStringLiteral("禁用预热/自启动"),
            QStringLiteral("安装监控"),
            QStringLiteral("项目"),
            QStringLiteral("执行前"),
            QStringLiteral("已释放"),
            QStringLiteral("当前占用"),
            QStringLiteral("根分区已用"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree 写入层"),
            QStringLiteral("KARE 写入层"),
            QStringLiteral("阶段"),
            QStringLiteral("状态"),
            QStringLiteral("说明"),
            QStringLiteral("错误日志"),
            QStringLiteral("就绪"),
            QStringLiteral("执行中"),
            QStringLiteral("完成"),
            QStringLiteral("失败"),
            QStringLiteral("计划"),
            QStringLiteral("选择要禁用的自启动项"),
            QStringLiteral("选择要清理的旧版本容器"),
            QStringLiteral("当前没有可禁用的自启动项，或都已经禁用。"),
            QStringLiteral("当前没有发现可安全清理的旧版本容器。"),
            QStringLiteral("错误日志"),
            QStringLiteral("实时状态"),
            QStringLiteral("正在更新"),
            QStringLiteral("已更新"),
            QStringLiteral("窗口可见时每秒刷新，扫描未完成时自动跳过")
        };
    }

    static Text en()
    {
        return {
            QStringLiteral("KylinOS V11 Desktop Space Cleaner"),
            QStringLiteral("Language:"),
            QStringLiteral("User:"),
            QStringLiteral("Scan first, then choose cleanup or suppression actions. The app shows the plan before execution and the result afterwards."),
            QStringLiteral("Scan"),
            QStringLiteral("Clean Old Containers"),
            QStringLiteral("Disable Preheat/Autostart"),
            QStringLiteral("Install Monitor"),
            QStringLiteral("Item"),
            QStringLiteral("Before"),
            QStringLiteral("Released"),
            QStringLiteral("Current"),
            QStringLiteral("Root Used"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree Upper"),
            QStringLiteral("KARE Upper"),
            QStringLiteral("Stage"),
            QStringLiteral("Status"),
            QStringLiteral("Detail"),
            QStringLiteral("Error Log"),
            QStringLiteral("Ready"),
            QStringLiteral("Running"),
            QStringLiteral("Done"),
            QStringLiteral("Failed"),
            QStringLiteral("Plan"),
            QStringLiteral("Select autostart entries to disable"),
            QStringLiteral("Select old container versions to clean"),
            QStringLiteral("No active autostart entries can be disabled, or all are already disabled."),
            QStringLiteral("No safely cleanable old container versions were found."),
            QStringLiteral("Error Log"),
            QStringLiteral("Live Status"),
            QStringLiteral("Updating"),
            QStringLiteral("Updated"),
            QStringLiteral("Refreshes every second while visible and skips overlapping scans")
        };
    }

    Text t() const { return language_->currentData().toString() == QStringLiteral("en") ? en() : zh(); }

    void buildUi()
    {
        setObjectName(QStringLiteral("AppRoot"));
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(16, 16, 16, 16);
        root->setSpacing(12);

        auto *headerFrame = new QFrame;
        headerFrame->setObjectName(QStringLiteral("HeaderFrame"));
        auto *header = new QHBoxLayout(headerFrame);
        header->setContentsMargins(18, 16, 18, 16);
        header->setSpacing(10);
        title_ = new QLabel;
        title_->setObjectName(QStringLiteral("Title"));
        QFont titleFont = title_->font();
        titleFont.setPointSize(titleFont.pointSize() + 5);
        titleFont.setBold(true);
        title_->setFont(titleFont);
        header->addWidget(title_, 1);
        header->addWidget(new SpaceVisual);

        languageLabel_ = new QLabel;
        language_ = new QComboBox;
        language_->addItem(QStringLiteral("中文"), QStringLiteral("zh"));
        language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
        connect(language_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CleanerWindow::applyLanguage);
        userLabel_ = new QLabel;
        userValue_ = new QLabel(user_);
        userValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        header->addWidget(languageLabel_);
        header->addWidget(language_);
        header->addSpacing(16);
        header->addWidget(userLabel_);
        header->addWidget(userValue_);
        root->addWidget(headerFrame);

        intro_ = new QLabel;
        intro_->setObjectName(QStringLiteral("Intro"));
        intro_->setWordWrap(true);
        root->addWidget(intro_);

        auto *statusFrame = new QFrame;
        statusFrame->setObjectName(QStringLiteral("StatusFrame"));
        auto *statusLayout = new QHBoxLayout(statusFrame);
        statusLayout->setContentsMargins(14, 10, 14, 10);
        statusLayout->setSpacing(12);
        statusTitle_ = new QLabel;
        statusTitle_->setObjectName(QStringLiteral("StatusTitle"));
        statusSummary_ = new QLabel;
        statusSummary_->setObjectName(QStringLiteral("StatusSummary"));
        statusSummary_->setWordWrap(true);
        lastUpdate_ = new QLabel;
        lastUpdate_->setObjectName(QStringLiteral("LastUpdate"));
        progress_ = new QProgressBar;
        progress_->setObjectName(QStringLiteral("Progress"));
        progress_->setTextVisible(false);
        progress_->setFixedWidth(140);
        progress_->setRange(0, 100);
        progress_->setValue(100);
        statusLayout->addWidget(statusTitle_);
        statusLayout->addWidget(statusSummary_, 1);
        statusLayout->addWidget(lastUpdate_);
        statusLayout->addWidget(progress_);
        root->addWidget(statusFrame);

        metrics_ = new QTableWidget(4, 4);
        metrics_->setObjectName(QStringLiteral("MetricsTable"));
        metrics_->verticalHeader()->hide();
        metrics_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        metrics_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        metrics_->setSelectionMode(QAbstractItemView::NoSelection);
        metrics_->setAlternatingRowColors(true);
        metrics_->setShowGrid(false);
        metrics_->verticalHeader()->setDefaultSectionSize(42);
        root->addWidget(metrics_);

        auto *actions = new QHBoxLayout;
        scanButton_ = new QPushButton;
        cleanOldButton_ = new QPushButton;
        autostartButton_ = new QPushButton;
        monitorButton_ = new QPushButton;
        scanButton_->setObjectName(QStringLiteral("PrimaryButton"));
        cleanOldButton_->setObjectName(QStringLiteral("ActionButton"));
        autostartButton_->setObjectName(QStringLiteral("ActionButton"));
        monitorButton_->setEnabled(false);
        actions->addWidget(scanButton_);
        actions->addWidget(cleanOldButton_);
        actions->addWidget(autostartButton_);
        actions->addWidget(monitorButton_);
        actions->addStretch(1);
        root->addLayout(actions);
        connect(scanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);
        connect(autostartButton_, &QPushButton::clicked, this, &CleanerWindow::showAutostartDialog);
        connect(cleanOldButton_, &QPushButton::clicked, this, &CleanerWindow::showContainerDialog);

        plan_ = new QTableWidget(0, 3);
        plan_->setObjectName(QStringLiteral("PlanTable"));
        plan_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        plan_->verticalHeader()->hide();
        plan_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        plan_->setSelectionMode(QAbstractItemView::NoSelection);
        plan_->setAlternatingRowColors(true);
        plan_->setShowGrid(false);
        plan_->verticalHeader()->setDefaultSectionSize(38);
        root->addWidget(plan_, 1);

        detailsBox_ = new QGroupBox;
        detailsBox_->setCheckable(true);
        detailsBox_->setChecked(false);
        auto *detailsLayout = new QVBoxLayout(detailsBox_);
        log_ = new QPlainTextEdit;
        log_->setReadOnly(true);
        log_->setMaximumBlockCount(3000);
        detailsLayout->addWidget(log_);
        root->addWidget(detailsBox_, 1);

        setStyleSheet(QStringLiteral(R"(
            QWidget#AppRoot { background: #f4f7fb; color: #1f2937; }
            QFrame#HeaderFrame {
                border-radius: 8px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #195ca8, stop:0.55 #2377b8, stop:1 #2d9c8f);
            }
            QLabel#Title { color: white; letter-spacing: 0px; }
            QFrame#HeaderFrame QLabel { color: white; }
            QLabel#Intro { color: #4b5563; }
            QFrame#StatusFrame {
                border: 1px solid #d8e2ee;
                border-radius: 8px;
                background: white;
            }
            QLabel#StatusTitle { color: #195ca8; font-weight: 700; }
            QLabel#StatusSummary { color: #111827; font-weight: 600; }
            QLabel#LastUpdate { color: #6b7280; }
            QProgressBar#Progress {
                border: 1px solid #cbd5e1;
                border-radius: 6px;
                background: #eef2f7;
                min-height: 10px;
                max-height: 10px;
            }
            QProgressBar#Progress::chunk {
                border-radius: 6px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2d9c8f, stop:1 #2f80ed);
            }
            QTableWidget#MetricsTable, QTableWidget#PlanTable {
                border: 1px solid #d8e2ee;
                border-radius: 8px;
                background: white;
                alternate-background-color: #f8fbff;
                selection-background-color: #dbeafe;
            }
            QHeaderView::section {
                background: #eaf1f8;
                color: #334155;
                border: 0;
                padding: 8px;
                font-weight: 700;
            }
            QPushButton {
                min-height: 34px;
                padding: 0 16px;
                border-radius: 7px;
                border: 1px solid #bfd0e0;
                background: white;
                color: #1f2937;
                font-weight: 600;
            }
            QPushButton:hover { background: #f0f7ff; border-color: #8bb8e8; }
            QPushButton:pressed { background: #dbeafe; }
            QPushButton#PrimaryButton {
                color: white;
                border: 1px solid #1f6fbf;
                background: #2377b8;
            }
            QPushButton#PrimaryButton:hover { background: #2f80ed; }
            QPushButton:disabled {
                color: #9ca3af;
                background: #edf1f5;
                border-color: #d1d5db;
            }
            QGroupBox {
                border: 1px solid #d8e2ee;
                border-radius: 8px;
                margin-top: 10px;
                background: white;
                color: #334155;
                font-weight: 700;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 12px;
                padding: 0 4px;
            }
            QPlainTextEdit {
                border: 0;
                background: #0f172a;
                color: #dbeafe;
                selection-background-color: #2563eb;
            }
        )"));
    }

    void applyLanguage()
    {
        const Text text = t();
        setWindowTitle(text.title);
        title_->setText(text.title);
        languageLabel_->setText(text.language);
        userLabel_->setText(text.user);
        intro_->setText(text.intro);
        statusTitle_->setText(text.live);
        scanButton_->setText(text.scan);
        cleanOldButton_->setText(text.cleanOld);
        autostartButton_->setText(text.disableAutostart);
        monitorButton_->setText(text.installMonitor);
        metrics_->setHorizontalHeaderLabels({text.metric, text.before, text.released, text.current});
        plan_->setHorizontalHeaderLabels({text.stage, text.status, text.detail});
        detailsBox_->setTitle(text.details);
        updateStatusSummary();

        const QStringList names{text.rootUsed, text.kaiming, text.ostree, text.kare};
        for (int row = 0; row < names.size(); ++row) {
            ensureItem(metrics_, row, 0)->setText(names.at(row));
            for (int col = 1; col < 4; ++col) {
                if (!metrics_->item(row, col)) {
                    metrics_->setItem(row, col, new QTableWidgetItem(QStringLiteral("-")));
                }
            }
        }
    }

    static QTableWidgetItem *ensureItem(QTableWidget *table, int row, int col)
    {
        if (!table->item(row, col)) {
            table->setItem(row, col, new QTableWidgetItem);
        }
        return table->item(row, col);
    }

    static qint64 jsonInt64(const QJsonObject &object, const QString &key)
    {
        bool ok = false;
        const qint64 value = object.value(key).toString().toLongLong(&ok);
        return ok ? value : static_cast<qint64>(object.value(key).toDouble(0));
    }

    static QString fmtBytes(qint64 bytes)
    {
        const QStringList units{QStringLiteral("B"), QStringLiteral("KiB"), QStringLiteral("MiB"), QStringLiteral("GiB"), QStringLiteral("TiB")};
        double size = qMax<qint64>(bytes, 0);
        for (const QString &unit : units) {
            if (size < 1024.0 || unit == units.last()) {
                return unit == QStringLiteral("B") ? QString::number(static_cast<qint64>(size)) + QLatin1Char(' ') + unit
                                                   : QString::number(size, 'f', 1) + QLatin1Char(' ') + unit;
            }
            size /= 1024.0;
        }
        return QStringLiteral("0 B");
    }

    void addPlanRow(const QString &stage, const QString &status, const QString &detail)
    {
        const int row = plan_->rowCount();
        plan_->insertRow(row);
        plan_->setItem(row, 0, new QTableWidgetItem(stage));
        plan_->setItem(row, 1, new QTableWidgetItem(status));
        plan_->setItem(row, 2, new QTableWidgetItem(detail));
        plan_->scrollToBottom();
    }

    void setBusy(bool busy)
    {
        busy_ = busy;
        scanButton_->setEnabled(!busy);
        cleanOldButton_->setEnabled(!busy);
        autostartButton_->setEnabled(!busy);
        if (busy) {
            progress_->setRange(0, 0);
            lastUpdate_->setText(t().updating);
        } else {
            progress_->setRange(0, 100);
            progress_->setValue(100);
        }
    }

    void scanManual()
    {
        scanInternal(true);
    }

    void setupWatchers()
    {
        watcher_ = new QFileSystemWatcher(this);
        const QStringList paths{
            QStringLiteral("/var/opt/kaiming"),
            QStringLiteral("/var/opt/kaiming/info"),
            QStringLiteral("/var/opt/kaiming/layers"),
            QStringLiteral("/var/opt/kaiming/layers/stable"),
            QStringLiteral("/sysroot/ostree/pkgs"),
            QStringLiteral("/opt/kare-applications"),
            QDir::homePath() + QStringLiteral("/.config/autostart")
        };
        for (const QString &path : paths) {
            if (QFileInfo(path).exists()) {
                watcher_->addPath(path);
            }
        }
        connect(watcher_, &QFileSystemWatcher::directoryChanged, this, &CleanerWindow::scheduleRefresh);
        connect(watcher_, &QFileSystemWatcher::fileChanged, this, &CleanerWindow::scheduleRefresh);
    }

    void scheduleRefresh()
    {
        if (!busy_ && isVisible()) {
            debounceTimer_->start();
        }
    }

    void refreshScan()
    {
        if (!busy_ && isVisible()) {
            scanInternal(false);
        }
    }

    void scanInternal(bool manual)
    {
        const Text text = t();
        setBusy(true);
        if (manual) {
            plan_->setRowCount(0);
            addPlanRow(text.planned, text.running, language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Read space usage, old Kaiming containers, and autostart entries.")
                : QStringLiteral("读取空间占用、旧 Kaiming 容器和自启动项。"));
        }
        runProcess({helper_, QStringLiteral("--scan"), QStringLiteral("--user"), user_}, [this, manual](int code, const QByteArray &output) {
            const Text text = t();
            if (code != 0) {
                log_->setPlainText(QString::fromLocal8Bit(output));
                addPlanRow(text.rawLog, text.failed, QString::fromLocal8Bit(output).left(240));
                setBusy(false);
                return;
            }
            const QJsonDocument doc = QJsonDocument::fromJson(output);
            if (!doc.isObject()) {
                log_->setPlainText(QString::fromLocal8Bit(output));
                addPlanRow(text.rawLog, text.failed, QStringLiteral("invalid helper JSON"));
                setBusy(false);
                return;
            }
            log_->clear();
            state_ = doc.object();
            updateMetrics();
            updateStatusSummary();
            if (manual) {
                addPlanRow(text.scan, text.done, scanSummary());
            }
            animateRefresh();
            setBusy(false);
        });
    }

    QString scanSummary() const
    {
        const int containers = state_.value(QStringLiteral("oldContainers")).toArray().size();
        int activeAutostarts = 0;
        for (const QJsonValue &value : state_.value(QStringLiteral("autostarts")).toArray()) {
            if (!value.toObject().value(QStringLiteral("disabled")).toBool()) {
                ++activeAutostarts;
            }
        }
        if (language_->currentData().toString() == QStringLiteral("en")) {
            return QStringLiteral("%1 old containers, %2 active autostart entries.").arg(containers).arg(activeAutostarts);
        }
        return QStringLiteral("发现 %1 个旧容器候选，%2 个仍启用的自启动项。").arg(containers).arg(activeAutostarts);
    }

    void updateMetrics()
    {
        const QJsonObject metrics = state_.value(QStringLiteral("metrics")).toObject();
        const QStringList keys{QStringLiteral("root_used"), QStringLiteral("kaiming"), QStringLiteral("ostree_upper"), QStringLiteral("kare_upper")};
        for (int row = 0; row < keys.size(); ++row) {
            const qint64 value = jsonInt64(metrics, keys.at(row));
            ensureItem(metrics_, row, 1)->setText(fmtBytes(value));
            ensureItem(metrics_, row, 2)->setText(QStringLiteral("-"));
            ensureItem(metrics_, row, 3)->setText(fmtBytes(value));
        }
    }

    void updateStatusSummary()
    {
        if (!statusSummary_) {
            return;
        }
        if (state_.isEmpty()) {
            statusSummary_->setText(t().autoRefresh);
            return;
        }
        statusSummary_->setText(scanSummary());
        const QString time = state_.value(QStringLiteral("time")).toString();
        const QDateTime parsed = QDateTime::fromString(time, Qt::ISODate);
        const QString stamp = parsed.isValid() ? parsed.toLocalTime().toString(QStringLiteral("HH:mm:ss"))
                                               : QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"));
        lastUpdate_->setText(t().updated + QStringLiteral(" ") + stamp);
    }

    void animateRefresh()
    {
        auto *effect = new QGraphicsOpacityEffect(metrics_);
        metrics_->setGraphicsEffect(effect);
        auto *animation = new QPropertyAnimation(effect, "opacity", metrics_);
        animation->setDuration(260);
        animation->setStartValue(0.55);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            metrics_->setGraphicsEffect(nullptr);
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void showAutostartDialog()
    {
        const QJsonArray entries = state_.value(QStringLiteral("autostarts")).toArray();
        QVector<QPair<QString, QCheckBox *>> boxes;
        QDialog dialog(this);
        dialog.setWindowTitle(t().selectAutostarts);
        auto *layout = new QVBoxLayout(&dialog);
        auto *scroll = new QScrollArea;
        auto *container = new QWidget;
        auto *list = new QVBoxLayout(container);

        for (const QJsonValue &value : entries) {
            const QJsonObject item = value.toObject();
            if (item.value(QStringLiteral("disabled")).toBool()) {
                continue;
            }
            auto *box = new QCheckBox(localName(item));
            box->setChecked(true);
            box->setToolTip(localDescription(item));
            list->addWidget(box);
            auto *detail = new QLabel(localDescription(item) + QStringLiteral("\n") + item.value(QStringLiteral("target")).toString());
            detail->setWordWrap(true);
            detail->setContentsMargins(26, 0, 0, 8);
            list->addWidget(detail);
            boxes.append({item.value(QStringLiteral("id")).toString(), box});
        }
        list->addStretch(1);
        scroll->setWidget(container);
        scroll->setWidgetResizable(true);
        scroll->setMinimumSize(640, 300);
        layout->addWidget(scroll);
        if (boxes.isEmpty()) {
            layout->addWidget(new QLabel(t().noAutostarts));
        }
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!boxes.isEmpty());
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        QStringList selected;
        for (const auto &pair : boxes) {
            if (pair.second->isChecked()) {
                selected << pair.first;
            }
        }
        if (selected.isEmpty()) {
            return;
        }
        applyAutostart(selected);
    }

    void showContainerDialog()
    {
        const QJsonArray entries = state_.value(QStringLiteral("oldContainers")).toArray();
        QVector<QPair<QString, QCheckBox *>> boxes;
        QDialog dialog(this);
        dialog.setWindowTitle(t().selectContainers);
        auto *layout = new QVBoxLayout(&dialog);
        auto *scroll = new QScrollArea;
        auto *container = new QWidget;
        auto *list = new QVBoxLayout(container);
        for (const QJsonValue &value : entries) {
            const QJsonObject item = value.toObject();
            auto *box = new QCheckBox(QStringLiteral("%1  %2  %3").arg(item.value(QStringLiteral("ref")).toString(),
                                                                       item.value(QStringLiteral("version")).toString(),
                                                                       fmtBytes(jsonInt64(item, QStringLiteral("bytes")))));
            box->setChecked(!item.value(QStringLiteral("inUse")).toBool());
            box->setEnabled(!item.value(QStringLiteral("inUse")).toBool());
            list->addWidget(box);
            auto *detail = new QLabel(item.value(QStringLiteral("path")).toString());
            detail->setWordWrap(true);
            detail->setContentsMargins(26, 0, 0, 8);
            list->addWidget(detail);
            boxes.append({item.value(QStringLiteral("path")).toString(), box});
        }
        list->addStretch(1);
        scroll->setWidget(container);
        scroll->setWidgetResizable(true);
        scroll->setMinimumSize(720, 340);
        layout->addWidget(scroll);
        if (boxes.isEmpty()) {
            layout->addWidget(new QLabel(t().noContainers));
        }
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!boxes.isEmpty());
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        QStringList selected;
        for (const auto &pair : boxes) {
            if (pair.second->isChecked()) {
                selected << pair.first;
            }
        }
        if (!selected.isEmpty()) {
            applyOldContainers(selected);
        }
    }

    QString localName(const QJsonObject &item) const
    {
        return language_->currentData().toString() == QStringLiteral("en")
            ? item.value(QStringLiteral("nameEn")).toString()
            : item.value(QStringLiteral("nameZh")).toString();
    }

    QString localDescription(const QJsonObject &item) const
    {
        return language_->currentData().toString() == QStringLiteral("en")
            ? item.value(QStringLiteral("descriptionEn")).toString()
            : item.value(QStringLiteral("descriptionZh")).toString();
    }

    void applyAutostart(const QStringList &ids)
    {
        const Text text = t();
        setBusy(true);
        addPlanRow(text.planned, text.running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Write Hidden=true overrides for %1 selected entries.").arg(ids.size())
            : QStringLiteral("为 %1 个选中的自启动项写入 Hidden=true 覆盖。").arg(ids.size()));
        runProcess({helper_, QStringLiteral("--apply-autostart"), QStringLiteral("--user"), user_, QStringLiteral("--entries"), ids.join(QLatin1Char(','))},
                   [this](int code, const QByteArray &output) {
            handleActionResult(code, output);
            scanInternal(false);
        });
    }

    void applyOldContainers(const QStringList &paths)
    {
        QStringList command{QStringLiteral("pkexec"), helper_, QStringLiteral("--apply-old-containers")};
        for (const QString &path : paths) {
            command << QStringLiteral("--container") << path;
        }
        setBusy(true);
        addPlanRow(t().planned, t().running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Move %1 old containers to DATA rollback quarantine.").arg(paths.size())
            : QStringLiteral("移动 %1 个旧容器到 DATA 回滚隔离区。").arg(paths.size()));
        runProcess(command, [this](int code, const QByteArray &output) {
            handleActionResult(code, output);
            scanInternal(false);
        });
    }

    void handleActionResult(int code, const QByteArray &output)
    {
        const Text text = t();
        const QJsonDocument doc = QJsonDocument::fromJson(output);
        if (code != 0 || !doc.isObject()) {
            log_->setPlainText(QString::fromLocal8Bit(output));
            addPlanRow(text.rawLog, text.failed, QString::fromLocal8Bit(output).left(240));
            setBusy(false);
            return;
        }
        log_->clear();
        int okCount = 0;
        int failCount = 0;
        qint64 released = 0;
        for (const QJsonValue &value : doc.object().value(QStringLiteral("results")).toArray()) {
            const QJsonObject item = value.toObject();
            if (item.value(QStringLiteral("ok")).toBool()) {
                ++okCount;
                released += jsonInt64(item, QStringLiteral("bytes"));
            } else {
                ++failCount;
            }
        }
        const QString summary = language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("%1 succeeded, %2 failed, released %3.").arg(okCount).arg(failCount).arg(fmtBytes(released))
            : QStringLiteral("%1 项成功，%2 项失败，释放 %3。").arg(okCount).arg(failCount).arg(fmtBytes(released));
        addPlanRow(text.planned, failCount == 0 ? text.done : text.failed, summary);
        setBusy(false);
    }

    using ProcessCallback = std::function<void(int, QByteArray)>;

    void runProcess(const QStringList &command, ProcessCallback callback)
    {
        if (command.isEmpty()) {
            return;
        }
        auto *process = new QProcess(this);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                [process, callback](int code, QProcess::ExitStatus) {
                    const QByteArray output = process->readAllStandardOutput() + process->readAllStandardError();
                    process->deleteLater();
                    callback(code, output);
                });
        const QString program = command.first();
        const QStringList args = command.mid(1);
        process->start(program, args);
    }

    QString user_;
    QString helper_;
    QJsonObject state_;
    bool busy_ = false;
    QLabel *title_ = nullptr;
    QLabel *languageLabel_ = nullptr;
    QLabel *userLabel_ = nullptr;
    QLabel *userValue_ = nullptr;
    QLabel *intro_ = nullptr;
    QLabel *statusTitle_ = nullptr;
    QLabel *statusSummary_ = nullptr;
    QLabel *lastUpdate_ = nullptr;
    QComboBox *language_ = nullptr;
    QProgressBar *progress_ = nullptr;
    QTableWidget *metrics_ = nullptr;
    QTableWidget *plan_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
    QPushButton *monitorButton_ = nullptr;
    QGroupBox *detailsBox_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
    QTimer *refreshTimer_ = nullptr;
    QTimer *debounceTimer_ = nullptr;
    QFileSystemWatcher *watcher_ = nullptr;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    CleanerWindow window;
    window.show();
    return app.exec();
}

#include "space_cleaner.moc"
