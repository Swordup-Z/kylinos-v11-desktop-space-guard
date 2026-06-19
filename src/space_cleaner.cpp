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
#include <QIcon>
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
#include <QStyle>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <cmath>

QT_CHARTS_USE_NAMESPACE

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

        QColor background = mixedColor(QColor(255, 255, 255), QColor(248, 251, 253), hover_);
        QColor border = mixedColor(QColor(217, 223, 231), QColor(155, 173, 190), hover_);
        if (objectName() == QStringLiteral("TotalMetricRow")) {
            background = mixedColor(QColor(248, 250, 252), QColor(239, 244, 248), hover_);
            border = mixedColor(QColor(132, 146, 160), QColor(82, 96, 110), hover_);
        } else if (objectName() == QStringLiteral("ChildMetricRow")) {
            background = mixedColor(QColor(255, 255, 255), QColor(250, 252, 253), hover_);
            border = mixedColor(QColor(224, 229, 235), QColor(170, 184, 198), hover_);
        }
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

        const bool primary = objectName() == QStringLiteral("PrimaryButton") || objectName() == QStringLiteral("NavSelected");
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

        const qreal padding = chrome ? 0.0 : 14.0;
        QRectF textRect = rect.adjusted(padding, 0, -padding, 0);
        painter.setPen(textColor);
        painter.setFont(font());
        if (!chrome && !icon().isNull()) {
            const QSize iconExtent = iconSize().isValid() ? iconSize() : QSize(18, 18);
            const int spacing = text().isEmpty() ? 0 : 8;
            const int textWidth = fontMetrics().horizontalAdvance(text());
            const int contentWidth = iconExtent.width() + spacing + textWidth;
            const int startX = static_cast<int>(rect.center().x() - contentWidth / 2.0);
            const QRect iconRect(startX,
                                 static_cast<int>(rect.center().y() - iconExtent.height() / 2.0),
                                 iconExtent.width(),
                                 iconExtent.height());
            icon().paint(&painter, iconRect, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
            textRect = QRectF(iconRect.right() + 1 + spacing,
                              rect.top(),
                              qMax(0.0, rect.right() - iconRect.right() - spacing - padding),
                              rect.height());
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
        } else {
            painter.drawText(textRect, Qt::AlignCenter, text());
        }
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

class ClickableCardFrame : public CardFrame {
    Q_OBJECT

public:
    explicit ClickableCardFrame(QWidget *parent = nullptr)
        : CardFrame(parent)
    {
        setCursor(Qt::PointingHandCursor);
    }

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
            emit clicked();
        }
        CardFrame::mouseReleaseEvent(event);
    }
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

        setMinimumSize(1180, 860);
        buildUi();
        applyLanguage();
        if (tabs_) {
            tabs_->setCurrentIndex(0);
        }
        QTimer::singleShot(100, this, [this]() {
            scanInternal(false);
        });
    }

private:
    struct Text {
        QString title;
        QString language;
        QString user;
        QString statusTab;
        QString scanTab;
        QString intro;
        QString scan;
        QString startScan;
        QString cleanOld;
        QString manageAutostart;
        QString scanProgressTitle;
        QString scanProgressDetail;
        QString scanResultTitle;
        QString optimizeSelected;
        QString rescan;
        QString optimizationItems;
        QString containerCleanup;
        QString autostartOptimization;
        QString metric;
        QString before;
        QString released;
        QString current;
        QString rootUsed;
        QString kaiming;
        QString ostree;
        QString kare;
        QString otherRoot;
        QString metricsRelation;
        QString overallTitle;
        QString explainedUsage;
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
        QString back;
        QString pendingScan;
        QString scanTime;
        QString cleanable;
        QString metricStatus;
        QString shareOfRoot;
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
        QString appSummaryTitle;
        QString appTotal;
        QString totalContainers;
        QString appContainersUsage;
        QString kaimingRelation;
    };

    static Text zh()
    {
        return {
            QStringLiteral("麒麟V11空间清理"),
            QStringLiteral("语言："),
            QStringLiteral("用户："),
            QStringLiteral("总览"),
            QStringLiteral("扫描"),
            QString(),
            QStringLiteral("扫描"),
            QStringLiteral("开始扫描"),
            QStringLiteral("清理旧容器"),
            QStringLiteral("管理预热/自启动"),
            QStringLiteral("正在扫描"),
            QStringLiteral("正在读取系统空间、Kaiming 容器和启动项状态"),
            QStringLiteral("扫描结果"),
            QStringLiteral("执行选中优化"),
            QStringLiteral("重新扫描"),
            QStringLiteral("可优化条目"),
            QStringLiteral("旧版本容器清理"),
            QStringLiteral("预热/自启动优化"),
            QStringLiteral("项目"),
            QStringLiteral("当前占用"),
            QStringLiteral("可清理"),
            QStringLiteral("状态"),
            QStringLiteral("根分区已用"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree 写入层"),
            QStringLiteral("KARE 写入层"),
            QStringLiteral("其他根分区占用"),
            QStringLiteral("根分区已用是总量；下面几项是已识别的子项或重点写入层，其他根分区占用包含系统基线、KARE base、普通应用目录、日志和缓存等未单独拆出的内容。"),
            QStringLiteral("整体空间占用"),
            QStringLiteral("已识别子项"),
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
            QStringLiteral("返回"),
            QStringLiteral("等待扫描"),
            QStringLiteral("扫描时间"),
            QStringLiteral("可清理"),
            QStringLiteral("状态"),
            QStringLiteral("占根分区"),
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
            QStringLiteral("使用中"),
            QStringLiteral("应用容器统计"),
            QStringLiteral("应用数"),
            QStringLiteral("容器数"),
            QStringLiteral("容器合计"),
            QStringLiteral("应用容器是 Kaiming 占用的应用级明细；Kaiming 还可能包含运行时、缓存和管理数据，因此两者不会总是相等。")
        };
    }

    static Text en()
    {
        return {
            QStringLiteral("KylinOS V11 Desktop Space Cleaner"),
            QStringLiteral("Language:"),
            QStringLiteral("User:"),
            QStringLiteral("Overview"),
            QStringLiteral("Scan"),
            QString(),
            QStringLiteral("Scan"),
            QStringLiteral("Start Scan"),
            QStringLiteral("Clean Old Containers"),
            QStringLiteral("Manage Preheat/Autostart"),
            QStringLiteral("Scanning"),
            QStringLiteral("Reading system usage, Kaiming containers, and startup entries"),
            QStringLiteral("Scan Results"),
            QStringLiteral("Optimize Selected"),
            QStringLiteral("Rescan"),
            QStringLiteral("Optimization Items"),
            QStringLiteral("Old Container Cleanup"),
            QStringLiteral("Preheat/Autostart Optimization"),
            QStringLiteral("Item"),
            QStringLiteral("Current Usage"),
            QStringLiteral("Cleanable"),
            QStringLiteral("Status"),
            QStringLiteral("Root Used"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree Upper"),
            QStringLiteral("KARE Upper"),
            QStringLiteral("Other Root Usage"),
            QStringLiteral("Root usage is the total. The rows below are recognized child categories or key writable layers. Other root usage includes the system baseline, KARE base, ordinary app directories, logs, caches, and anything not broken out separately."),
            QStringLiteral("Overall Space Usage"),
            QStringLiteral("Recognized Items"),
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
            QStringLiteral("Back"),
            QStringLiteral("Waiting for scan"),
            QStringLiteral("Scan Time"),
            QStringLiteral("Cleanable"),
            QStringLiteral("Status"),
            QStringLiteral("Share of Root"),
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
            QStringLiteral("In Use"),
            QStringLiteral("Application Container Summary"),
            QStringLiteral("Applications"),
            QStringLiteral("Containers"),
            QStringLiteral("Container Total"),
            QStringLiteral("Application containers are the app-level breakdown inside Kaiming usage. Kaiming may also include runtimes, caches, indexes, and management data, so the totals do not always match.")
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
        auto *windowRoot = new QVBoxLayout(this);
        windowRoot->setContentsMargins(14, 10, 14, 14);
        windowRoot->setSpacing(12);

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
        windowRoot->addWidget(chrome_);

        auto *content = new QWidget;
        content->setObjectName(QStringLiteral("ContentPane"));
        auto *root = new QVBoxLayout(content);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(12);
        windowRoot->addWidget(content, 1);

        auto *headerFrame = new QFrame;
        headerFrame->setObjectName(QStringLiteral("HeaderFrame"));
        auto *header = new QVBoxLayout(headerFrame);
        header->setContentsMargins(22, 14, 22, 14);
        header->setSpacing(0);

        auto *topBar = new QHBoxLayout;
        topBar->setSpacing(10);
        title_ = new QLabel;
        title_->setObjectName(QStringLiteral("Title"));
        title_->setWordWrap(true);
        title_->setMinimumWidth(260);
        QFont titleFont = title_->font();
        titleFont.setPointSize(titleFont.pointSize() + 8);
        titleFont.setBold(true);
        title_->setFont(titleFont);
        topBar->addWidget(title_, 1);
        languageLabel_ = new QLabel;
        language_ = new QComboBox;
        language_->addItem(QStringLiteral("中文"), QStringLiteral("zh"));
        language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
        language_->setMinimumWidth(116);
        language_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
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
        copy->setContentsMargins(0, 0, 0, 0);
        copy->setSpacing(0);
        intro_ = new QLabel;
        intro_->setObjectName(QStringLiteral("Intro"));
        intro_->setWordWrap(true);
        intro_->setVisible(false);
        copy->addWidget(intro_);

        heroScanButton_ = new AnimatedButton;
        heroScanButton_->setObjectName(QStringLiteral("PrimaryButton"));
        heroScanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        heroScanButton_->setIconSize(QSize(18, 18));
        heroScanButton_->setMinimumWidth(142);
        heroScanButton_->setVisible(false);
        connect(heroScanButton_, &QPushButton::clicked, this, [this]() {
            if (tabs_) {
                tabs_->setCurrentIndex(1);
            }
            scanManual();
        });
        copy->addWidget(heroScanButton_, 0, Qt::AlignLeft);

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
        lastUpdate_->setMinimumWidth(128);
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
        statusFrame->setVisible(false);
        copy->addWidget(statusFrame);
        copy->addStretch(1);
        auto *visualCard = new CardFrame;
        visualCard->setObjectName(QStringLiteral("VisualCard"));
        visualCard->setInteractive(false);
        visualCard->setVisible(false);
        auto *visualLayout = new QVBoxLayout(visualCard);
        visualLayout->setContentsMargins(10, 10, 10, 10);
        visual_ = new SpaceVisual;
        visualLayout->addWidget(visual_);
        body->addLayout(copy, 1);
        body->addWidget(visualCard, 0, Qt::AlignRight | Qt::AlignVCenter);
        header->addLayout(body);
        root->addWidget(headerFrame);

        auto *navFrame = new QFrame;
        navFrame->setObjectName(QStringLiteral("NavFrame"));
        auto *navLayout = new QHBoxLayout(navFrame);
        navLayout->setContentsMargins(8, 8, 8, 8);
        navLayout->setSpacing(10);
        statusNavButton_ = new AnimatedButton;
        scanNavButton_ = new AnimatedButton;
        statusNavButton_->setObjectName(QStringLiteral("NavSelected"));
        scanNavButton_->setObjectName(QStringLiteral("NavButton"));
        statusNavButton_->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
        scanNavButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        statusNavButton_->setIconSize(QSize(18, 18));
        scanNavButton_->setIconSize(QSize(18, 18));
        statusNavButton_->setMinimumWidth(152);
        scanNavButton_->setMinimumWidth(152);
        navLayout->addWidget(statusNavButton_);
        navLayout->addWidget(scanNavButton_);
        navLayout->addStretch(1);
        root->addWidget(navFrame);

        tabs_ = new QTabWidget;
        tabs_->setObjectName(QStringLiteral("MainTabs"));
        tabs_->setDocumentMode(true);
        tabs_->setIconSize(QSize(18, 18));
        tabs_->tabBar()->hide();
        connect(statusNavButton_, &QPushButton::clicked, this, [this]() {
            if (tabs_) {
                tabs_->setCurrentIndex(0);
            }
        });
        connect(scanNavButton_, &QPushButton::clicked, this, [this]() {
            if (tabs_) {
                tabs_->setCurrentIndex(1);
            }
        });
        connect(tabs_, &QTabWidget::currentChanged, this, &CleanerWindow::updateMainNav);

        auto *statusPage = new QWidget;
        statusPage->setObjectName(QStringLiteral("TabPage"));
        auto *statusPageLayout = new QVBoxLayout(statusPage);
        statusPageLayout->setContentsMargins(0, 12, 0, 0);
        statusPageLayout->setSpacing(12);

        auto *statusSwitcher = new QHBoxLayout;
        statusSwitcher->setSpacing(10);
        metricsPageButton_ = new AnimatedButton;
        appsPageButton_ = new AnimatedButton;
        metricsPageButton_->setObjectName(QStringLiteral("PrimaryButton"));
        appsPageButton_->setObjectName(QStringLiteral("ActionButton"));
        metricsPageButton_->setIcon(style()->standardIcon(QStyle::SP_DriveHDIcon));
        appsPageButton_->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
        statusSwitcher->addWidget(metricsPageButton_);
        statusSwitcher->addWidget(appsPageButton_);
        statusSwitcher->addStretch(1);
        statusPageLayout->addLayout(statusSwitcher);

        statusStack_ = new QStackedWidget;
        statusStack_->setObjectName(QStringLiteral("StatusStack"));
        statusPageLayout->addWidget(statusStack_, 1);

        auto *metricsPage = new QWidget;
        metricsPage->setObjectName(QStringLiteral("TabPage"));
        auto *metricsPageLayout = new QVBoxLayout(metricsPage);
        metricsPageLayout->setContentsMargins(0, 0, 0, 0);
        metricsPageLayout->setSpacing(12);

        auto *appsPage = new QWidget;
        appsPage->setObjectName(QStringLiteral("TabPage"));
        auto *appsPageLayout = new QVBoxLayout(appsPage);
        appsPageLayout->setContentsMargins(0, 0, 0, 0);
        appsPageLayout->setSpacing(12);

        statusStack_->addWidget(metricsPage);
        statusStack_->addWidget(appsPage);
        connect(metricsPageButton_, &QPushButton::clicked, this, [this]() {
            showStatusSubPage(0);
        });
        connect(appsPageButton_, &QPushButton::clicked, this, [this]() {
            showStatusSubPage(1);
        });

        auto *scanPage = new QWidget;
        scanPage->setObjectName(QStringLiteral("TabPage"));
        auto *scanPageLayout = new QVBoxLayout(scanPage);
        scanPageLayout->setContentsMargins(0, 12, 0, 0);
        scanPageLayout->setSpacing(12);

        tabs_->addTab(statusPage, style()->standardIcon(QStyle::SP_ComputerIcon), QString());
        tabs_->addTab(scanPage, style()->standardIcon(QStyle::SP_BrowserReload), QString());
        tabs_->setCurrentIndex(0);
        root->addWidget(tabs_);

        auto *overallCard = new CardFrame;
        overallCard->setObjectName(QStringLiteral("CardFrame"));
        overallCard->setInteractive(false);
        auto *overallLayout = new QVBoxLayout(overallCard);
        overallLayout->setContentsMargins(16, 16, 16, 16);
        overallLayout->setSpacing(10);
        overallTitle_ = new QLabel(overallCard);
        overallTitle_->setObjectName(QStringLiteral("SectionTitle"));
        overallTitle_->hide();
        auto *overallScroll = createCardList(&overallRows_, 92);
        overallScroll->setMaximumHeight(108);
        overallLayout->addWidget(overallScroll);
        metricsPageLayout->addWidget(overallCard);

        auto *metricsCard = new CardFrame;
        metricsCard->setObjectName(QStringLiteral("CardFrame"));
        metricsCard->setInteractive(false);
        auto *metricsLayout = new QVBoxLayout(metricsCard);
        metricsLayout->setContentsMargins(16, 16, 16, 16);
        metricsLayout->setSpacing(10);
        metricsTitle_ = new QLabel(metricsCard);
        metricsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        metricsTitle_->hide();
        metricsRelation_ = new QLabel(metricsCard);
        metricsRelation_->setObjectName(QStringLiteral("Intro"));
        metricsRelation_->setWordWrap(true);
        metricsRelation_->hide();
        metricsSeries_ = new QPieSeries;
        metricsSeries_->setHoleSize(0.52);
        metricsSeries_->setPieSize(0.82);
        metricsChart_ = new QChart;
        metricsChart_->setTheme(QChart::ChartThemeLight);
        metricsChart_->addSeries(metricsSeries_);
        metricsChart_->legend()->setVisible(true);
        metricsChart_->legend()->setAlignment(Qt::AlignRight);
        metricsChart_->setBackgroundVisible(true);
        metricsChart_->setBackgroundBrush(QBrush(QColor(251, 252, 253)));
        metricsChart_->setPlotAreaBackgroundVisible(false);
        metricsChart_->setBackgroundRoundness(0);
        metricsChart_->setMargins(QMargins(0, 0, 0, 0));
        metricsChartView_ = new QChartView(metricsChart_);
        metricsChartView_->setObjectName(QStringLiteral("MetricChart"));
        metricsChartView_->setAutoFillBackground(false);
        metricsChartView_->setRenderHint(QPainter::Antialiasing);
        metricsChartView_->setMinimumHeight(210);
        metricsChartView_->setMaximumHeight(230);
        metricsLayout->addWidget(metricsChartView_);
        auto *metricsScroll = createCardList(&metricsRows_, 230);
        metricsList_ = metricsScroll->widget();
        metricsLayout->addWidget(metricsScroll);
        metricsPageLayout->addWidget(metricsCard);
        metricsPageLayout->addStretch(1);

        appsStack_ = new QStackedWidget;
        appsStack_->setObjectName(QStringLiteral("AppsStack"));
        appsPageLayout->addWidget(appsStack_, 1);

        auto *appsOverview = new QWidget;
        appsOverview->setObjectName(QStringLiteral("TabPage"));
        auto *appsOverviewLayout = new QVBoxLayout(appsOverview);
        appsOverviewLayout->setContentsMargins(0, 0, 0, 0);
        appsOverviewLayout->setSpacing(12);

        auto *appsCard = new CardFrame;
        appsCard->setObjectName(QStringLiteral("CardFrame"));
        appsCard->setInteractive(false);
        auto *appsLayout = new QVBoxLayout(appsCard);
        appsLayout->setContentsMargins(16, 14, 16, 16);
        appsLayout->setSpacing(10);
        appsTitle_ = new QLabel;
        appsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        appsLayout->addWidget(appsTitle_);
        appsRelation_ = new QLabel;
        appsRelation_->setObjectName(QStringLiteral("Intro"));
        appsRelation_->setWordWrap(true);
        appsLayout->addWidget(appsRelation_);
        auto *appsSummaryScroll = createCardList(&appsSummaryRows_, 88);
        appsSummaryScroll->setMaximumHeight(104);
        appsLayout->addWidget(appsSummaryScroll);
        auto *appsScroll = createCardList(&appsRows_, 380);
        appsList_ = appsScroll->widget();
        appsLayout->addWidget(appsScroll);
        appsOverviewLayout->addWidget(appsCard);
        appsOverviewLayout->addStretch(1);

        auto *detailPage = new QWidget;
        detailPage->setObjectName(QStringLiteral("TabPage"));
        auto *detailLayout = new QVBoxLayout(detailPage);
        detailLayout->setContentsMargins(0, 0, 0, 0);
        detailLayout->setSpacing(12);
        auto *detailHeader = new QHBoxLayout;
        detailHeader->setSpacing(10);
        backToAppsButton_ = new AnimatedButton;
        backToAppsButton_->setObjectName(QStringLiteral("ActionButton"));
        backToAppsButton_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
        detailTitle_ = new QLabel;
        detailTitle_->setObjectName(QStringLiteral("SectionTitle"));
        detailHeader->addWidget(backToAppsButton_);
        detailHeader->addWidget(detailTitle_, 1);
        detailLayout->addLayout(detailHeader);
        auto *detailCard = new CardFrame;
        detailCard->setObjectName(QStringLiteral("CardFrame"));
        detailCard->setInteractive(false);
        auto *detailCardLayout = new QVBoxLayout(detailCard);
        detailCardLayout->setContentsMargins(16, 14, 16, 16);
        detailCardLayout->setSpacing(10);
        auto *detailScroll = createCardList(&detailRows_, 420);
        detailList_ = detailScroll->widget();
        detailCardLayout->addWidget(detailScroll);
        detailLayout->addWidget(detailCard, 1);
        connect(backToAppsButton_, &QPushButton::clicked, this, [this]() {
            if (appsStack_) {
                appsStack_->setCurrentIndex(0);
                fadeIn(appsStack_->currentWidget());
            }
        });

        appsStack_->addWidget(appsOverview);
        appsStack_->addWidget(detailPage);

        scanStack_ = new QStackedWidget;
        scanStack_->setObjectName(QStringLiteral("ScanStack"));
        scanPageLayout->addWidget(scanStack_, 1);

        auto *scanLaunchPage = new QWidget;
        scanLaunchPage->setObjectName(QStringLiteral("TabPage"));
        auto *scanLaunchLayout = new QVBoxLayout(scanLaunchPage);
        scanLaunchLayout->setContentsMargins(0, 0, 0, 0);
        scanLaunchLayout->setSpacing(12);

        auto *actionsCard = new CardFrame;
        actionsCard->setObjectName(QStringLiteral("HeroActionCard"));
        actionsCard->setInteractive(false);
        auto *actionsCardLayout = new QVBoxLayout(actionsCard);
        actionsCardLayout->setContentsMargins(20, 18, 20, 18);
        actionsCardLayout->setSpacing(14);
        actionsTitle_ = new QLabel;
        actionsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        actionsCardLayout->addWidget(actionsTitle_);
        scanButton_ = new AnimatedButton;
        scanButton_->setObjectName(QStringLiteral("PrimaryButton"));
        scanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        scanButton_->setIconSize(QSize(18, 18));
        scanButton_->setMinimumWidth(168);
        actionsCardLayout->addWidget(scanButton_, 0, Qt::AlignLeft);
        scanLaunchLayout->addWidget(actionsCard);
        scanLaunchLayout->addStretch(1);
        connect(scanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);

        auto *scanProgressPage = new QWidget;
        scanProgressPage->setObjectName(QStringLiteral("TabPage"));
        auto *scanProgressLayout = new QVBoxLayout(scanProgressPage);
        scanProgressLayout->setContentsMargins(0, 0, 0, 0);
        scanProgressLayout->setSpacing(12);
        auto *progressCard = new CardFrame;
        progressCard->setObjectName(QStringLiteral("HeroActionCard"));
        progressCard->setInteractive(false);
        auto *progressLayout = new QVBoxLayout(progressCard);
        progressLayout->setContentsMargins(20, 18, 20, 18);
        progressLayout->setSpacing(14);
        scanProgressTitle_ = new QLabel;
        scanProgressTitle_->setObjectName(QStringLiteral("SectionTitle"));
        scanProgressDetail_ = new QLabel;
        scanProgressDetail_->setObjectName(QStringLiteral("Intro"));
        scanProgressDetail_->setWordWrap(true);
        scanFlowProgress_ = new QProgressBar;
        scanFlowProgress_->setObjectName(QStringLiteral("FlowProgress"));
        scanFlowProgress_->setRange(0, 100);
        scanFlowProgress_->setValue(0);
        progressLayout->addWidget(scanProgressTitle_);
        progressLayout->addWidget(scanProgressDetail_);
        progressLayout->addWidget(scanFlowProgress_);
        scanProgressLayout->addWidget(progressCard);
        scanProgressLayout->addStretch(1);

        auto *scanResultPage = new QWidget;
        scanResultPage->setObjectName(QStringLiteral("TabPage"));
        auto *scanResultLayout = new QVBoxLayout(scanResultPage);
        scanResultLayout->setContentsMargins(0, 0, 0, 0);
        scanResultLayout->setSpacing(12);
        auto *resultCard = new CardFrame;
        resultCard->setObjectName(QStringLiteral("CardFrame"));
        resultCard->setInteractive(false);
        auto *resultLayout = new QVBoxLayout(resultCard);
        resultLayout->setContentsMargins(16, 14, 16, 16);
        resultLayout->setSpacing(10);
        resultTitle_ = new QLabel;
        resultTitle_->setObjectName(QStringLiteral("SectionTitle"));
        resultSummary_ = new QLabel;
        resultSummary_->setObjectName(QStringLiteral("Intro"));
        resultSummary_->setWordWrap(true);
        resultLayout->addWidget(resultTitle_);
        resultLayout->addWidget(resultSummary_);
        auto *optimizationScroll = createCardList(&optimizationRows_, 330);
        optimizationList_ = optimizationScroll->widget();
        resultLayout->addWidget(optimizationScroll, 1);
        auto *resultActions = new QHBoxLayout;
        resultActions->setSpacing(10);
        applyOptimizationsButton_ = new AnimatedButton;
        rescanButton_ = new AnimatedButton;
        applyOptimizationsButton_->setObjectName(QStringLiteral("PrimaryButton"));
        rescanButton_->setObjectName(QStringLiteral("ActionButton"));
        applyOptimizationsButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        rescanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        applyOptimizationsButton_->setIconSize(QSize(18, 18));
        rescanButton_->setIconSize(QSize(18, 18));
        resultActions->addWidget(applyOptimizationsButton_);
        resultActions->addWidget(rescanButton_);
        resultActions->addStretch(1);
        resultLayout->addLayout(resultActions);
        scanResultLayout->addWidget(resultCard, 1);
        connect(applyOptimizationsButton_, &QPushButton::clicked, this, &CleanerWindow::applySelectedOptimizations);
        connect(rescanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);

        auto *planCard = new CardFrame;
        planCard->setObjectName(QStringLiteral("CardFrame"));
        planCard->setInteractive(false);
        auto *planLayout = new QVBoxLayout(planCard);
        planLayout->setContentsMargins(16, 14, 16, 16);
        planLayout->setSpacing(10);
        planTitle_ = new QLabel;
        planTitle_->setObjectName(QStringLiteral("SectionTitle"));
        planLayout->addWidget(planTitle_);
        auto *planScroll = createCardList(&planRows_, 260);
        planList_ = planScroll->widget();
        planLayout->addWidget(planScroll, 1);
        scanResultLayout->addWidget(planCard);

        scanStack_->addWidget(scanLaunchPage);
        scanStack_->addWidget(scanProgressPage);
        scanStack_->addWidget(scanResultPage);

        setStyleSheet(QStringLiteral(R"(
            QWidget#AppRoot { background: #f5f7fa; color: #1d1d1f; }
            QWidget#ContentPane { background: transparent; }
            QScrollArea#ContentScroll {
                border: 0;
                background: transparent;
            }
            QScrollArea#CardListScroll {
                border: 0;
                background: transparent;
            }
            QScrollArea#ContentScroll > QWidget > QWidget {
                background: transparent;
            }
            QWidget#CardList, QWidget#TabPage {
                background: transparent;
            }
            QTabWidget#MainTabs::pane {
                border: 0;
                background: transparent;
                margin-top: 8px;
            }
            QTabBar::tab {
                min-width: 132px;
                min-height: 38px;
                padding: 0 18px;
                margin-right: 8px;
                border: 1px solid #d9dfe7;
                border-radius: 8px;
                background: #ffffff;
                color: #4a4f58;
                font-weight: 700;
            }
            QTabBar::tab:selected {
                background: #1d1d1f;
                color: #ffffff;
                border-color: #1d1d1f;
            }
            QTabBar::tab:hover:!selected {
                background: #f2f5f8;
                border-color: #aebaca;
            }
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
            QFrame#HeroActionCard {
                border-radius: 8px;
                border: 1px solid #d9dfe7;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #ffffff, stop:0.62 #f8fafc, stop:1 #eef3f7);
            }
            QFrame#StatusFrame {
                border: 1px solid #d9dfe7;
                border-radius: 8px;
                background: #ffffff;
                color: #1d1d1f;
            }
            QLabel#StatusTitle { color: #1d1d1f; font-weight: 700; }
            QLabel#StatusSummary { color: #24262b; font-weight: 600; }
            QLabel#LastUpdate { color: #737780; }
            QFrame#NavFrame {
                border: 1px solid #d9dfe7;
                border-radius: 8px;
                background: rgba(255, 255, 255, 214);
            }
            QLabel#SectionTitle {
                color: #1d1d1f;
                font-size: 15px;
                font-weight: 700;
            }
            QLabel#RowTitle {
                color: #1d1d1f;
                font-size: 15px;
                font-weight: 800;
            }
            QFrame#ValuePill {
                border: 1px solid #e1e6ed;
                border-radius: 8px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #ffffff, stop:1 #f4f7fa);
            }
            QLabel#PillLabel {
                color: #727782;
                font-size: 11px;
                font-weight: 700;
            }
            QLabel#PillValue {
                color: #20242a;
                font-size: 14px;
                font-weight: 800;
            }
            QFrame#PathPill {
                border: 1px solid #e1e6ed;
                border-radius: 8px;
                background: #fbfcfd;
            }
            QLabel#PathValue {
                color: #2b3037;
                font-size: 12px;
                font-weight: 650;
                line-height: 140%;
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
            QProgressBar#FlowProgress {
                border: 1px solid #cbd3dd;
                border-radius: 8px;
                background: #eef2f6;
                min-height: 16px;
                max-height: 16px;
                text-align: center;
                color: transparent;
            }
            QProgressBar#FlowProgress::chunk {
                border-radius: 8px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #1d1d1f, stop:0.48 #596572, stop:1 #9aa7b3);
            }
            QChartView#MetricChart {
                border: 1px solid #e1e6ed;
                border-radius: 8px;
                background: #fbfcfd;
            }
            QFrame#MetricRail {
                background: transparent;
            }
            QFrame#MetricRailLine {
                background: #c8d0d9;
                border-radius: 1px;
            }
            QProgressBar#TotalMetricBar,
            QProgressBar#ChildMetricBar {
                border: 0;
                border-radius: 4px;
                background: #e9eef3;
                min-height: 8px;
                max-height: 8px;
            }
            QProgressBar#TotalMetricBar::chunk {
                border-radius: 4px;
                background: #1d1d1f;
            }
            QProgressBar#ChildMetricBar::chunk {
                border-radius: 4px;
                background: #697684;
            }
            QCheckBox {
                color: #1d1d1f;
                font-weight: 750;
                spacing: 10px;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
                border-radius: 5px;
                border: 1px solid #aebaca;
                background: #ffffff;
            }
            QCheckBox::indicator:checked {
                background: #1d1d1f;
                border-color: #1d1d1f;
            }
            QCheckBox::indicator:disabled {
                background: #eef2f6;
                border-color: #d9dfe7;
            }
            QComboBox {
                min-height: 34px;
                padding: 0 30px 0 12px;
                border-radius: 7px;
                border: 1px solid #cbd3dd;
                background: #ffffff;
                color: #1d1d1f;
                font-weight: 600;
            }
            QComboBox:hover {
                border-color: #9fb0c2;
                background: #f7f9fb;
            }
            QComboBox::drop-down {
                width: 26px;
                border: 0;
                background: transparent;
            }
            QComboBox::down-arrow {
                image: none;
                width: 0;
                height: 0;
                border-left: 4px solid transparent;
                border-right: 4px solid transparent;
                border-top: 5px solid #3a3d42;
                margin-right: 8px;
            }
            QComboBox QAbstractItemView {
                border: 1px solid #cbd3dd;
                background: #ffffff;
                color: #1d1d1f;
                selection-background-color: #dce5ee;
                selection-color: #1d1d1f;
                outline: 0;
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
            QPushButton#PrimaryButton:hover,
            QPushButton#NavSelected:hover { background: #343941; }
            QPushButton#NavButton {
                min-height: 40px;
                border-radius: 8px;
                background: transparent;
                border-color: transparent;
                color: #3f454e;
            }
            QPushButton#NavSelected {
                min-height: 40px;
                border-radius: 8px;
                color: #ffffff;
                border: 1px solid #1d1d1f;
                background: #1d1d1f;
            }
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
        if (heroScanButton_) {
            heroScanButton_->setText(text.startScan);
        }
        if (statusNavButton_) {
            statusNavButton_->setText(text.statusTab);
        }
        if (scanNavButton_) {
            scanNavButton_->setText(text.scanTab);
        }
        if (scanButton_) {
            scanButton_->setText(text.startScan);
        }
        if (cleanOldButton_) {
            cleanOldButton_->setText(text.cleanOld);
        }
        if (autostartButton_) {
            autostartButton_->setText(text.manageAutostart);
        }
        metricsTitle_->setText(text.metricsTitle);
        if (overallTitle_) {
            overallTitle_->setText(text.overallTitle);
        }
        if (metricsRelation_) {
            metricsRelation_->setText(text.metricsRelation);
        }
        appsTitle_->setText(text.applicationsTitle);
        if (appsRelation_) {
            appsRelation_->setText(text.kaimingRelation);
        }
        if (metricsPageButton_) {
            metricsPageButton_->setText(text.metricsTitle);
        }
        if (appsPageButton_) {
            appsPageButton_->setText(text.applicationsTitle);
        }
        if (backToAppsButton_) {
            backToAppsButton_->setText(text.back);
        }
        if (actionsTitle_) {
            actionsTitle_->setText(text.scanTab);
        }
        if (scanProgressTitle_) {
            scanProgressTitle_->setText(text.scanProgressTitle);
        }
        if (scanProgressDetail_) {
            scanProgressDetail_->setText(text.scanProgressDetail);
        }
        if (resultTitle_) {
            resultTitle_->setText(text.scanResultTitle);
        }
        if (planTitle_) {
            planTitle_->setText(text.resultTitle);
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setText(text.optimizeSelected);
        }
        if (rescanButton_) {
            rescanButton_->setText(text.rescan);
        }
        if (tabs_) {
            tabs_->setTabText(0, text.statusTab);
            tabs_->setTabText(1, text.scanTab);
        }
        updateStatusSummary();

        if (!state_.isEmpty()) {
            updateMetrics();
            updateApplications();
        } else {
            resetOverallCard();
            resetMetricCards();
            resetApplicationSummary();
            clearRows(appsRows_);
        }
    }

    QScrollArea *createCardList(QVBoxLayout **rows, int minimumHeight)
    {
        auto *scroll = new QScrollArea;
        scroll->setObjectName(QStringLiteral("CardListScroll"));
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setMinimumHeight(minimumHeight);
        auto *content = new QWidget;
        content->setObjectName(QStringLiteral("CardList"));
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(2, 2, 2, 2);
        layout->setSpacing(10);
        scroll->setWidget(content);
        *rows = layout;
        return scroll;
    }

    static QLabel *makeLabel(const QString &text, const QString &objectName)
    {
        auto *label = new QLabel(text);
        label->setObjectName(objectName);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setWordWrap(objectName == QStringLiteral("PillValue") || objectName == QStringLiteral("PathValue"));
        label->setToolTip(text);
        return label;
    }

    static void clearRows(QVBoxLayout *layout)
    {
        if (!layout) {
            return;
        }
        while (QLayoutItem *item = layout->takeAt(0)) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        layout->addStretch(1);
    }

    QFrame *createValuePill(const QString &label, const QString &value)
    {
        auto *group = new QFrame;
        group->setObjectName(QStringLiteral("ValuePill"));
        auto *groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(10, 4, 10, 4);
        groupLayout->setSpacing(2);
        groupLayout->addWidget(makeLabel(label, QStringLiteral("PillLabel")));
        groupLayout->addWidget(makeLabel(value, QStringLiteral("PillValue")));
        return group;
    }

    CardFrame *createMetricRow(const QString &name,
                               const QString &usage,
                               const QString &cleanable,
                               const QString &status,
                               qint64 value,
                               qint64 rootValue,
                               bool total)
    {
        auto *row = new CardFrame;
        row->setObjectName(total ? QStringLiteral("TotalMetricRow") : QStringLiteral("ChildMetricRow"));
        row->setInteractive(true);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, total ? 10 : 8, 14, total ? 10 : 8);
        layout->setSpacing(12);

        if (!total) {
            auto *rail = new QFrame;
            rail->setObjectName(QStringLiteral("MetricRail"));
            rail->setFixedWidth(22);
            auto *railLayout = new QVBoxLayout(rail);
            railLayout->setContentsMargins(9, 0, 9, 0);
            auto *line = new QFrame;
            line->setObjectName(QStringLiteral("MetricRailLine"));
            line->setFixedWidth(2);
            railLayout->addWidget(line);
            layout->addWidget(rail);
        }

        auto *main = new QVBoxLayout;
        main->setSpacing(7);
        auto *title = makeLabel(total ? name + QStringLiteral("  ·  总计") : name, QStringLiteral("RowTitle"));
        main->addWidget(title);

        auto *bar = new QProgressBar;
        bar->setObjectName(total ? QStringLiteral("TotalMetricBar") : QStringLiteral("ChildMetricBar"));
        bar->setTextVisible(false);
        bar->setRange(0, 1000);
        const int percentValue = rootValue > 0 ? static_cast<int>(qBound<qint64>(0, value * 1000 / rootValue, 1000)) : (total ? 1000 : 0);
        bar->setValue(total ? 1000 : percentValue);
        main->addWidget(bar);
        layout->addLayout(main, 3);

        const Text text = t();
        const QString percent = rootValue > 0
            ? QString::number(value * 100.0 / rootValue, 'f', total ? 0 : 1) + QLatin1Char('%')
            : QStringLiteral("-");
        layout->addWidget(createValuePill(text.before, usage), 1);
        layout->addWidget(createValuePill(text.released, cleanable), 1);
        layout->addWidget(createValuePill(total ? text.metricStatus : text.shareOfRoot, total ? status : percent), 1);
        return row;
    }

    ClickableCardFrame *createInfoRow(const QString &title,
                                      const QStringList &labels,
                                      const QStringList &values,
                                      bool clickable = false)
    {
        auto *row = new ClickableCardFrame;
        row->setObjectName(QStringLiteral("InfoRow"));
        row->setInteractive(true);
        if (!clickable) {
            row->setCursor(Qt::ArrowCursor);
        }
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 8, 14, 8);
        layout->setSpacing(12);

        auto *titleLabel = makeLabel(title, QStringLiteral("RowTitle"));
        titleLabel->setMinimumWidth(180);
        layout->addWidget(titleLabel, 2);

        for (int i = 0; i < values.size(); ++i) {
            layout->addWidget(createValuePill(labels.value(i), values.at(i)), 1);
        }
        return row;
    }

    CardFrame *createContainerCard(const QJsonObject &item)
    {
        const Text text = t();
        auto *row = new CardFrame;
        row->setObjectName(QStringLiteral("ContainerRow"));
        row->setInteractive(false);
        auto *layout = new QVBoxLayout(row);
        layout->setContentsMargins(14, 10, 14, 10);
        layout->setSpacing(10);

        auto *title = makeLabel(item.value(QStringLiteral("module")).toString(), QStringLiteral("RowTitle"));
        layout->addWidget(title);

        auto *meta = new QHBoxLayout;
        meta->setSpacing(10);
        meta->addWidget(createValuePill(text.version, item.value(QStringLiteral("version")).toString()), 1);
        meta->addWidget(createValuePill(text.appSize, fmtBytes(jsonInt64(item, QStringLiteral("bytes")))), 1);
        meta->addWidget(createValuePill(text.currentLayer, item.value(QStringLiteral("current")).toBool() ? text.active : text.disabled), 1);
        meta->addWidget(createValuePill(text.inUse, item.value(QStringLiteral("inUse")).toBool() ? text.active : text.disabled), 1);
        layout->addLayout(meta);

        auto *pathFrame = new QFrame;
        pathFrame->setObjectName(QStringLiteral("PathPill"));
        auto *pathLayout = new QVBoxLayout(pathFrame);
        pathLayout->setContentsMargins(10, 6, 10, 6);
        pathLayout->setSpacing(3);
        pathLayout->addWidget(makeLabel(text.path, QStringLiteral("PillLabel")));
        pathLayout->addWidget(makeLabel(item.value(QStringLiteral("path")).toString(), QStringLiteral("PathValue")));
        layout->addWidget(pathFrame);
        return row;
    }

    CardFrame *createOptimizationRow(QCheckBox *box, const QString &title, const QString &detail, const QString &size)
    {
        auto *row = new CardFrame;
        row->setObjectName(QStringLiteral("InfoRow"));
        row->setInteractive(true);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 10, 14, 10);
        layout->setSpacing(12);
        box->setText(title);
        layout->addWidget(box, 2);
        auto *detailLabel = makeLabel(detail, QStringLiteral("PathValue"));
        layout->addWidget(detailLabel, 3);
        if (!size.isEmpty()) {
            layout->addWidget(createValuePill(t().appSize, size), 1);
        }
        return row;
    }

    void resetMetricCards()
    {
        const Text text = t();
        clearRows(metricsRows_);
        clearMetricChart();
        const QStringList names{text.kaiming, text.ostree, text.kare, text.otherRoot};
        for (const QString &name : names) {
            addMetricCard(name, text.pendingScan, QStringLiteral("-"), text.pendingScan);
        }
    }

    void resetOverallCard()
    {
        const Text text = t();
        clearRows(overallRows_);
        if (!overallRows_) {
            return;
        }
        if (overallRows_->count() > 0 && overallRows_->itemAt(overallRows_->count() - 1)->spacerItem()) {
            delete overallRows_->takeAt(overallRows_->count() - 1);
        }
        overallRows_->addWidget(createInfoRow(text.rootUsed,
                                              {text.before, text.explainedUsage, text.otherRoot},
                                              {text.pendingScan, text.pendingScan, text.pendingScan}));
        overallRows_->addStretch(1);
    }

    void addMetricCard(const QString &name, const QString &usage, const QString &cleanable, const QString &status)
    {
        addMetricCard(name, usage, cleanable, status, 0, 0, false);
    }

    void addMetricCard(const QString &name,
                       const QString &usage,
                       const QString &cleanable,
                       const QString &status,
                       qint64 value,
                       qint64 rootValue,
                       bool total)
    {
        if (!metricsRows_) {
            return;
        }
        if (metricsRows_->count() > 0 && metricsRows_->itemAt(metricsRows_->count() - 1)->spacerItem()) {
            delete metricsRows_->takeAt(metricsRows_->count() - 1);
        }
        metricsRows_->addWidget(createMetricRow(name, usage, cleanable, status, value, rootValue, total));
        metricsRows_->addStretch(1);
    }

    void clearMetricChart()
    {
        if (!metricsSeries_) {
            return;
        }
        metricsSeries_->clear();
        metricsSeries_->append(t().pendingScan, 1.0);
        if (QPieSlice *slice = metricsSeries_->slices().isEmpty() ? nullptr : metricsSeries_->slices().first()) {
            slice->setColor(QColor(203, 211, 221));
            slice->setBorderColor(QColor(203, 211, 221));
            slice->setLabelVisible(false);
        }
    }

    void updateMetricChart(const QStringList &names, const QList<qint64> &values, qint64 rootValue)
    {
        if (!metricsSeries_) {
            return;
        }
        metricsSeries_->clear();
        const QList<QColor> colors{
            QColor(29, 31, 36),
            QColor(92, 105, 118),
            QColor(130, 145, 158),
            QColor(181, 190, 199)
        };
        for (int i = 0; i < names.size() && i < values.size(); ++i) {
            const qint64 value = qMax<qint64>(0, values.at(i));
            if (value == 0) {
                continue;
            }
            QPieSlice *slice = metricsSeries_->append(names.at(i), static_cast<qreal>(value));
            const QColor color = colors.at(i % colors.size());
            slice->setColor(color);
            slice->setBorderColor(QColor(255, 255, 255));
            const QString percent = rootValue > 0
                ? QString::number(value * 100.0 / rootValue, 'f', 1) + QLatin1Char('%')
                : QStringLiteral("-");
            slice->setLabel(names.at(i) + QStringLiteral(" ") + percent);
            slice->setLabelVisible(true);
        }
        if (metricsSeries_->slices().isEmpty()) {
            clearMetricChart();
        }
    }

    void addApplicationCard(int row, const QJsonObject &app)
    {
        if (!appsRows_) {
            return;
        }
        if (appsRows_->count() > 0 && appsRows_->itemAt(appsRows_->count() - 1)->spacerItem()) {
            delete appsRows_->takeAt(appsRows_->count() - 1);
        }
        const Text text = t();
        auto *card = createInfoRow(app.value(QStringLiteral("ref")).toString(),
                                   {text.appKind, text.appContainers, text.appSize},
                                   {app.value(QStringLiteral("kind")).toString(),
                                    QString::number(app.value(QStringLiteral("containerCount")).toInt()),
                                    fmtBytes(jsonInt64(app, QStringLiteral("bytes")))},
                                   true);
        connect(card, &ClickableCardFrame::clicked, this, [this, row]() {
            showApplicationContainers(row);
        });
        appsRows_->addWidget(card);
        appsRows_->addStretch(1);
    }

    void resetApplicationSummary()
    {
        const Text text = t();
        clearRows(appsSummaryRows_);
        if (!appsSummaryRows_) {
            return;
        }
        if (appsSummaryRows_->count() > 0 && appsSummaryRows_->itemAt(appsSummaryRows_->count() - 1)->spacerItem()) {
            delete appsSummaryRows_->takeAt(appsSummaryRows_->count() - 1);
        }
        appsSummaryRows_->addWidget(createInfoRow(text.appSummaryTitle,
                                                  {text.appTotal, text.totalContainers, text.appContainersUsage},
                                                  {text.pendingScan, text.pendingScan, text.pendingScan}));
        appsSummaryRows_->addStretch(1);
    }

    void updateApplicationSummary(const QJsonArray &applications)
    {
        const Text text = t();
        int containerCount = 0;
        qint64 totalBytes = 0;
        for (const QJsonValue &value : applications) {
            const QJsonObject app = value.toObject();
            containerCount += app.value(QStringLiteral("containerCount")).toInt();
            totalBytes += jsonInt64(app, QStringLiteral("bytes"));
        }
        clearRows(appsSummaryRows_);
        if (!appsSummaryRows_) {
            return;
        }
        if (appsSummaryRows_->count() > 0 && appsSummaryRows_->itemAt(appsSummaryRows_->count() - 1)->spacerItem()) {
            delete appsSummaryRows_->takeAt(appsSummaryRows_->count() - 1);
        }
        appsSummaryRows_->addWidget(createInfoRow(text.appSummaryTitle,
                                                  {text.appTotal, text.totalContainers, text.appContainersUsage},
                                                  {QString::number(applications.size()),
                                                   QString::number(containerCount),
                                                   fmtBytes(totalBytes)}));
        appsSummaryRows_->addStretch(1);
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
        if (!planRows_) {
            return;
        }
        if (planRows_->count() > 0 && planRows_->itemAt(planRows_->count() - 1)->spacerItem()) {
            delete planRows_->takeAt(planRows_->count() - 1);
        }
        auto *row = createInfoRow(stage, {t().status, t().detail}, {status, detail});
        planRows_->addWidget(row);
        planRows_->addStretch(1);
    }

    void setBusy(bool busy)
    {
        busy_ = busy;
        if (visual_) {
            visual_->setWorking(busy);
        }
        if (scanButton_) {
            scanButton_->setEnabled(!busy);
        }
        if (cleanOldButton_) {
            cleanOldButton_->setEnabled(!busy);
        }
        if (autostartButton_) {
            autostartButton_->setEnabled(!busy);
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setEnabled(!busy);
        }
        if (rescanButton_) {
            rescanButton_->setEnabled(!busy);
        }
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

    void beginScanProgress()
    {
        if (!scanStack_) {
            return;
        }
        scanProgressValue_ = 4;
        if (scanFlowProgress_) {
            scanFlowProgress_->setValue(scanProgressValue_);
        }
        if (scanProgressDetail_) {
            scanProgressDetail_->setText(t().scanProgressDetail);
        }
        scanStack_->setCurrentIndex(1);
        fadeIn(scanStack_->currentWidget());
        if (!scanProgressTimer_) {
            scanProgressTimer_ = new QTimer(this);
            connect(scanProgressTimer_, &QTimer::timeout, this, [this]() {
                if (!scanFlowProgress_) {
                    return;
                }
                scanProgressValue_ = qMin(88, scanProgressValue_ + 7);
                scanFlowProgress_->setValue(scanProgressValue_);
            });
        }
        scanProgressTimer_->start(350);
    }

    void finishScanProgress()
    {
        if (scanProgressTimer_) {
            scanProgressTimer_->stop();
        }
        scanProgressValue_ = 100;
        if (scanFlowProgress_) {
            scanFlowProgress_->setValue(100);
        }
    }

    void scanInternal(bool manual)
    {
        const Text text = t();
        setBusy(true);
        if (manual) {
            clearRows(planRows_);
            beginScanProgress();
        }
        runProcess({helper_, QStringLiteral("--scan"), QStringLiteral("--user"), user_}, [this, manual](int code, const QByteArray &output) {
            const Text text = t();
            if (manual) {
                finishScanProgress();
            }
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
                showScanResults();
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

    void addOptimizationCard(CardFrame *card)
    {
        if (!optimizationRows_) {
            return;
        }
        if (optimizationRows_->count() > 0 && optimizationRows_->itemAt(optimizationRows_->count() - 1)->spacerItem()) {
            delete optimizationRows_->takeAt(optimizationRows_->count() - 1);
        }
        optimizationRows_->addWidget(card);
        optimizationRows_->addStretch(1);
    }

    void showScanResults()
    {
        const Text text = t();
        resultContainerBoxes_.clear();
        resultAutostartBoxes_.clear();
        clearRows(optimizationRows_);
        clearRows(planRows_);

        const QJsonArray containers = state_.value(QStringLiteral("oldContainers")).toArray();
        const QJsonArray autostarts = state_.value(QStringLiteral("autostarts")).toArray();
        int selectableContainers = 0;
        int selectableAutostarts = 0;

        addOptimizationCard(createInfoRow(text.containerCleanup, {text.status}, {text.selectContainers}));
        for (const QJsonValue &value : containers) {
            const QJsonObject item = value.toObject();
            auto *box = new QCheckBox;
            const bool selectable = !item.value(QStringLiteral("inUse")).toBool();
            box->setChecked(selectable);
            box->setEnabled(selectable);
            if (selectable) {
                ++selectableContainers;
            }
            const QString title = QStringLiteral("%1  %2").arg(item.value(QStringLiteral("ref")).toString(),
                                                               item.value(QStringLiteral("version")).toString());
            addOptimizationCard(createOptimizationRow(box,
                                                      title,
                                                      item.value(QStringLiteral("path")).toString(),
                                                      fmtBytes(jsonInt64(item, QStringLiteral("bytes")))));
            resultContainerBoxes_.append({item.value(QStringLiteral("path")).toString(), box});
        }

        addOptimizationCard(createInfoRow(text.autostartOptimization, {text.status}, {text.selectAutostarts}));
        for (const QJsonValue &value : autostarts) {
            const QJsonObject item = value.toObject();
            if (item.value(QStringLiteral("disabled")).toBool()) {
                continue;
            }
            auto *box = new QCheckBox;
            box->setChecked(true);
            ++selectableAutostarts;
            addOptimizationCard(createOptimizationRow(box,
                                                      localName(item),
                                                      localDescription(item) + QStringLiteral("\n") + item.value(QStringLiteral("target")).toString(),
                                                      QString()));
            resultAutostartBoxes_.append({item.value(QStringLiteral("id")).toString(), box});
        }

        if (selectableContainers == 0 && selectableAutostarts == 0) {
            addOptimizationCard(createInfoRow(text.optimizationItems, {text.status}, {text.noCleanable}));
        }

        if (resultSummary_) {
            resultSummary_->setText(scanSummary());
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setEnabled(selectableContainers > 0 || selectableAutostarts > 0);
        }
        if (scanStack_) {
            scanStack_->setCurrentIndex(2);
            fadeIn(scanStack_->currentWidget());
        }
    }

    void updateMetrics()
    {
        const Text text = t();
        const QJsonObject metrics = state_.value(QStringLiteral("metrics")).toObject();
        const qint64 rootValue = jsonInt64(metrics, QStringLiteral("root_used"));
        const QStringList keys{QStringLiteral("kaiming"), QStringLiteral("ostree_upper"), QStringLiteral("kare_upper"), QStringLiteral("root_other")};
        const QStringList names{text.kaiming, text.ostree, text.kare, text.otherRoot};
        QList<qint64> values;
        qint64 explained = 0;
        for (const QString &key : keys) {
            const qint64 value = jsonInt64(metrics, key);
            values << value;
            explained += value;
        }
        resetOverallCard();
        if (overallRows_ && overallRows_->count() > 0 && overallRows_->itemAt(overallRows_->count() - 1)->spacerItem()) {
            delete overallRows_->takeAt(overallRows_->count() - 1);
        }
        clearRows(overallRows_);
        if (overallRows_) {
            overallRows_->addWidget(createInfoRow(text.rootUsed,
                                                  {text.before, text.explainedUsage, text.otherRoot},
                                                  {fmtBytes(rootValue), fmtBytes(explained - values.value(3)), fmtBytes(values.value(3))}));
            overallRows_->addStretch(1);
        }
        updateMetricChart(names, values, rootValue);
        const qint64 oldContainerBytes = oldContainersBytes();
        clearRows(metricsRows_);
        for (int row = 0; row < keys.size(); ++row) {
            const qint64 value = values.at(row);
            const QString cleanable = row == 0 ? fmtBytes(oldContainerBytes) : QStringLiteral("-");
            QString status;
            if (row == 0) {
                status = oldContainerBytes > 0 ? text.cleanable : text.noCleanable;
            } else {
                status = text.normal;
            }
            addMetricCard(names.at(row), fmtBytes(value), cleanable, status, value, rootValue, false);
        }
    }

    void updateApplications()
    {
        const QJsonArray applications = state_.value(QStringLiteral("applications")).toArray();
        updateApplicationSummary(applications);
        clearRows(appsRows_);
        for (int row = 0; row < applications.size(); ++row) {
            const QJsonObject app = applications.at(row).toObject();
            addApplicationCard(row, app);
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

        if (!detailRows_ || !appsStack_) {
            return;
        }
        detailTitle_->setText(text.containerDetails + QStringLiteral(" - ") + app.value(QStringLiteral("ref")).toString());
        clearRows(detailRows_);
        for (int i = 0; i < containers.size(); ++i) {
            const QJsonObject item = containers.at(i).toObject();
            if (detailRows_->count() > 0 && detailRows_->itemAt(detailRows_->count() - 1)->spacerItem()) {
                delete detailRows_->takeAt(detailRows_->count() - 1);
            }
            detailRows_->addWidget(createContainerCard(item));
            detailRows_->addStretch(1);
        }
        appsStack_->setCurrentIndex(1);
        fadeIn(appsStack_->currentWidget());
    }

    void updateMainNav(int index)
    {
        if (!statusNavButton_ || !scanNavButton_) {
            return;
        }
        statusNavButton_->setObjectName(index == 0 ? QStringLiteral("NavSelected") : QStringLiteral("NavButton"));
        scanNavButton_->setObjectName(index == 1 ? QStringLiteral("NavSelected") : QStringLiteral("NavButton"));
        statusNavButton_->style()->unpolish(statusNavButton_);
        statusNavButton_->style()->polish(statusNavButton_);
        scanNavButton_->style()->unpolish(scanNavButton_);
        scanNavButton_->style()->polish(scanNavButton_);
        fadeIn(tabs_ ? tabs_->currentWidget() : nullptr);
    }

    void showStatusSubPage(int index)
    {
        if (!statusStack_) {
            return;
        }
        statusStack_->setCurrentIndex(index);
        metricsPageButton_->setObjectName(index == 0 ? QStringLiteral("PrimaryButton") : QStringLiteral("ActionButton"));
        appsPageButton_->setObjectName(index == 1 ? QStringLiteral("PrimaryButton") : QStringLiteral("ActionButton"));
        metricsPageButton_->style()->unpolish(metricsPageButton_);
        metricsPageButton_->style()->polish(metricsPageButton_);
        appsPageButton_->style()->unpolish(appsPageButton_);
        appsPageButton_->style()->polish(appsPageButton_);
        fadeIn(statusStack_->currentWidget());
    }

    void fadeIn(QWidget *widget)
    {
        if (!widget) {
            return;
        }
        auto *effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        auto *animation = new QPropertyAnimation(effect, "opacity", widget);
        animation->setDuration(180);
        animation->setStartValue(0.72);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QPropertyAnimation::finished, widget, [widget]() {
            widget->setGraphicsEffect(nullptr);
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
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
        if (!metricsList_) {
            return;
        }
        auto *effect = new QGraphicsOpacityEffect(metricsList_);
        metricsList_->setGraphicsEffect(effect);
        auto *animation = new QPropertyAnimation(effect, "opacity", metricsList_);
        animation->setDuration(260);
        animation->setStartValue(0.55);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            if (metricsList_) {
                metricsList_->setGraphicsEffect(nullptr);
            }
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

    void applySelectedOptimizations()
    {
        QStringList containerPaths;
        QStringList disableIds;
        for (const auto &pair : resultContainerBoxes_) {
            if (pair.second && pair.second->isChecked() && pair.second->isEnabled()) {
                containerPaths << pair.first;
            }
        }
        for (const auto &pair : resultAutostartBoxes_) {
            if (pair.second && pair.second->isChecked()) {
                disableIds << pair.first;
            }
        }
        clearRows(planRows_);
        if (containerPaths.isEmpty() && disableIds.isEmpty()) {
            addPlanRow(t().planned, t().done, t().noCleanable);
            return;
        }
        setBusy(true);
        runSelectedAutostartOptimization(disableIds, containerPaths);
    }

    void runSelectedAutostartOptimization(const QStringList &disableIds, const QStringList &containerPaths)
    {
        if (disableIds.isEmpty()) {
            runSelectedContainerCleanup(containerPaths);
            return;
        }
        addPlanRow(t().autostartOptimization, t().running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Disable %1 selected preheat/autostart entries.").arg(disableIds.size())
            : QStringLiteral("禁用 %1 个选中的预热/自启动项。").arg(disableIds.size()));
        runProcess({helper_, QStringLiteral("--manage-autostart"), QStringLiteral("--user"), user_,
                    QStringLiteral("--disable-entries"), disableIds.join(QLatin1Char(',')),
                    QStringLiteral("--enable-entries"), QString()},
                   [this, containerPaths](int code, const QByteArray &output) {
            appendActionResult(t().autostartOptimization, code, output);
            runSelectedContainerCleanup(containerPaths);
        });
    }

    void runSelectedContainerCleanup(const QStringList &paths)
    {
        if (paths.isEmpty()) {
            finishSelectedOptimization();
            return;
        }
        QStringList command{QStringLiteral("pkexec"), helper_, QStringLiteral("--apply-old-containers")};
        for (const QString &path : paths) {
            command << QStringLiteral("--container") << path;
        }
        addPlanRow(t().containerCleanup, t().running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Move %1 selected old containers to DATA rollback quarantine.").arg(paths.size())
            : QStringLiteral("移动 %1 个选中的旧容器到 DATA 回滚隔离区。").arg(paths.size()));
        runProcess(command, [this](int code, const QByteArray &output) {
            appendActionResult(t().containerCleanup, code, output);
            finishSelectedOptimization();
        });
    }

    void appendActionResult(const QString &stage, int code, const QByteArray &output)
    {
        const Text text = t();
        const QJsonDocument doc = QJsonDocument::fromJson(output);
        if (code != 0 || !doc.isObject()) {
            const QString path = writeErrorLog(QStringLiteral("selected optimization failed"), output);
            addPlanRow(stage, text.failed, language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Operation failed. See the error dialog for the log path.")
                : QStringLiteral("操作失败。错误日志位置见弹窗。"));
            showErrorDialog(path);
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
        addPlanRow(stage, failCount == 0 ? text.done : text.failed, summary);
        if (failCount > 0) {
            const QString path = writeErrorLog(QStringLiteral("partial selected optimization failure"), output);
            showErrorDialog(path);
        }
    }

    void finishSelectedOptimization()
    {
        setBusy(false);
        scanInternal(false);
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
    QTabWidget *tabs_ = nullptr;
    QStackedWidget *statusStack_ = nullptr;
    QStackedWidget *appsStack_ = nullptr;
    QStackedWidget *scanStack_ = nullptr;
    QLabel *overallTitle_ = nullptr;
    QLabel *metricsTitle_ = nullptr;
    QLabel *metricsRelation_ = nullptr;
    QLabel *appsTitle_ = nullptr;
    QLabel *appsRelation_ = nullptr;
    QLabel *detailTitle_ = nullptr;
    QLabel *actionsTitle_ = nullptr;
    QLabel *resultTitle_ = nullptr;
    QLabel *resultSummary_ = nullptr;
    QLabel *planTitle_ = nullptr;
    QLabel *scanProgressTitle_ = nullptr;
    QLabel *scanProgressDetail_ = nullptr;
    QWidget *metricsList_ = nullptr;
    QWidget *appsList_ = nullptr;
    QWidget *detailList_ = nullptr;
    QWidget *planList_ = nullptr;
    QWidget *optimizationList_ = nullptr;
    QChartView *metricsChartView_ = nullptr;
    QChart *metricsChart_ = nullptr;
    QPieSeries *metricsSeries_ = nullptr;
    QVBoxLayout *overallRows_ = nullptr;
    QVBoxLayout *metricsRows_ = nullptr;
    QVBoxLayout *appsSummaryRows_ = nullptr;
    QVBoxLayout *appsRows_ = nullptr;
    QVBoxLayout *detailRows_ = nullptr;
    QVBoxLayout *planRows_ = nullptr;
    QVBoxLayout *optimizationRows_ = nullptr;
    QProgressBar *scanFlowProgress_ = nullptr;
    QTimer *scanProgressTimer_ = nullptr;
    int scanProgressValue_ = 0;
    QVector<QPair<QString, QCheckBox *>> resultContainerBoxes_;
    QVector<QPair<QString, QCheckBox *>> resultAutostartBoxes_;
    QPushButton *heroScanButton_ = nullptr;
    QPushButton *statusNavButton_ = nullptr;
    QPushButton *scanNavButton_ = nullptr;
    QPushButton *metricsPageButton_ = nullptr;
    QPushButton *appsPageButton_ = nullptr;
    QPushButton *backToAppsButton_ = nullptr;
    QPushButton *applyOptimizationsButton_ = nullptr;
    QPushButton *rescanButton_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
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
