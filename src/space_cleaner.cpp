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
#include <QMouseEvent>
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
        bg.setColorAt(0.00, QColor(255, 255, 255, 210));
        bg.setColorAt(0.54, QColor(247, 249, 252, 178));
        bg.setColorAt(1.00, QColor(233, 238, 244, 132));
        painter.setPen(QPen(QColor(255, 255, 255, 168), 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(card, 8, 8);

        QRadialGradient wash(QPointF(58, 58), 62);
        wash.setColorAt(0.00, QColor(18, 22, 30, 42));
        wash.setColorAt(0.58, QColor(72, 82, 95, 18));
        wash.setColorAt(1.00, QColor(18, 22, 30, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(wash);
        painter.drawEllipse(QPointF(58, 58), 62, 38);

        QRadialGradient mistWash(QPointF(155, 60), 58);
        mistWash.setColorAt(0.00, QColor(98, 118, 138, 38 + static_cast<int>(26 * activity_)));
        mistWash.setColorAt(0.66, QColor(98, 118, 138, 12));
        mistWash.setColorAt(1.00, QColor(98, 118, 138, 0));
        painter.setBrush(mistWash);
        painter.drawEllipse(QPointF(155, 60), 58, 38);

        const QColor ink(28, 31, 36);
        const QColor graphite(66, 72, 82);
        const QColor mist(103, 121, 139);
        const QColor silver(157, 168, 181);

        for (int i = 0; i < 3; ++i) {
            const QRectF layer(26 + i * 7, 31 + i * 16, 62, 14);
            painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 90 - i * 18), 1));
            painter.setBrush(QColor(255, 255, 255, 138 - i * 14));
            painter.drawRoundedRect(layer, 7, 7);
            painter.setPen(QPen(QColor(graphite.red(), graphite.green(), graphite.blue(), 54), 1));
            painter.drawLine(layer.left() + 11, layer.center().y(), layer.right() - 12, layer.center().y());
        }

        QPainterPath flow;
        flow.moveTo(83, 58);
        flow.cubicTo(105, 30, 132, 86, 154, 58);
        flow.cubicTo(166, 43, 178, 47, 188, 57);
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(), 58 + static_cast<int>(86 * activity_)),
                            2.4 + activity_ * 1.1,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawPath(flow);

        for (int i = 0; i < 6; ++i) {
            const qreal t = std::fmod(phase_ + i * 0.17, 1.0);
            const qreal x = 86 + t * 98;
            const qreal y = 58 + std::sin((t * 2.0 + phase_) * 3.14159265359) * 12;
            const int alpha = 42 + static_cast<int>(118 * activity_);
            const QColor dot = i % 2 ? silver : ink;
            painter.setBrush(QColor(dot.red(), dot.green(), dot.blue(), alpha));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(x, y), 1.8 + activity_ * 1.6, 1.8 + activity_ * 1.6);
        }

        const QPointF ringCenter(163, 59);
        const QRectF outer(ringCenter.x() - 30, ringCenter.y() - 30, 60, 60);
        const qreal quietPulse = 0.5 + 0.5 * std::sin(phase_ * 6.28318530718);
        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 44), 7, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(outer, 40 * 16, 285 * 16);
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(),
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
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(), 126), 1.4));
        const qreal x = 116 + std::fmod(phase_ * 96, 74.0);
        painter.drawLine(QPointF(x, 94), QPointF(x + 18, 94));
    }

private:
    qreal phase_ = 0.0;
    qreal activity_ = 0.0;
    QPropertyAnimation *activityAnimation_ = nullptr;
};

static QColor mixedColor(const QColor &a, const QColor &b, qreal t)
{
    t = qBound<qreal>(0.0, t, 1.0);
    return QColor(static_cast<int>(a.red() + (b.red() - a.red()) * t),
                  static_cast<int>(a.green() + (b.green() - a.green()) * t),
                  static_cast<int>(a.blue() + (b.blue() - a.blue()) * t),
                  static_cast<int>(a.alpha() + (b.alpha() - a.alpha()) * t));
}

class CardFrame : public QFrame {
    Q_OBJECT
    Q_PROPERTY(qreal hover READ hover WRITE setHover)

public:
    explicit CardFrame(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setAttribute(Qt::WA_Hover, true);
        setAutoFillBackground(false);
        hoverAnimation_ = new QPropertyAnimation(this, "hover", this);
        hoverAnimation_->setDuration(180);
        hoverAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal hover() const { return hover_; }

    void setHover(qreal hover)
    {
        hover_ = hover;
        update();
    }

    void setInteractive(bool interactive)
    {
        interactive_ = interactive;
        if (!interactive_) {
            animateTo(0.0);
        }
    }

protected:
    void enterEvent(QEvent *event) override
    {
        if (interactive_) {
            animateTo(1.0);
        }
        QFrame::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        if (interactive_) {
            animateTo(0.0);
        }
        QFrame::leaveEvent(event);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF rect = QRectF(0.5, 0.5, width() - 1, height() - 1);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(28, 31, 36, 10 + static_cast<int>(10 * hover_)));
        painter.drawRoundedRect(rect.adjusted(1, 3 + hover_, -1, -1), 8, 8);

        const QColor background = mixedColor(QColor(255, 255, 255), QColor(248, 251, 253), hover_);
        const QColor border = mixedColor(QColor(217, 223, 231), QColor(155, 173, 190), hover_);
        painter.setBrush(background);
        painter.setPen(QPen(border, 1));
        painter.drawRoundedRect(rect.adjusted(0, 0, 0, -2), 8, 8);
    }

private:
    void animateTo(qreal target)
    {
        hoverAnimation_->stop();
        hoverAnimation_->setStartValue(hover_);
        hoverAnimation_->setEndValue(target);
        hoverAnimation_->start();
    }

    qreal hover_ = 0.0;
    bool interactive_ = true;
    QPropertyAnimation *hoverAnimation_ = nullptr;
};

class AnimatedButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal hover READ hover WRITE setHover)
    Q_PROPERTY(qreal press READ press WRITE setPress)

public:
    explicit AnimatedButton(QWidget *parent = nullptr)
        : QPushButton(parent)
    {
        setMinimumHeight(36);
        setCursor(Qt::PointingHandCursor);
        hoverAnimation_ = new QPropertyAnimation(this, "hover", this);
        hoverAnimation_->setDuration(170);
        hoverAnimation_->setEasingCurve(QEasingCurve::OutCubic);
        pressAnimation_ = new QPropertyAnimation(this, "press", this);
        pressAnimation_->setDuration(120);
        pressAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal hover() const { return hover_; }
    qreal press() const { return press_; }

    void setHover(qreal hover)
    {
        hover_ = hover;
        update();
    }

    void setPress(qreal press)
    {
        press_ = press;
        update();
    }

protected:
    void enterEvent(QEvent *event) override
    {
        animate(hoverAnimation_, hover_, 1.0);
        QPushButton::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        animate(hoverAnimation_, hover_, 0.0);
        animate(pressAnimation_, press_, 0.0);
        QPushButton::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        animate(pressAnimation_, press_, 1.0);
        QPushButton::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        animate(pressAnimation_, press_, 0.0);
        QPushButton::mouseReleaseEvent(event);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const bool primary = objectName() == QStringLiteral("PrimaryButton");
        const bool chrome = objectName() == QStringLiteral("ChromeButton") || objectName() == QStringLiteral("CloseButton");
        const bool close = objectName() == QStringLiteral("CloseButton");
        QColor base = primary ? QColor(29, 29, 31) : QColor(255, 255, 255);
        QColor hoverColor = primary ? QColor(55, 61, 70) : QColor(241, 245, 249);
        QColor pressColor = primary ? QColor(18, 22, 30) : QColor(227, 234, 242);
        QColor border = primary ? QColor(29, 29, 31) : QColor(203, 211, 221);
        QColor textColor = primary ? QColor(255, 255, 255) : QColor(29, 29, 31);
        if (chrome) {
            base = QColor(255, 255, 255, 0);
            hoverColor = close ? QColor(255, 69, 58, 34) : QColor(29, 31, 36, 18);
            pressColor = close ? QColor(255, 69, 58, 58) : QColor(29, 31, 36, 30);
            border = QColor(255, 255, 255, 0);
            textColor = close ? mixedColor(QColor(67, 72, 81), QColor(190, 52, 48), hover_)
                              : QColor(67, 72, 81);
        }

        if (!isEnabled()) {
            base = QColor(238, 242, 246);
            hoverColor = base;
            pressColor = base;
            border = QColor(217, 223, 231);
            textColor = QColor(164, 169, 177);
        }

        QColor fill = mixedColor(mixedColor(base, hoverColor, hover_), pressColor, press_);
        border = chrome ? border : mixedColor(border, QColor(111, 130, 150), hover_);

        const QRectF rect = QRectF(0.5, 0.5 + press_ * 1.0, width() - 1, height() - 1 - press_ * 1.0);
        if (!chrome) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(28, 31, 36, static_cast<int>(14 * hover_)));
            painter.drawRoundedRect(rect.adjusted(0, 2, 0, 2), 7, 7);
        }
        painter.setPen(QPen(border, 1));
        painter.setBrush(fill);
        painter.drawRoundedRect(rect, 7, 7);

        painter.setPen(textColor);
        painter.setFont(font());
        const qreal padding = chrome ? 0.0 : 14.0;
        painter.drawText(rect.adjusted(padding, 0, -padding, 0), Qt::AlignCenter, text());
    }

private:
    void animate(QPropertyAnimation *animation, qreal start, qreal target)
    {
        animation->stop();
        animation->setStartValue(start);
        animation->setEndValue(target);
        animation->start();
    }

    qreal hover_ = 0.0;
    qreal press_ = 0.0;
    QPropertyAnimation *hoverAnimation_ = nullptr;
    QPropertyAnimation *pressAnimation_ = nullptr;
};

class CleanerWindow : public QWidget {
    Q_OBJECT

public:
    CleanerWindow()
    {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
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
        QString manageAutostart;
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
        QString metricsTitle;
        QString actionsTitle;
        QString resultTitle;
        QString ok;
        QString cancel;
        QString pendingScan;
        QString scanTime;
        QString cleanable;
        QString metricStatus;
        QString normal;
        QString noCleanable;
        QString active;
        QString disabled;
        QString applicationsTitle;
        QString appName;
        QString appKind;
        QString appContainers;
        QString appSize;
        QString containerDetails;
        QString module;
        QString version;
        QString path;
        QString currentLayer;
        QString inUse;
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
            QStringLiteral("管理预热/自启动"),
            QStringLiteral("安装监控"),
            QStringLiteral("项目"),
            QStringLiteral("当前占用"),
            QStringLiteral("可清理"),
            QStringLiteral("状态"),
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
            QStringLiteral("管理预热/自启动项"),
            QStringLiteral("选择要清理的旧版本容器"),
            QStringLiteral("当前没有发现可管理的预热/自启动项。"),
            QStringLiteral("当前没有发现可安全清理的旧版本容器。"),
            QStringLiteral("错误"),
            QStringLiteral("当前状态"),
            QStringLiteral("正在更新"),
            QStringLiteral("扫描时间"),
            QStringLiteral("启动时自动扫描一次；后续可手动重新扫描"),
            QStringLiteral("操作失败"),
            QStringLiteral("操作未完成。错误详情已写入："),
            QStringLiteral("空间占用"),
            QStringLiteral("可执行操作"),
            QStringLiteral("执行结果"),
            QStringLiteral("确定"),
            QStringLiteral("取消"),
            QStringLiteral("等待扫描"),
            QStringLiteral("扫描时间"),
            QStringLiteral("可清理"),
            QStringLiteral("状态"),
            QStringLiteral("正常"),
            QStringLiteral("暂无安全清理项"),
            QStringLiteral("启用"),
            QStringLiteral("禁用"),
            QStringLiteral("应用容器"),
            QStringLiteral("应用"),
            QStringLiteral("类型"),
            QStringLiteral("容器数"),
            QStringLiteral("总占用"),
            QStringLiteral("容器明细"),
            QStringLiteral("模块"),
            QStringLiteral("版本"),
            QStringLiteral("路径"),
            QStringLiteral("当前引用"),
            QStringLiteral("使用中")
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
            QStringLiteral("Manage Preheat/Autostart"),
            QStringLiteral("Install Monitor"),
            QStringLiteral("Item"),
            QStringLiteral("Current Usage"),
            QStringLiteral("Cleanable"),
            QStringLiteral("Status"),
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
            QStringLiteral("Manage preheat/autostart entries"),
            QStringLiteral("Select old container versions to clean"),
            QStringLiteral("No manageable preheat/autostart entries were found."),
            QStringLiteral("No safely cleanable old container versions were found."),
            QStringLiteral("Error"),
            QStringLiteral("Current Status"),
            QStringLiteral("Updating"),
            QStringLiteral("Scan Time"),
            QStringLiteral("Scans once at startup. Use Scan to refresh manually."),
            QStringLiteral("Operation Failed"),
            QStringLiteral("The operation did not complete. Error details were written to:"),
            QStringLiteral("Space Usage"),
            QStringLiteral("Actions"),
            QStringLiteral("Results"),
            QStringLiteral("OK"),
            QStringLiteral("Cancel"),
            QStringLiteral("Waiting for scan"),
            QStringLiteral("Scan Time"),
            QStringLiteral("Cleanable"),
            QStringLiteral("Status"),
            QStringLiteral("Normal"),
            QStringLiteral("No safe cleanup candidates"),
            QStringLiteral("Enabled"),
            QStringLiteral("Disabled"),
            QStringLiteral("Application Containers"),
            QStringLiteral("Application"),
            QStringLiteral("Type"),
            QStringLiteral("Containers"),
            QStringLiteral("Total Size"),
            QStringLiteral("Container Details"),
            QStringLiteral("Module"),
            QStringLiteral("Version"),
            QStringLiteral("Path"),
            QStringLiteral("Current"),
            QStringLiteral("In Use")
        };
    }

    Text t() const { return language_->currentData().toString() == QStringLiteral("en") ? en() : zh(); }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != chrome_) {
            return QWidget::eventFilter(watched, event);
        }
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                draggingWindow_ = true;
                dragOffset_ = mouse->globalPos() - frameGeometry().topLeft();
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove && draggingWindow_) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->buttons() & Qt::LeftButton) {
                if (isMaximized()) {
                    showNormal();
                }
                move(mouse->globalPos() - dragOffset_);
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            draggingWindow_ = false;
            return true;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            isMaximized() ? showNormal() : showMaximized();
            return true;
        }
        return QWidget::eventFilter(watched, event);
    }

    void buildUi()
    {
        setObjectName(QStringLiteral("AppRoot"));
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(14, 10, 14, 14);
        root->setSpacing(12);

        chrome_ = new QFrame;
        chrome_->setObjectName(QStringLiteral("ChromeBar"));
        chrome_->installEventFilter(this);
        auto *chromeLayout = new QHBoxLayout(chrome_);
        chromeLayout->setContentsMargins(8, 0, 8, 0);
        chromeLayout->setSpacing(8);
        chromeTitle_ = new QLabel;
        chromeTitle_->setObjectName(QStringLiteral("ChromeTitle"));
        chromeLayout->addWidget(chromeTitle_, 1);
        minimizeButton_ = new AnimatedButton;
        maximizeButton_ = new AnimatedButton;
        closeButton_ = new AnimatedButton;
        minimizeButton_->setObjectName(QStringLiteral("ChromeButton"));
        maximizeButton_->setObjectName(QStringLiteral("ChromeButton"));
        closeButton_->setObjectName(QStringLiteral("CloseButton"));
        minimizeButton_->setText(QStringLiteral("−"));
        maximizeButton_->setText(QStringLiteral("□"));
        closeButton_->setText(QStringLiteral("×"));
        const QSize chromeButtonSize(32, 28);
        minimizeButton_->setFixedSize(chromeButtonSize);
        maximizeButton_->setFixedSize(chromeButtonSize);
        closeButton_->setFixedSize(chromeButtonSize);
        connect(minimizeButton_, &QPushButton::clicked, this, &QWidget::showMinimized);
        connect(maximizeButton_, &QPushButton::clicked, this, [this]() {
            isMaximized() ? showNormal() : showMaximized();
        });
        connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
        chromeLayout->addWidget(minimizeButton_);
        chromeLayout->addWidget(maximizeButton_);
        chromeLayout->addWidget(closeButton_);
        root->addWidget(chrome_);

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
        auto *visualCard = new CardFrame;
        visualCard->setObjectName(QStringLiteral("VisualCard"));
        visualCard->setInteractive(false);
        auto *visualLayout = new QVBoxLayout(visualCard);
        visualLayout->setContentsMargins(10, 10, 10, 10);
        visual_ = new SpaceVisual;
        visualLayout->addWidget(visual_);
        body->addLayout(copy, 1);
        body->addWidget(visualCard, 0, Qt::AlignRight | Qt::AlignVCenter);
        header->addLayout(body);
        root->addWidget(headerFrame);

        auto *metricsCard = new CardFrame;
        metricsCard->setObjectName(QStringLiteral("CardFrame"));
        metricsCard->setInteractive(false);
        auto *metricsLayout = new QVBoxLayout(metricsCard);
        metricsLayout->setContentsMargins(16, 14, 16, 16);
        metricsLayout->setSpacing(10);
        metricsTitle_ = new QLabel;
        metricsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        metricsLayout->addWidget(metricsTitle_);
        metrics_ = new QTableWidget(4, 4);
        metrics_->setObjectName(QStringLiteral("MetricsTable"));
        metrics_->verticalHeader()->hide();
        metrics_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        metrics_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        metrics_->setSelectionMode(QAbstractItemView::NoSelection);
        metrics_->setAlternatingRowColors(true);
        metrics_->setShowGrid(false);
        metrics_->verticalHeader()->setDefaultSectionSize(42);
        metricsLayout->addWidget(metrics_);
        root->addWidget(metricsCard);

        auto *appsCard = new CardFrame;
        appsCard->setObjectName(QStringLiteral("CardFrame"));
        appsCard->setInteractive(false);
        auto *appsLayout = new QVBoxLayout(appsCard);
        appsLayout->setContentsMargins(16, 14, 16, 16);
        appsLayout->setSpacing(10);
        appsTitle_ = new QLabel;
        appsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        appsLayout->addWidget(appsTitle_);
        apps_ = new QTableWidget(0, 4);
        apps_->setObjectName(QStringLiteral("AppsTable"));
        apps_->verticalHeader()->hide();
        apps_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        apps_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        apps_->setSelectionBehavior(QAbstractItemView::SelectRows);
        apps_->setSelectionMode(QAbstractItemView::SingleSelection);
        apps_->setAlternatingRowColors(true);
        apps_->setShowGrid(false);
        apps_->verticalHeader()->setDefaultSectionSize(36);
        apps_->setMinimumHeight(150);
        appsLayout->addWidget(apps_);
        root->addWidget(appsCard);
        connect(apps_, &QTableWidget::cellClicked, this, [this](int row, int) {
            showApplicationContainers(row);
        });

        auto *actionsCard = new CardFrame;
        actionsCard->setObjectName(QStringLiteral("CardFrame"));
        actionsCard->setInteractive(false);
        auto *actionsCardLayout = new QVBoxLayout(actionsCard);
        actionsCardLayout->setContentsMargins(16, 14, 16, 16);
        actionsCardLayout->setSpacing(10);
        actionsTitle_ = new QLabel;
        actionsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        actionsCardLayout->addWidget(actionsTitle_);
        auto *actions = new QHBoxLayout;
        actions->setSpacing(10);
        scanButton_ = new AnimatedButton;
        cleanOldButton_ = new AnimatedButton;
        autostartButton_ = new AnimatedButton;
        monitorButton_ = new AnimatedButton;
        scanButton_->setObjectName(QStringLiteral("PrimaryButton"));
        cleanOldButton_->setObjectName(QStringLiteral("ActionButton"));
        autostartButton_->setObjectName(QStringLiteral("ActionButton"));
        monitorButton_->setEnabled(false);
        actions->addWidget(scanButton_);
        actions->addWidget(cleanOldButton_);
        actions->addWidget(autostartButton_);
        actions->addWidget(monitorButton_);
        actions->addStretch(1);
        actionsCardLayout->addLayout(actions);
        root->addWidget(actionsCard);
        connect(scanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);
        connect(autostartButton_, &QPushButton::clicked, this, &CleanerWindow::showAutostartDialog);
        connect(cleanOldButton_, &QPushButton::clicked, this, &CleanerWindow::showContainerDialog);

        auto *planCard = new CardFrame;
        planCard->setObjectName(QStringLiteral("CardFrame"));
        planCard->setInteractive(false);
        auto *planLayout = new QVBoxLayout(planCard);
        planLayout->setContentsMargins(16, 14, 16, 16);
        planLayout->setSpacing(10);
        resultTitle_ = new QLabel;
        resultTitle_->setObjectName(QStringLiteral("SectionTitle"));
        planLayout->addWidget(resultTitle_);
        plan_ = new QTableWidget(0, 3);
        plan_->setObjectName(QStringLiteral("PlanTable"));
        plan_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        plan_->verticalHeader()->hide();
        plan_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        plan_->setSelectionMode(QAbstractItemView::NoSelection);
        plan_->setAlternatingRowColors(true);
        plan_->setShowGrid(false);
        plan_->verticalHeader()->setDefaultSectionSize(38);
        planLayout->addWidget(plan_, 1);
        root->addWidget(planCard, 1);

        setStyleSheet(QStringLiteral(R"(
            QWidget#AppRoot { background: #f5f7fa; color: #1d1d1f; }
            QFrame#ChromeBar { background: transparent; }
            QLabel#ChromeTitle { color: #555b64; font-weight: 600; }
            QFrame#HeaderFrame {
                border-radius: 8px;
                border: 1px solid #d9dfe7;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #ffffff, stop:0.56 #f5f7fa, stop:1 #edf2f7);
            }
            QLabel#Title { color: #1d1d1f; letter-spacing: 0px; }
            QFrame#HeaderFrame QLabel { color: #3a3d42; }
            QLabel#Intro { color: #63666d; font-size: 14px; line-height: 150%; }
            QFrame#StatusFrame {
                border: 1px solid #d9dfe7;
                border-radius: 8px;
                background: #ffffff;
            }
            QLabel#StatusTitle { color: #1d1d1f; font-weight: 700; }
            QLabel#StatusSummary { color: #24262b; font-weight: 600; }
            QLabel#LastUpdate { color: #737780; }
            QLabel#SectionTitle {
                color: #1d1d1f;
                font-size: 15px;
                font-weight: 700;
            }
            QProgressBar#Progress {
                border: 1px solid #cbd3dd;
                border-radius: 6px;
                background: #eef2f6;
                min-height: 10px;
                max-height: 10px;
            }
            QProgressBar#Progress::chunk {
                border-radius: 6px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #1d1d1f, stop:1 #66798b);
            }
            QTableWidget#MetricsTable, QTableWidget#AppsTable, QTableWidget#PlanTable {
                border: 1px solid #d9dfe7;
                border-radius: 8px;
                background: #ffffff;
                alternate-background-color: #f7f9fb;
                selection-background-color: #dce5ee;
            }
            QHeaderView::section {
                background: #edf1f5;
                color: #343941;
                border: 0;
                padding: 8px;
                font-weight: 700;
            }
            QPushButton {
                min-height: 34px;
                padding: 0 16px;
                border-radius: 7px;
                border: 1px solid #cbd3dd;
                background: #ffffff;
                color: #1d1d1f;
                font-weight: 600;
            }
            QPushButton:hover { background: #f1f5f9; border-color: #9fb0c2; }
            QPushButton:pressed { background: #e3eaf2; }
            QPushButton#PrimaryButton {
                color: #ffffff;
                border: 1px solid #1d1d1f;
                background: #1d1d1f;
            }
            QPushButton#PrimaryButton:hover { background: #343941; }
            QPushButton:disabled {
                color: #a4a9b1;
                background: #eef2f6;
                border-color: #d9dfe7;
            }
        )"));
    }

    void applyLanguage()
    {
        const Text text = t();
        setWindowTitle(text.title);
        chromeTitle_->setText(text.title);
        title_->setText(text.title);
        languageLabel_->setText(text.language);
        userLabel_->setText(text.user);
        intro_->setText(text.intro);
        statusTitle_->setText(text.live);
        scanButton_->setText(text.scan);
        cleanOldButton_->setText(text.cleanOld);
        autostartButton_->setText(text.manageAutostart);
        monitorButton_->setText(text.installMonitor);
        metricsTitle_->setText(text.metricsTitle);
        appsTitle_->setText(text.applicationsTitle);
        actionsTitle_->setText(text.actionsTitle);
        resultTitle_->setText(text.resultTitle);
        metrics_->setHorizontalHeaderLabels({text.metric, text.current, text.cleanable, text.metricStatus});
        apps_->setHorizontalHeaderLabels({text.appName, text.appKind, text.appContainers, text.appSize});
        plan_->setHorizontalHeaderLabels({text.stage, text.status, text.detail});
        updateStatusSummary();

        const QStringList names{text.rootUsed, text.kaiming, text.ostree, text.kare};
        for (int row = 0; row < names.size(); ++row) {
            ensureItem(metrics_, row, 0)->setText(names.at(row));
            for (int col = 1; col < 4; ++col) {
                ensureItem(metrics_, row, col)->setText(state_.isEmpty() ? text.pendingScan : QStringLiteral("-"));
            }
        }
        if (!state_.isEmpty()) {
            updateMetrics();
            updateApplications();
        } else {
            apps_->setRowCount(0);
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
            updateApplications();
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
        const Text text = t();
        const QJsonObject metrics = state_.value(QStringLiteral("metrics")).toObject();
        const QStringList keys{QStringLiteral("root_used"), QStringLiteral("kaiming"), QStringLiteral("ostree_upper"), QStringLiteral("kare_upper")};
        const qint64 oldContainerBytes = oldContainersBytes();
        for (int row = 0; row < keys.size(); ++row) {
            const qint64 value = jsonInt64(metrics, keys.at(row));
            ensureItem(metrics_, row, 1)->setText(fmtBytes(value));
            ensureItem(metrics_, row, 2)->setText(row == 1 ? fmtBytes(oldContainerBytes) : QStringLiteral("-"));
            if (row == 1) {
                ensureItem(metrics_, row, 3)->setText(oldContainerBytes > 0 ? text.cleanable : text.noCleanable);
            } else {
                ensureItem(metrics_, row, 3)->setText(text.normal);
            }
        }
    }

    void updateApplications()
    {
        const QJsonArray applications = state_.value(QStringLiteral("applications")).toArray();
        apps_->setRowCount(applications.size());
        for (int row = 0; row < applications.size(); ++row) {
            const QJsonObject app = applications.at(row).toObject();
            ensureItem(apps_, row, 0)->setText(app.value(QStringLiteral("ref")).toString());
            ensureItem(apps_, row, 1)->setText(app.value(QStringLiteral("kind")).toString());
            ensureItem(apps_, row, 2)->setText(QString::number(app.value(QStringLiteral("containerCount")).toInt()));
            ensureItem(apps_, row, 3)->setText(fmtBytes(jsonInt64(app, QStringLiteral("bytes"))));
        }
    }

    void showApplicationContainers(int row)
    {
        const QJsonArray applications = state_.value(QStringLiteral("applications")).toArray();
        if (row < 0 || row >= applications.size()) {
            return;
        }
        const Text text = t();
        const QJsonObject app = applications.at(row).toObject();
        const QJsonArray containers = app.value(QStringLiteral("containers")).toArray();

        QDialog dialog(this);
        dialog.setStyleSheet(styleSheet());
        dialog.setWindowTitle(text.containerDetails + QStringLiteral(" - ") + app.value(QStringLiteral("ref")).toString());
        auto *layout = new QVBoxLayout(&dialog);
        auto *table = new QTableWidget(containers.size(), 6);
        table->setHorizontalHeaderLabels({text.module, text.version, text.appSize, text.currentLayer, text.inUse, text.path});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->hide();
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setAlternatingRowColors(true);
        table->setShowGrid(false);
        for (int i = 0; i < containers.size(); ++i) {
            const QJsonObject item = containers.at(i).toObject();
            table->setItem(i, 0, new QTableWidgetItem(item.value(QStringLiteral("module")).toString()));
            table->setItem(i, 1, new QTableWidgetItem(item.value(QStringLiteral("version")).toString()));
            table->setItem(i, 2, new QTableWidgetItem(fmtBytes(jsonInt64(item, QStringLiteral("bytes")))));
            table->setItem(i, 3, new QTableWidgetItem(item.value(QStringLiteral("current")).toBool() ? text.active : text.disabled));
            table->setItem(i, 4, new QTableWidgetItem(item.value(QStringLiteral("inUse")).toBool() ? text.active : text.disabled));
            table->setItem(i, 5, new QTableWidgetItem(item.value(QStringLiteral("path")).toString()));
        }
        table->setMinimumSize(840, 320);
        layout->addWidget(table);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
        localizeDialogButtons(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        layout->addWidget(buttons);
        dialog.exec();
    }

    qint64 oldContainersBytes() const
    {
        qint64 total = 0;
        for (const QJsonValue &value : state_.value(QStringLiteral("oldContainers")).toArray()) {
            total += jsonInt64(value.toObject(), QStringLiteral("bytes"));
        }
        return total;
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
        lastUpdate_->setText(t().scanTime + QStringLiteral(" ") + stamp);
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
        QVector<bool> initialEnabled;
        QDialog dialog(this);
        dialog.setStyleSheet(styleSheet());
        dialog.setWindowTitle(t().selectAutostarts);
        auto *layout = new QVBoxLayout(&dialog);
        auto *scroll = new QScrollArea;
        auto *container = new QWidget;
        auto *list = new QVBoxLayout(container);

        for (const QJsonValue &value : entries) {
            const QJsonObject item = value.toObject();
            const bool enabled = !item.value(QStringLiteral("disabled")).toBool();
            auto *box = new QCheckBox(localName(item));
            box->setChecked(enabled);
            box->setToolTip(localDescription(item));
            list->addWidget(box);
            auto *detail = new QLabel((enabled ? t().active : t().disabled) + QStringLiteral(" · ")
                                      + localDescription(item) + QStringLiteral("\n")
                                      + item.value(QStringLiteral("target")).toString());
            detail->setWordWrap(true);
            detail->setContentsMargins(26, 0, 0, 8);
            list->addWidget(detail);
            boxes.append({item.value(QStringLiteral("id")).toString(), box});
            initialEnabled.append(enabled);
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
        localizeDialogButtons(buttons);
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!boxes.isEmpty());
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        QStringList enableIds;
        QStringList disableIds;
        for (int i = 0; i < boxes.size(); ++i) {
            const bool enabled = boxes.at(i).second->isChecked();
            if (enabled == initialEnabled.at(i)) {
                continue;
            }
            if (enabled) {
                enableIds << boxes.at(i).first;
            } else {
                disableIds << boxes.at(i).first;
            }
        }
        if (enableIds.isEmpty() && disableIds.isEmpty()) {
            return;
        }
        applyAutostartChanges(disableIds, enableIds);
    }

    void showContainerDialog()
    {
        const QJsonArray entries = state_.value(QStringLiteral("oldContainers")).toArray();
        QVector<QPair<QString, QCheckBox *>> boxes;
        QDialog dialog(this);
        dialog.setStyleSheet(styleSheet());
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
        localizeDialogButtons(buttons);
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

    void applyAutostartChanges(const QStringList &disableIds, const QStringList &enableIds)
    {
        const Text text = t();
        setBusy(true);
        addPlanRow(text.planned, text.running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Apply %1 disable and %2 enable autostart changes.").arg(disableIds.size()).arg(enableIds.size())
            : QStringLiteral("应用 %1 个禁用、%2 个启用的自启动变更。").arg(disableIds.size()).arg(enableIds.size()));
        runProcess({helper_, QStringLiteral("--manage-autostart"), QStringLiteral("--user"), user_,
                    QStringLiteral("--disable-entries"), disableIds.join(QLatin1Char(',')),
                    QStringLiteral("--enable-entries"), enableIds.join(QLatin1Char(','))},
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
        QMessageBox box(QMessageBox::Warning,
                        t().errorTitle,
                        t().errorMessage + QStringLiteral("\n") + path,
                        QMessageBox::NoButton,
                        this);
        box.setStyleSheet(styleSheet());
        box.addButton(t().ok, QMessageBox::AcceptRole);
        box.exec();
    }

    void localizeDialogButtons(QDialogButtonBox *buttons)
    {
        if (auto *okButton = buttons->button(QDialogButtonBox::Ok)) {
            okButton->setText(t().ok);
        }
        if (auto *cancelButton = buttons->button(QDialogButtonBox::Cancel)) {
            cancelButton->setText(t().cancel);
        }
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
    bool draggingWindow_ = false;
    QPoint dragOffset_;
    QFrame *chrome_ = nullptr;
    QLabel *chromeTitle_ = nullptr;
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
    QLabel *metricsTitle_ = nullptr;
    QLabel *appsTitle_ = nullptr;
    QLabel *actionsTitle_ = nullptr;
    QLabel *resultTitle_ = nullptr;
    QTableWidget *metrics_ = nullptr;
    QTableWidget *apps_ = nullptr;
    QTableWidget *plan_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
    QPushButton *monitorButton_ = nullptr;
    QPushButton *minimizeButton_ = nullptr;
    QPushButton *maximizeButton_ = nullptr;
    QPushButton *closeButton_ = nullptr;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    CleanerWindow window;
    window.show();
    return app.exec();
}

#include "space_cleaner.moc"
