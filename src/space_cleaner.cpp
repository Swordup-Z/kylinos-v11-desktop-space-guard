#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
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
    Q_PROPERTY(qreal activity READ activity WRITE setActivity)

public:
    explicit SpaceVisual(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setFixedSize(210, 118);
        auto *animation = new QPropertyAnimation(this, "phase", this);
        animation->setDuration(5200);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setLoopCount(-1);
        animation->setEasingCurve(QEasingCurve::InOutSine);
        animation->start();

        activityAnimation_ = new QPropertyAnimation(this, "activity", this);
        activityAnimation_->setDuration(360);
        activityAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal phase() const { return phase_; }
    qreal activity() const { return activity_; }

    void setPhase(qreal phase)
    {
        phase_ = phase;
        update();
    }

    void setActivity(qreal activity)
    {
        activity_ = activity;
        update();
    }

    void setWorking(bool working)
    {
        activityAnimation_->stop();
        activityAnimation_->setStartValue(activity_);
        activityAnimation_->setEndValue(working ? 1.0 : 0.0);
        activityAnimation_->start();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF card(1, 1, width() - 2, height() - 2);
        QLinearGradient bg(card.topLeft(), card.bottomRight());
        bg.setColorAt(0.00, QColor(255, 255, 255, 178));
        bg.setColorAt(0.54, QColor(244, 241, 234, 132));
        bg.setColorAt(1.00, QColor(228, 224, 214, 92));
        painter.setPen(QPen(QColor(255, 255, 255, 135), 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(card, 8, 8);

        QRadialGradient wash(QPointF(58, 58), 62);
        wash.setColorAt(0.00, QColor(39, 44, 56, 34));
        wash.setColorAt(0.62, QColor(88, 83, 73, 16));
        wash.setColorAt(1.00, QColor(39, 44, 56, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(wash);
        painter.drawEllipse(QPointF(58, 58), 62, 38);

        QRadialGradient goldWash(QPointF(155, 60), 58);
        goldWash.setColorAt(0.00, QColor(158, 132, 72, 42 + static_cast<int>(24 * activity_)));
        goldWash.setColorAt(0.68, QColor(158, 132, 72, 12));
        goldWash.setColorAt(1.00, QColor(158, 132, 72, 0));
        painter.setBrush(goldWash);
        painter.drawEllipse(QPointF(155, 60), 58, 38);

        const QColor ink(43, 48, 59);
        const QColor stone(108, 105, 97);
        const QColor warmGold(159, 132, 73);

        for (int i = 0; i < 3; ++i) {
            const QRectF layer(26 + i * 7, 31 + i * 16, 62, 14);
            painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 90 - i * 18), 1));
            painter.setBrush(QColor(255, 255, 255, 118 - i * 12));
            painter.drawRoundedRect(layer, 7, 7);
            painter.setPen(QPen(QColor(stone.red(), stone.green(), stone.blue(), 54), 1));
            painter.drawLine(layer.left() + 11, layer.center().y(), layer.right() - 12, layer.center().y());
        }

        QPainterPath flow;
        flow.moveTo(83, 58);
        flow.cubicTo(105, 30, 132, 86, 154, 58);
        flow.cubicTo(166, 43, 178, 47, 188, 57);
        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 52 + static_cast<int>(78 * activity_)),
                            2.4 + activity_ * 1.1,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawPath(flow);

        for (int i = 0; i < 6; ++i) {
            const qreal t = std::fmod(phase_ + i * 0.17, 1.0);
            const qreal x = 86 + t * 98;
            const qreal y = 58 + std::sin((t * 2.0 + phase_) * 3.14159265359) * 12;
            const int alpha = 42 + static_cast<int>(118 * activity_);
            const QColor dot = i % 2 ? warmGold : ink;
            painter.setBrush(QColor(dot.red(), dot.green(), dot.blue(), alpha));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(x, y), 1.8 + activity_ * 1.6, 1.8 + activity_ * 1.6);
        }

        const QPointF ringCenter(163, 59);
        const QRectF outer(ringCenter.x() - 30, ringCenter.y() - 30, 60, 60);
        const qreal quietPulse = 0.5 + 0.5 * std::sin(phase_ * 6.28318530718);
        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 44), 7, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(outer, 40 * 16, 285 * 16);
        painter.setPen(QPen(QColor(warmGold.red(), warmGold.green(), warmGold.blue(),
                                   126 + static_cast<int>(70 * activity_)),
                            7,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawArc(outer,
                        static_cast<int>((84 + phase_ * 360) * 16),
                        static_cast<int>((88 + quietPulse * 24 + activity_ * 52) * 16));

        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 82), 1));
        painter.setBrush(QColor(255, 255, 255, 145));
        painter.drawEllipse(ringCenter, 14, 14);
        painter.setBrush(QColor(ink.red(), ink.green(), ink.blue(), 185));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(ringCenter, 4.5, 4.5);

        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 32), 1));
        painter.drawLine(QPointF(116, 94), QPointF(190, 94));
        painter.setPen(QPen(QColor(warmGold.red(), warmGold.green(), warmGold.blue(), 112), 1.4));
        const qreal x = 116 + std::fmod(phase_ * 96, 74.0);
        painter.drawLine(QPointF(x, 94), QPointF(x + 18, 94));
    }

private:
    qreal phase_ = 0.0;
    qreal activity_ = 0.0;
    QPropertyAnimation *activityAnimation_ = nullptr;
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
        QString errorTitle;
        QString errorMessage;
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
            QStringLiteral("错误"),
            QStringLiteral("就绪"),
            QStringLiteral("执行中"),
            QStringLiteral("完成"),
            QStringLiteral("失败"),
            QStringLiteral("计划"),
            QStringLiteral("选择要禁用的自启动项"),
            QStringLiteral("选择要清理的旧版本容器"),
            QStringLiteral("当前没有可禁用的自启动项，或都已经禁用。"),
            QStringLiteral("当前没有发现可安全清理的旧版本容器。"),
            QStringLiteral("错误"),
            QStringLiteral("实时状态"),
            QStringLiteral("正在更新"),
            QStringLiteral("已更新"),
            QStringLiteral("启动时自动扫描一次；后续可手动重新扫描"),
            QStringLiteral("操作失败"),
            QStringLiteral("操作未完成。错误详情已写入：")
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
            QStringLiteral("Error"),
            QStringLiteral("Ready"),
            QStringLiteral("Running"),
            QStringLiteral("Done"),
            QStringLiteral("Failed"),
            QStringLiteral("Plan"),
            QStringLiteral("Select autostart entries to disable"),
            QStringLiteral("Select old container versions to clean"),
            QStringLiteral("No active autostart entries can be disabled, or all are already disabled."),
            QStringLiteral("No safely cleanable old container versions were found."),
            QStringLiteral("Error"),
            QStringLiteral("Live Status"),
            QStringLiteral("Updating"),
            QStringLiteral("Updated"),
            QStringLiteral("Scans once at startup. Use Scan to refresh manually."),
            QStringLiteral("Operation Failed"),
            QStringLiteral("The operation did not complete. Error details were written to:")
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
        auto *header = new QVBoxLayout(headerFrame);
        header->setContentsMargins(22, 18, 22, 18);
        header->setSpacing(16);

        auto *topBar = new QHBoxLayout;
        topBar->setSpacing(10);
        title_ = new QLabel;
        title_->setObjectName(QStringLiteral("Title"));
        QFont titleFont = title_->font();
        titleFont.setPointSize(titleFont.pointSize() + 8);
        titleFont.setBold(true);
        title_->setFont(titleFont);
        topBar->addWidget(title_, 1);
        languageLabel_ = new QLabel;
        language_ = new QComboBox;
        language_->addItem(QStringLiteral("中文"), QStringLiteral("zh"));
        language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
        connect(language_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CleanerWindow::applyLanguage);
        userLabel_ = new QLabel;
        userValue_ = new QLabel(user_);
        userValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        topBar->addWidget(languageLabel_);
        topBar->addWidget(language_);
        topBar->addSpacing(12);
        topBar->addWidget(userLabel_);
        topBar->addWidget(userValue_);
        header->addLayout(topBar);

        auto *body = new QHBoxLayout;
        body->setSpacing(20);
        auto *copy = new QVBoxLayout;
        copy->setSpacing(14);
        intro_ = new QLabel;
        intro_->setObjectName(QStringLiteral("Intro"));
        intro_->setWordWrap(true);
        copy->addWidget(intro_);

        auto *statusFrame = new QFrame;
        statusFrame->setObjectName(QStringLiteral("StatusFrame"));
        auto *statusLayout = new QHBoxLayout(statusFrame);
        statusLayout->setContentsMargins(14, 12, 14, 12);
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
        copy->addWidget(statusFrame);
        copy->addStretch(1);
        visual_ = new SpaceVisual;
        body->addLayout(copy, 1);
        body->addWidget(visual_, 0, Qt::AlignRight | Qt::AlignVCenter);
        header->addLayout(body);
        root->addWidget(headerFrame);

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

        setStyleSheet(QStringLiteral(R"(
            QWidget#AppRoot { background: #f4f2ec; color: #262a31; }
            QFrame#HeaderFrame {
                border-radius: 8px;
                border: 1px solid #d8d1c2;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #fbfaf6, stop:0.56 #efe9dc, stop:1 #e8ece8);
            }
            QLabel#Title { color: #252a31; letter-spacing: 0px; }
            QFrame#HeaderFrame QLabel { color: #42464d; }
            QLabel#Intro { color: #5d5f5c; font-size: 14px; line-height: 150%; }
            QFrame#StatusFrame {
                border: 1px solid #d8d1c2;
                border-radius: 8px;
                background: rgba(255, 255, 255, 165);
            }
            QLabel#StatusTitle { color: #2f3440; font-weight: 700; }
            QLabel#StatusSummary { color: #252a31; font-weight: 600; }
            QLabel#LastUpdate { color: #77736a; }
            QProgressBar#Progress {
                border: 1px solid #cdc4b2;
                border-radius: 6px;
                background: #ece7dc;
                min-height: 10px;
                max-height: 10px;
            }
            QProgressBar#Progress::chunk {
                border-radius: 6px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #303642, stop:1 #9f8449);
            }
            QTableWidget#MetricsTable, QTableWidget#PlanTable {
                border: 1px solid #d8d1c2;
                border-radius: 8px;
                background: #fffefa;
                alternate-background-color: #f7f3eb;
                selection-background-color: #ded6c7;
            }
            QHeaderView::section {
                background: #e8e1d3;
                color: #383d45;
                border: 0;
                padding: 8px;
                font-weight: 700;
            }
            QPushButton {
                min-height: 34px;
                padding: 0 16px;
                border-radius: 7px;
                border: 1px solid #c9c0ae;
                background: #fffefa;
                color: #252a31;
                font-weight: 600;
            }
            QPushButton:hover { background: #f6f0e4; border-color: #a99468; }
            QPushButton:pressed { background: #e8dfd0; }
            QPushButton#PrimaryButton {
                color: #fffefa;
                border: 1px solid #2d323c;
                background: #303642;
            }
            QPushButton#PrimaryButton:hover { background: #444a55; }
            QPushButton:disabled {
                color: #a6a098;
                background: #ece7dc;
                border-color: #d6d0c3;
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
        if (visual_) {
            visual_->setWorking(busy);
        }
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
                const QString path = writeErrorLog(QStringLiteral("scan helper failed"), output);
                addPlanRow(text.rawLog, text.failed, language_->currentData().toString() == QStringLiteral("en")
                    ? QStringLiteral("Scan failed. See the error dialog for the log path.")
                    : QStringLiteral("扫描失败。错误日志位置见弹窗。"));
                showErrorDialog(path);
                setBusy(false);
                return;
            }
            const QJsonDocument doc = QJsonDocument::fromJson(output);
            if (!doc.isObject()) {
                const QString path = writeErrorLog(QStringLiteral("invalid helper JSON"), output);
                addPlanRow(text.rawLog, text.failed, language_->currentData().toString() == QStringLiteral("en")
                    ? QStringLiteral("Scan returned invalid data. See the error dialog for the log path.")
                    : QStringLiteral("扫描返回数据无效。错误日志位置见弹窗。"));
                showErrorDialog(path);
                setBusy(false);
                return;
            }
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
        });
    }

    void handleActionResult(int code, const QByteArray &output)
    {
        const Text text = t();
        const QJsonDocument doc = QJsonDocument::fromJson(output);
        if (code != 0 || !doc.isObject()) {
            const QString path = writeErrorLog(QStringLiteral("apply action failed"), output);
            addPlanRow(text.rawLog, text.failed, language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Action failed. See the error dialog for the log path.")
                : QStringLiteral("操作失败。错误日志位置见弹窗。"));
            showErrorDialog(path);
            setBusy(false);
            return;
        }
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
        if (failCount > 0) {
            const QString path = writeErrorLog(QStringLiteral("partial action failure"), output);
            showErrorDialog(path);
        }
        addPlanRow(text.planned, failCount == 0 ? text.done : text.failed, summary);
        setBusy(false);
    }

    QString errorLogPath() const
    {
        return QDir::homePath() + QStringLiteral("/.local/state/kylin-space-guard/error.log");
    }

    QString writeErrorLog(const QString &context, const QByteArray &output) const
    {
        const QString path = errorLogPath();
        QFileInfo info(path);
        QDir().mkpath(info.absolutePath());
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            file.write("==== ");
            file.write(QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8());
            file.write(" ");
            file.write(context.toUtf8());
            file.write(" ====\n");
            file.write(output);
            if (!output.endsWith('\n')) {
                file.write("\n");
            }
            file.write("\n");
        }
        return path;
    }

    void showErrorDialog(const QString &path)
    {
        QMessageBox::warning(this, t().errorTitle, t().errorMessage + QStringLiteral("\n") + path);
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
    SpaceVisual *visual_ = nullptr;
    QTableWidget *metrics_ = nullptr;
    QTableWidget *plan_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
    QPushButton *monitorButton_ = nullptr;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    CleanerWindow window;
    window.show();
    return app.exec();
}

#include "space_cleaner.moc"
