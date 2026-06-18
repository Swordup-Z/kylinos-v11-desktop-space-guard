#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTimer>

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

        setMinimumSize(980, 680);
        buildUi();
        applyLanguage();
        QTimer::singleShot(100, this, &CleanerWindow::scan);
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
            QStringLiteral("详细日志"),
            QStringLiteral("就绪"),
            QStringLiteral("执行中"),
            QStringLiteral("完成"),
            QStringLiteral("失败"),
            QStringLiteral("计划"),
            QStringLiteral("选择要禁用的自启动项"),
            QStringLiteral("选择要清理的旧版本容器"),
            QStringLiteral("当前没有可禁用的自启动项，或都已经禁用。"),
            QStringLiteral("当前没有发现可安全清理的旧版本容器。"),
            QStringLiteral("原始输出")
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
            QStringLiteral("Details"),
            QStringLiteral("Ready"),
            QStringLiteral("Running"),
            QStringLiteral("Done"),
            QStringLiteral("Failed"),
            QStringLiteral("Plan"),
            QStringLiteral("Select autostart entries to disable"),
            QStringLiteral("Select old container versions to clean"),
            QStringLiteral("No active autostart entries can be disabled, or all are already disabled."),
            QStringLiteral("No safely cleanable old container versions were found."),
            QStringLiteral("Raw Output")
        };
    }

    Text t() const { return language_->currentData().toString() == QStringLiteral("en") ? en() : zh(); }

    void buildUi()
    {
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(16, 16, 16, 16);
        root->setSpacing(12);

        auto *header = new QHBoxLayout;
        title_ = new QLabel;
        QFont titleFont = title_->font();
        titleFont.setPointSize(titleFont.pointSize() + 5);
        titleFont.setBold(true);
        title_->setFont(titleFont);
        header->addWidget(title_, 1);

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
        root->addLayout(header);

        intro_ = new QLabel;
        intro_->setWordWrap(true);
        root->addWidget(intro_);

        metrics_ = new QTableWidget(4, 4);
        metrics_->verticalHeader()->hide();
        metrics_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        metrics_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        metrics_->setSelectionMode(QAbstractItemView::NoSelection);
        root->addWidget(metrics_);

        auto *actions = new QHBoxLayout;
        scanButton_ = new QPushButton;
        cleanOldButton_ = new QPushButton;
        autostartButton_ = new QPushButton;
        monitorButton_ = new QPushButton;
        monitorButton_->setEnabled(false);
        actions->addWidget(scanButton_);
        actions->addWidget(cleanOldButton_);
        actions->addWidget(autostartButton_);
        actions->addWidget(monitorButton_);
        actions->addStretch(1);
        root->addLayout(actions);
        connect(scanButton_, &QPushButton::clicked, this, &CleanerWindow::scan);
        connect(autostartButton_, &QPushButton::clicked, this, &CleanerWindow::showAutostartDialog);
        connect(cleanOldButton_, &QPushButton::clicked, this, &CleanerWindow::showContainerDialog);

        plan_ = new QTableWidget(0, 3);
        plan_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        plan_->verticalHeader()->hide();
        plan_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        plan_->setSelectionMode(QAbstractItemView::NoSelection);
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
    }

    void applyLanguage()
    {
        const Text text = t();
        setWindowTitle(text.title);
        title_->setText(text.title);
        languageLabel_->setText(text.language);
        userLabel_->setText(text.user);
        intro_->setText(text.intro);
        scanButton_->setText(text.scan);
        cleanOldButton_->setText(text.cleanOld);
        autostartButton_->setText(text.disableAutostart);
        monitorButton_->setText(text.installMonitor);
        metrics_->setHorizontalHeaderLabels({text.metric, text.before, text.released, text.current});
        plan_->setHorizontalHeaderLabels({text.stage, text.status, text.detail});
        detailsBox_->setTitle(text.details);

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
        scanButton_->setEnabled(!busy);
        cleanOldButton_->setEnabled(!busy);
        autostartButton_->setEnabled(!busy);
    }

    void scan()
    {
        const Text text = t();
        setBusy(true);
        plan_->setRowCount(0);
        addPlanRow(text.planned, text.running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Read space usage, old Kaiming containers, and autostart entries.")
            : QStringLiteral("读取空间占用、旧 Kaiming 容器和自启动项。"));
        runProcess({helper_, QStringLiteral("--scan"), QStringLiteral("--user"), user_}, [this](int code, const QByteArray &output) {
            const Text text = t();
            log_->setPlainText(QString::fromLocal8Bit(output));
            if (code != 0) {
                addPlanRow(text.rawLog, text.failed, QString::fromLocal8Bit(output).left(240));
                setBusy(false);
                return;
            }
            const QJsonDocument doc = QJsonDocument::fromJson(output);
            if (!doc.isObject()) {
                addPlanRow(text.rawLog, text.failed, QStringLiteral("invalid helper JSON"));
                setBusy(false);
                return;
            }
            state_ = doc.object();
            updateMetrics();
            addPlanRow(text.rawLog, text.done, scanSummary());
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
            scan();
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
            scan();
        });
    }

    void handleActionResult(int code, const QByteArray &output)
    {
        const Text text = t();
        log_->setPlainText(QString::fromLocal8Bit(output));
        const QJsonDocument doc = QJsonDocument::fromJson(output);
        if (code != 0 || !doc.isObject()) {
            addPlanRow(text.rawLog, text.failed, QString::fromLocal8Bit(output).left(240));
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
        addPlanRow(text.rawLog, failCount == 0 ? text.done : text.failed, summary);
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
    QLabel *title_ = nullptr;
    QLabel *languageLabel_ = nullptr;
    QLabel *userLabel_ = nullptr;
    QLabel *userValue_ = nullptr;
    QLabel *intro_ = nullptr;
    QComboBox *language_ = nullptr;
    QTableWidget *metrics_ = nullptr;
    QTableWidget *plan_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
    QPushButton *monitorButton_ = nullptr;
    QGroupBox *detailsBox_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    CleanerWindow window;
    window.show();
    return app.exec();
}

#include "space_cleaner.moc"

