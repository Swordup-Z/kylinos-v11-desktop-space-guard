#include <QApplication>
#include <QAbstractItemView>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QParallelAnimationGroup>
#include <QProcess>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStyle>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegendMarker>
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
        setFixedSize(260, 150);
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
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const qreal baseWidth = 260.0;
        const qreal baseHeight = 210.0;
        const qreal scale = qMin(width() / baseWidth, height() / baseHeight);
        painter.translate((width() - baseWidth * scale) / 2.0,
                          (height() - baseHeight * scale) / 2.0);
        painter.scale(scale, scale);

        const QRectF card(1, 1, baseWidth - 2, baseHeight - 2);
        QLinearGradient bg(card.topLeft(), card.bottomRight());
        bg.setColorAt(0.00, QColor(255, 255, 255, 30));
        bg.setColorAt(0.48, QColor(255, 255, 255, 14));
        bg.setColorAt(1.00, QColor(255, 255, 255, 8));
        painter.setPen(QPen(QColor(255, 255, 255, 38), 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(card, 8, 8);

        QRadialGradient wash(QPointF(76, 74), 74);
        wash.setColorAt(0.00, QColor(35, 240, 174, 56));
        wash.setColorAt(0.58, QColor(42, 187, 220, 18));
        wash.setColorAt(1.00, QColor(18, 22, 30, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(wash);
        painter.drawEllipse(QPointF(76, 74), 70, 42);

        QRadialGradient mistWash(QPointF(184, 70), 72);
        mistWash.setColorAt(0.00, QColor(255, 62, 219, 38 + static_cast<int>(36 * activity_)));
        mistWash.setColorAt(0.66, QColor(255, 133, 60, 14));
        mistWash.setColorAt(1.00, QColor(98, 118, 138, 0));
        painter.setBrush(mistWash);
        painter.drawEllipse(QPointF(184, 70), 70, 44);

        const QColor ink(255, 255, 255);
        const QColor graphite(202, 215, 234);
        const QColor mist(38, 232, 180);
        const QColor silver(255, 169, 74);

        for (int i = 0; i < 3; ++i) {
            const QRectF layer(32 + i * 8, 42 + i * 18, 76, 16);
            painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 80 - i * 16), 1));
            painter.setBrush(QColor(255, 255, 255, 36 - i * 6));
            painter.drawRoundedRect(layer, 7, 7);
            painter.setPen(QPen(QColor(graphite.red(), graphite.green(), graphite.blue(), 54), 1));
            painter.drawLine(layer.left() + 11, layer.center().y(), layer.right() - 12, layer.center().y());
        }

        QPainterPath flow;
        flow.moveTo(105, 74);
        flow.cubicTo(128, 34, 158, 106, 185, 70);
        flow.cubicTo(200, 50, 218, 56, 232, 70);
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(), 58 + static_cast<int>(86 * activity_)),
                            2.4 + activity_ * 1.1,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawPath(flow);

        for (int i = 0; i < 6; ++i) {
            const qreal t = std::fmod(phase_ + i * 0.17, 1.0);
            const qreal x = 108 + t * 122;
            const qreal y = 72 + std::sin((t * 2.0 + phase_) * 3.14159265359) * 14;
            const int alpha = 42 + static_cast<int>(118 * activity_);
            const QColor dot = i % 2 ? silver : ink;
            painter.setBrush(QColor(dot.red(), dot.green(), dot.blue(), alpha));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(x, y), 1.8 + activity_ * 1.6, 1.8 + activity_ * 1.6);
        }

        const QPointF ringCenter(198, 74);
        const QRectF outer(ringCenter.x() - 38, ringCenter.y() - 38, 76, 76);
        const qreal quietPulse = 0.5 + 0.5 * std::sin(phase_ * 6.28318530718);
        painter.setPen(QPen(QColor(255, 255, 255, 38), 8, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(outer, 40 * 16, 285 * 16);
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(),
                                   150 + static_cast<int>(70 * activity_)),
                            8,
                            Qt::SolidLine,
                            Qt::RoundCap));
        painter.drawArc(outer,
                        static_cast<int>((84 + phase_ * 360) * 16),
                        static_cast<int>((88 + quietPulse * 24 + activity_ * 52) * 16));

        painter.setPen(QPen(QColor(255, 255, 255, 76), 1));
        painter.setBrush(QColor(255, 255, 255, 54));
        painter.drawEllipse(ringCenter, 18, 18);
        painter.setBrush(QColor(255, 255, 255, 220));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(ringCenter, 5.5, 5.5);

        painter.setPen(QPen(QColor(ink.red(), ink.green(), ink.blue(), 32), 1));
        painter.drawLine(QPointF(132, 120), QPointF(232, 120));
        painter.setPen(QPen(QColor(mist.red(), mist.green(), mist.blue(), 126), 1.4));
        const qreal x = 132 + std::fmod(phase_ * 122, 100.0);
        painter.drawLine(QPointF(x, 120), QPointF(x + 22, 120));
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

class GlassShellFrame : public QFrame {
    Q_OBJECT
    Q_PROPERTY(qreal phase READ phase WRITE setPhase)
    Q_PROPERTY(qreal themeBlend READ themeBlend WRITE setThemeBlend)

public:
    explicit GlassShellFrame(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setAutoFillBackground(false);
        motionTimer_ = new QTimer(this);
        connect(motionTimer_, &QTimer::timeout, this, [this]() {
            phase_ += 0.0042;
            if (phase_ > 10000.0) {
                phase_ = std::fmod(phase_, 1.0);
            }
            update();
        });
        motionTimer_->start(33);

        themeAnimation_ = new QPropertyAnimation(this, "themeBlend", this);
        themeAnimation_->setDuration(280);
        themeAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    }

    qreal phase() const { return phase_; }
    qreal themeBlend() const { return themeBlend_; }

    void setPhase(qreal phase)
    {
        phase_ = phase;
        if (qApp) {
            qApp->setProperty("spaceGuardGlassPhase", phase_);
        }
        update();
    }

    void setThemeBlend(qreal blend)
    {
        themeBlend_ = qBound<qreal>(0.0, blend, 1.0);
        update();
    }

    void setTargetTheme(int theme)
    {
        theme = qBound(0, theme, 2);
        if (theme == targetTheme_ && themeBlend_ >= 1.0) {
            return;
        }
        sourceTheme_ = blendedThemeIndex_;
        targetTheme_ = theme;
        themeAnimation_->stop();
        themeAnimation_->setStartValue(0.0);
        themeAnimation_->setEndValue(1.0);
        themeAnimation_->start();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF rect(0.5, 0.5, width() - 1, height() - 1);
        const ThemeColors from = colorsForTheme(sourceTheme_);
        const ThemeColors to = colorsForTheme(targetTheme_);
        const qreal eased = themeBlend_;
        const QColor a = mixedColor(from.a, to.a, eased);
        const QColor b = mixedColor(from.b, to.b, eased);
        const QColor c = mixedColor(from.c, to.c, eased);
        const QColor d = mixedColor(from.d, to.d, eased);
        const QColor accent = mixedColor(from.accent, to.accent, eased);
        blendedThemeIndex_ = eased >= 1.0 ? targetTheme_ : sourceTheme_;

        QPainterPath shape;
        shape.addRoundedRect(rect, 8, 8);
        painter.setClipPath(shape);

        QLinearGradient base(rect.topLeft(), rect.bottomRight());
        base.setColorAt(0.00, a);
        base.setColorAt(0.32, b);
        base.setColorAt(0.68, c);
        base.setColorAt(1.00, d);
        painter.fillPath(shape, base);

        const qreal orbit = phase_ * 6.28318530718;
        const QPointF leftWash(rect.left() + rect.width() * (0.18 + 0.05 * std::sin(orbit)),
                               rect.top() + rect.height() * (0.18 + 0.05 * std::cos(orbit * 0.8)));
        QRadialGradient first(leftWash, rect.width() * 0.58);
        first.setColorAt(0.00, QColor(255, 255, 255, 82));
        first.setColorAt(0.22, mixedColor(from.wash, to.wash, eased));
        first.setColorAt(0.66, QColor(255, 255, 255, 16));
        first.setColorAt(1.00, QColor(255, 255, 255, 0));
        painter.fillRect(rect, first);

        const QPointF rightWash(rect.left() + rect.width() * (0.78 + 0.06 * std::cos(orbit * 0.7)),
                                rect.top() + rect.height() * (0.48 + 0.07 * std::sin(orbit)));
        QRadialGradient second(rightWash, rect.width() * 0.52);
        second.setColorAt(0.00, accent);
        second.setColorAt(0.55, QColor(accent.red(), accent.green(), accent.blue(), 30));
        second.setColorAt(1.00, QColor(accent.red(), accent.green(), accent.blue(), 0));
        painter.fillRect(rect, second);

        const qreal sheenDrift = 0.5 + 0.5 * std::sin(orbit * 0.42);
        const qreal sheenSpread = 0.5 + 0.5 * std::sin(orbit * 0.42 + 1.35);
        QLinearGradient sheen(QPointF(rect.left() + rect.width() * (0.08 + sheenDrift * 0.38), rect.top()),
                              QPointF(rect.left() + rect.width() * (0.52 + sheenSpread * 0.34), rect.bottom()));
        sheen.setColorAt(0.00, QColor(255, 255, 255, 0));
        sheen.setColorAt(0.46, QColor(255, 255, 255, 28));
        sheen.setColorAt(0.56, QColor(255, 255, 255, 12));
        sheen.setColorAt(1.00, QColor(255, 255, 255, 0));
        painter.fillRect(rect, sheen);

        painter.setClipping(false);
        painter.setPen(QPen(QColor(255, 255, 255, 44), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect, 8, 8);
    }

private:
    struct ThemeColors {
        QColor a;
        QColor b;
        QColor c;
        QColor d;
        QColor wash;
        QColor accent;
    };

    static ThemeColors colorsForTheme(int theme)
    {
        if (theme == 1) {
            return {QColor(4, 111, 119), QColor(18, 82, 143), QColor(17, 41, 121), QColor(7, 3, 64),
                    QColor(115, 236, 222, 54), QColor(64, 235, 213, 88)};
        }
        if (theme == 2) {
            return {QColor(161, 45, 183), QColor(83, 20, 145), QColor(39, 5, 96), QColor(9, 2, 62),
                    QColor(255, 151, 238, 76), QColor(255, 62, 219, 90)};
        }
        return {QColor(161, 52, 9), QColor(134, 46, 18), QColor(91, 18, 86), QColor(23, 3, 61),
                QColor(255, 151, 238, 76), QColor(255, 128, 42, 88)};
    }

    qreal phase_ = 0.0;
    qreal themeBlend_ = 1.0;
    int sourceTheme_ = 0;
    int targetTheme_ = 0;
    int blendedThemeIndex_ = 0;
    QTimer *motionTimer_ = nullptr;
    QPropertyAnimation *themeAnimation_ = nullptr;
};

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
        if (objectName() == QStringLiteral("RootOverviewCard")) {
            return;
        }
        const int theme = qApp ? qApp->property("spaceGuardTheme").toInt() : 0;
        const QColor glassPanel(255, 255, 255, 20);
        const QColor glassPanelHover(255, 255, 255, 34);
        const QColor glassRow(255, 255, 255, 16);
        const QColor glassRowHover(255, 255, 255, 30);
        const QColor borderBase = theme == 1 ? QColor(77, 221, 213, 120)
                                : theme == 2 ? QColor(207, 142, 255, 118)
                                             : QColor(255, 165, 102, 126);
        const QColor borderHover = theme == 1 ? QColor(120, 255, 231, 178)
                                 : theme == 2 ? QColor(255, 116, 232, 176)
                                              : QColor(255, 196, 128, 182);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(28, 31, 36, 10 + static_cast<int>(10 * hover_)));
        painter.drawRoundedRect(rect.adjusted(1, 3 + hover_, -1, -1), 12, 12);

        QColor background = mixedColor(glassPanel, glassPanelHover, hover_);
        QColor border = mixedColor(borderBase, borderHover, hover_);
        if (objectName() == QStringLiteral("SmartSummaryCard")) {
            background = mixedColor(QColor(255, 255, 255, 18), QColor(255, 255, 255, 32), hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("SmartCarePanel")) {
            background = mixedColor(QColor(255, 255, 255, 14), QColor(255, 255, 255, 25), hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("SmartTaskCard")) {
            background = mixedColor(glassRow, glassRowHover, hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("BottomActionBar")) {
            background = mixedColor(QColor(255, 255, 255, 24), QColor(255, 255, 255, 38), hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("OptimizationTaskRow") || objectName() == QStringLiteral("AutostartActionRow")) {
            background = mixedColor(glassRow, glassRowHover, hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("TotalMetricRow")) {
            background = mixedColor(QColor(255, 255, 255, 20), QColor(255, 255, 255, 34), hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("ChildMetricRow")) {
            background = mixedColor(glassRow, glassRowHover, hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("InfoRow") || objectName() == QStringLiteral("ContainerRow")) {
            background = mixedColor(glassRow, glassRowHover, hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        } else if (objectName() == QStringLiteral("CardFrame")) {
            background = mixedColor(glassPanel, glassPanelHover, hover_);
            border = mixedColor(borderBase, borderHover, hover_);
        }
        painter.setBrush(background);
        painter.setPen(QPen(border, 1));
        painter.drawRoundedRect(rect.adjusted(0, 0, 0, -2), 12, 12);
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
        setMinimumHeight(40);
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
        const qreal tapScale = 1.0 - 0.04 * press_;
        if (tapScale < 0.999) {
            painter.translate(width() / 2.0, height() / 2.0);
            painter.scale(tapScale, tapScale);
            painter.translate(-width() / 2.0, -height() / 2.0);
        }

        const bool orb = objectName() == QStringLiteral("OrbButton");
        const bool nav = objectName() == QStringLiteral("NavButton");
        const bool navSelected = objectName() == QStringLiteral("NavSelected");
        const bool sideNav = nav || navSelected;
        const bool action = objectName() == QStringLiteral("ActionButton");
        const bool headerScan = objectName() == QStringLiteral("HeaderScanButton");
        const bool primary = objectName() == QStringLiteral("PrimaryButton") || navSelected || orb;
        const bool chrome = objectName() == QStringLiteral("ChromeButton") || objectName() == QStringLiteral("CloseButton");
        const bool trafficClose = objectName() == QStringLiteral("TrafficCloseButton");
        const bool trafficMin = objectName() == QStringLiteral("TrafficMinButton");
        const bool trafficMax = objectName() == QStringLiteral("TrafficMaxButton");
        const bool traffic = trafficClose || trafficMin || trafficMax;
        const bool close = objectName() == QStringLiteral("CloseButton");
        QColor base = primary ? QColor(178, 40, 224) : QColor(255, 255, 255);
        QColor hoverColor = primary ? QColor(222, 64, 238) : QColor(241, 245, 249);
        QColor pressColor = primary ? QColor(106, 32, 172) : QColor(227, 234, 242);
        QColor border = primary ? QColor(244, 124, 255) : QColor(203, 211, 221);
        QColor textColor = primary ? QColor(255, 255, 255) : QColor(29, 29, 31);
        if (chrome) {
            base = QColor(255, 255, 255, 0);
            hoverColor = close ? QColor(255, 69, 58, 34) : QColor(29, 31, 36, 18);
            pressColor = close ? QColor(255, 69, 58, 58) : QColor(29, 31, 36, 30);
            border = QColor(255, 255, 255, 0);
            textColor = close ? mixedColor(QColor(255, 255, 255, 220), QColor(255, 92, 86), hover_)
                              : QColor(255, 255, 255, 220);
        } else if (navSelected) {
            base = QColor(255, 180, 241, 48);
            hoverColor = QColor(255, 202, 247, 68);
            pressColor = QColor(255, 154, 236, 76);
            border = QColor(255, 255, 255, 44);
            textColor = QColor(255, 255, 255);
        } else if (nav) {
            base = QColor(255, 255, 255, 0);
            hoverColor = QColor(255, 255, 255, 34);
            pressColor = QColor(255, 255, 255, 54);
            border = QColor(255, 255, 255, 0);
            textColor = QColor(232, 222, 248);
        } else if (action) {
            base = QColor(255, 255, 255, 26);
            hoverColor = QColor(255, 255, 255, 42);
            pressColor = QColor(255, 255, 255, 56);
            border = QColor(255, 255, 255, 42);
            textColor = QColor(248, 244, 255);
        } else if (headerScan) {
            const int theme = qApp ? qApp->property("spaceGuardTheme").toInt() : 0;
            const QColor accent = theme == 1 ? QColor(66, 235, 213)
                                : theme == 2 ? QColor(223, 53, 255)
                                             : QColor(255, 145, 56);
            base = QColor(accent.red(), accent.green(), accent.blue(), 58);
            hoverColor = QColor(accent.red(), accent.green(), accent.blue(), 82);
            pressColor = QColor(accent.red(), accent.green(), accent.blue(), 104);
            border = QColor(255, 255, 255, 54);
            textColor = QColor(255, 255, 255);
        }

        if (!isEnabled()) {
            base = QColor(255, 255, 255, 20);
            hoverColor = base;
            pressColor = base;
            border = QColor(255, 255, 255, 28);
            textColor = QColor(255, 255, 255, 104);
        }

        if (traffic) {
            QColor dot = trafficClose ? QColor(255, 92, 86)
                         : trafficMin ? QColor(255, 189, 46)
                                      : QColor(39, 201, 63);
            dot = mixedColor(dot, QColor(255, 255, 255), hover_ * 0.16);
            const QRectF circle(width() / 2.0 - 6.5,
                                height() / 2.0 - 6.5 + press_,
                                13,
                                13);
            painter.setPen(QPen(QColor(255, 255, 255, 70), 1));
            painter.setBrush(dot);
            painter.drawEllipse(circle);
            return;
        }

        if (orb) {
            const QRectF glow = QRectF(4, 4 + press_ * 1.5, width() - 8, height() - 8);
            QRadialGradient halo(glow.center(), glow.width() * 0.62);
            halo.setColorAt(0.00, QColor(255, 45, 222, 170));
            halo.setColorAt(0.58, QColor(195, 55, 255, 92 + static_cast<int>(50 * hover_)));
            halo.setColorAt(1.00, QColor(34, 8, 84, 0));
            painter.setPen(Qt::NoPen);
            painter.setBrush(halo);
            painter.drawEllipse(glow.adjusted(-12, -12, 12, 12));

            QRadialGradient fillGradient(glow.topLeft(), glow.width());
            fillGradient.setColorAt(0.00, QColor(255, 84, 230));
            fillGradient.setColorAt(0.56, QColor(180, 35, 230));
            fillGradient.setColorAt(1.00, QColor(94, 28, 178));
            painter.setPen(QPen(QColor(255, 255, 255, 150), 2));
            painter.setBrush(fillGradient);
            painter.drawEllipse(glow);
            painter.setPen(QColor(255, 255, 255));
            QFont orbFont = font();
            orbFont.setBold(true);
            painter.setFont(orbFont);
            painter.drawText(glow.adjusted(12, 8, -12, -8), Qt::AlignCenter | Qt::TextWordWrap, text());
            return;
        }

        QColor fill = mixedColor(mixedColor(base, hoverColor, hover_), pressColor, press_);
        border = chrome ? border : mixedColor(border, QColor(111, 130, 150), hover_);

        const QRectF outerRect = QRectF(0.5, 0.5 + press_ * 1.0, width() - 1, height() - 1 - press_ * 1.0);
        QRectF rect = outerRect;
        if (sideNav) {
            const QSizeF highlightSize(54, 54);
            const qreal visualCenterOffset = property("navVisualOffset").toReal();
            rect = QRectF((width() - highlightSize.width()) / 2.0 + visualCenterOffset,
                          (height() - highlightSize.height()) / 2.0 + press_ * 1.0,
                          highlightSize.width(),
                          highlightSize.height());
        }
        if (!chrome && !sideNav) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(28, 31, 36, static_cast<int>(14 * hover_)));
            painter.drawRoundedRect(rect.adjusted(0, 2, 0, 2), 10, 10);
        }
        painter.setPen(QPen(border, 1));
        painter.setBrush(fill);
        const qreal radius = sideNav ? 12.0 : 10.0;
        painter.drawRoundedRect(rect, radius, radius);
        if (chrome) {
            const QString windowAction = property("windowAction").toString();
            if (!windowAction.isEmpty()) {
                painter.setPen(QPen(textColor, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                const QPointF c(rect.center().x(), rect.center().y() - 6);
                if (windowAction == QStringLiteral("minimize")) {
                    painter.drawLine(QPointF(c.x() - 5.5, c.y()), QPointF(c.x() + 5.5, c.y()));
                } else if (windowAction == QStringLiteral("maximize")) {
                    painter.drawRoundedRect(QRectF(c.x() - 5.0, c.y() - 5.0, 10.0, 10.0), 1.5, 1.5);
                } else if (windowAction == QStringLiteral("close")) {
                    painter.drawLine(QPointF(c.x() - 5.0, c.y() - 5.0), QPointF(c.x() + 5.0, c.y() + 5.0));
                    painter.drawLine(QPointF(c.x() + 5.0, c.y() - 5.0), QPointF(c.x() - 5.0, c.y() + 5.0));
                }
                return;
            }
        }

        const qreal padding = chrome ? 0.0 : 14.0;
        QRectF textRect = rect.adjusted(padding, 0, -padding, 0);
        painter.setPen(textColor);
        painter.setFont(font());
        if (sideNav) {
            const QString navKind = property("navKind").toString();
            if (!navKind.isEmpty()) {
                drawNavGlyph(painter, rect, navKind, textColor);
                return;
            }
        }
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
    void drawNavGlyph(QPainter &painter, const QRectF &rect, const QString &kind, const QColor &color)
    {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(color, 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        const QPointF c = rect.center();
        const QRectF box(c.x() - 13, c.y() - 13, 26, 26);
        if (kind == QStringLiteral("space")) {
            QPainterPath path;
            path.moveTo(c.x() - 11, c.y() + 7);
            path.lineTo(c.x() - 4, c.y() - 7);
            path.lineTo(c.x() + 11, c.y() - 7);
            path.lineTo(c.x() + 4, c.y() + 7);
            path.closeSubpath();
            painter.setBrush(QColor(color.red(), color.green(), color.blue(), 238));
            painter.setPen(Qt::NoPen);
            painter.drawPath(path);
        } else if (kind == QStringLiteral("apps")) {
            painter.setPen(QPen(color, 2.8, Qt::SolidLine, Qt::RoundCap));
            for (int i = 0; i < 3; ++i) {
                const qreal y = c.y() - 8 + i * 8;
                painter.drawLine(QPointF(c.x() - 10, y), QPointF(c.x() - 2, y));
                painter.drawLine(QPointF(c.x() + 5, y), QPointF(c.x() + 10, y));
            }
        } else if (kind == QStringLiteral("container")) {
            QPainterPath path;
            path.moveTo(c.x() - 12, c.y() + 6);
            path.lineTo(c.x() - 5, c.y() - 7);
            path.lineTo(c.x() + 12, c.y() - 7);
            path.lineTo(c.x() + 5, c.y() + 6);
            path.closeSubpath();
            painter.drawPath(path);
        } else if (kind == QStringLiteral("autostart")) {
            painter.setPen(QPen(color, 2.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(QPointF(c.x() - 9, c.y() + 9), QPointF(c.x() + 9, c.y() - 9));
            painter.drawLine(QPointF(c.x() + 9, c.y() - 9), QPointF(c.x() + 9, c.y() + 2));
            painter.drawLine(QPointF(c.x() + 9, c.y() - 9), QPointF(c.x() - 2, c.y() - 9));
        } else if (kind == QStringLiteral("scan")) {
            painter.setPen(QPen(color, 2.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawArc(box.adjusted(3, 3, -3, -3), 35 * 16, 285 * 16);
            QPainterPath head;
            head.moveTo(c.x() + 10, c.y() - 2);
            head.lineTo(c.x() + 14, c.y() + 5);
            head.lineTo(c.x() + 6, c.y() + 5);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawPath(head);
        }
        painter.restore();
    }

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

class LanguagePopupFrame : public QFrame {
    Q_OBJECT

public:
    explicit LanguagePopupFrame(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRectF panel = rect().adjusted(0.5, 0.5, -0.5, -0.5);
        painter.setPen(QPen(QColor(255, 255, 255, 58), 1.0));
        painter.setBrush(QColor(24, 18, 42, 132));
        painter.drawRoundedRect(panel, 10.0, 10.0);
    }
};

class GlassComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit GlassComboBox(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
        if (qApp) {
            qApp->installEventFilter(this);
        }
    }

    ~GlassComboBox() override
    {
        if (qApp) {
            qApp->removeEventFilter(this);
        }
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (!popup_ || !popup_->isVisible()) {
            return QComboBox::eventFilter(watched, event);
        }
        if (event->type() != QEvent::MouseButtonPress) {
            return QComboBox::eventFilter(watched, event);
        }
        auto *widget = qobject_cast<QWidget *>(watched);
        if (!widget) {
            return QComboBox::eventFilter(watched, event);
        }
        if (widget == this || isAncestorOf(widget) || widget == popup_ || popup_->isAncestorOf(widget)) {
            return QComboBox::eventFilter(watched, event);
        }
        hidePopup();
        return QComboBox::eventFilter(watched, event);
    }

    void showPopup() override
    {
        if (popup_ && popup_->isVisible()) {
            popup_->hide();
            return;
        }
        rebuildPopup();
        if (!popup_) {
            return;
        }
        const QPoint pos = mapTo(window(), QPoint(0, height() + 6));
        popup_->setFixedWidth(qMax(width(), 128));
        popup_->move(pos);
        popup_->show();
        popup_->raise();
    }

    void hidePopup() override
    {
        if (popup_) {
            popup_->hide();
        }
    }

private:
    void rebuildPopup()
    {
        if (popup_) {
            popup_->deleteLater();
            popup_ = nullptr;
        }
        QWidget *host = window();
        if (!host) {
            return;
        }
        auto *frame = new LanguagePopupFrame(host);
        frame->setObjectName(QStringLiteral("LanguagePopup"));
        auto *layout = new QVBoxLayout(frame);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(2);
        for (int i = 0; i < count(); ++i) {
            auto *button = new QPushButton((i == currentIndex() ? QStringLiteral("✓ ") : QStringLiteral("  ")) + itemText(i), frame);
            button->setObjectName(QStringLiteral("LanguagePopupItem"));
            button->setProperty("current", i == currentIndex());
            button->setCursor(Qt::PointingHandCursor);
            button->setMinimumHeight(34);
            connect(button, &QPushButton::clicked, this, [this, i]() {
                setCurrentIndex(i);
                hidePopup();
            });
            layout->addWidget(button);
        }
        frame->adjustSize();
        popup_ = frame;
    }

    QFrame *popup_ = nullptr;
};

class CleanerWindow : public QWidget {
    Q_OBJECT

public:
    CleanerWindow()
    {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
        user_ = qEnvironmentVariable("USER");
        if (user_.isEmpty()) {
            user_ = QStringLiteral("zengjianqi");
        }
        helper_ = QApplication::applicationDirPath() + QStringLiteral("/kylin-space-cleaner-helper");
        if (!QFileInfo::exists(helper_)) {
            helper_ = QApplication::applicationDirPath() + QStringLiteral("/../libexec/kylin-space-cleaner-helper");
        }

        setMinimumSize(1040, 760);
        buildUi();
        applyLanguage();
        resize(1040, 760);
        if (tabs_) {
            tabs_->setCurrentIndex(0);
        }
        applyPageTheme();
        QTimer::singleShot(100, this, [this]() {
            scanInternal(false);
        });
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF rect(0.5, 0.5, width() - 1, height() - 1);
        const int theme = qApp ? qApp->property("spaceGuardTheme").toInt() : 0;
        const QColor a = theme == 1 ? QColor(4, 111, 119)
                       : theme == 2 ? QColor(161, 45, 183)
                                    : QColor(161, 52, 9);
        const QColor b = theme == 1 ? QColor(7, 3, 64)
                       : theme == 2 ? QColor(9, 2, 62)
                                    : QColor(23, 3, 61);
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0.0, a);
        gradient.setColorAt(1.0, b);
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawRoundedRect(rect, 8, 8);
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
        QString kareBase;
        QString appPayload;
        QString systemOther;
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

    struct AutostartSelectionRow {
        QString id;
        bool currentlyEnabled = false;
        QCheckBox *box = nullptr;
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
            QStringLiteral("管理自启动项"),
            QStringLiteral("正在扫描"),
            QStringLiteral("正在读取系统空间、Kaiming 容器和启动项状态"),
            QStringLiteral("扫描结果"),
            QStringLiteral("执行选中优化"),
            QStringLiteral("重新扫描"),
            QStringLiteral("可优化条目"),
            QStringLiteral("旧版本容器清理"),
            QStringLiteral("自启动项优化"),
            QStringLiteral("项目"),
            QStringLiteral("当前占用"),
            QStringLiteral("可清理"),
            QStringLiteral("状态"),
            QStringLiteral("根分区总占用（不含 /home /data）"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree 写入层"),
            QStringLiteral("KARE 写入层"),
            QStringLiteral("其他根分区占用"),
            QStringLiteral("KARE base"),
            QStringLiteral("APP占用"),
            QStringLiteral("系统与缓存"),
            QStringLiteral("根分区总占用来自 / 文件系统本身，不包含独立挂载的 /home 和 /data；下面几项是已识别的子项或重点写入层，其他根分区占用包含系统基线、KARE base、普通应用目录、日志和缓存等未单独拆出的内容。"),
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
            QStringLiteral("管理自启动项"),
            QStringLiteral("选择要清理的旧版本容器"),
            QStringLiteral("当前没有发现可管理的自启动项。"),
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
            QStringLiteral("Manage Autostart Entries"),
            QStringLiteral("Scanning"),
            QStringLiteral("Reading system usage, Kaiming containers, and startup entries"),
            QStringLiteral("Scan Results"),
            QStringLiteral("Optimize Selected"),
            QStringLiteral("Rescan"),
            QStringLiteral("Optimization Items"),
            QStringLiteral("Old Container Cleanup"),
            QStringLiteral("Autostart Entry Optimization"),
            QStringLiteral("Item"),
            QStringLiteral("Current Usage"),
            QStringLiteral("Cleanable"),
            QStringLiteral("Status"),
            QStringLiteral("Root Total Used (excluding /home /data)"),
            QStringLiteral("Kaiming"),
            QStringLiteral("ostree Upper"),
            QStringLiteral("KARE Upper"),
            QStringLiteral("Other Root Usage"),
            QStringLiteral("KARE base"),
            QStringLiteral("App Usage"),
            QStringLiteral("System & Cache"),
            QStringLiteral("Root total usage comes from the / filesystem itself and excludes separately mounted /home and /data. The rows below are recognized child categories or key writable layers. Other root usage includes the system baseline, KARE base, ordinary app directories, logs, caches, and anything not broken out separately."),
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
            QStringLiteral("Manage autostart entries"),
            QStringLiteral("Select old container versions to clean"),
            QStringLiteral("No manageable autostart entries were found."),
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

    static constexpr int kButtonHeight = 40;
    static constexpr int kCompactButtonWidth = 88;
    static constexpr int kSecondaryButtonWidth = 108;
    static constexpr int kBackButtonWidth = 118;
    static constexpr int kActionButtonWidth = 136;
    static constexpr int kPrimaryButtonWidth = 156;
    static constexpr int kNavigationButtonWidth = 272;
    static constexpr int kToolbarButtonHeight = 50;
    static constexpr int kToolbarSecondaryButtonWidth = 132;
    static constexpr int kToolbarPrimaryButtonWidth = 178;

    static void applyButtonMetrics(QPushButton *button,
                                   int width = 0,
                                   bool fixedWidth = true,
                                   int height = kButtonHeight)
    {
        if (!button) {
            return;
        }
        button->setMinimumHeight(height);
        button->setMaximumHeight(height);
        button->setCursor(Qt::PointingHandCursor);
        if (width > 0) {
            button->setMinimumWidth(width);
            if (fixedWidth) {
                button->setMaximumWidth(width);
            }
        }
        button->setSizePolicy(fixedWidth ? QSizePolicy::Fixed : QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

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
        windowRoot->setContentsMargins(0, 0, 0, 0);
        windowRoot->setSpacing(0);

        appShell_ = new GlassShellFrame;
        appShell_->setObjectName(QStringLiteral("AppShellCard"));
        auto *shellRoot = new QVBoxLayout(appShell_);
        shellRoot->setContentsMargins(0, 0, 0, 0);
        shellRoot->setSpacing(0);
        windowRoot->addWidget(appShell_, 1);

        chrome_ = new QFrame;
        chrome_->setObjectName(QStringLiteral("ChromeBar"));
        chrome_->setFixedHeight(54);
        chrome_->installEventFilter(this);
        auto *chromeLayout = new QHBoxLayout(chrome_);
        chromeLayout->setContentsMargins(22, 8, 22, 8);
        chromeLayout->setSpacing(10);
        chromeTitle_ = new QLabel;
        chromeTitle_->setObjectName(QStringLiteral("ChromeTitle"));
        chromeTitle_->setAlignment(Qt::AlignCenter);
        minimizeButton_ = new AnimatedButton;
        maximizeButton_ = new AnimatedButton;
        closeButton_ = new AnimatedButton;
        minimizeButton_->setObjectName(QStringLiteral("ChromeButton"));
        maximizeButton_->setObjectName(QStringLiteral("ChromeButton"));
        closeButton_->setObjectName(QStringLiteral("CloseButton"));
        minimizeButton_->setProperty("windowAction", QStringLiteral("minimize"));
        maximizeButton_->setProperty("windowAction", QStringLiteral("maximize"));
        closeButton_->setProperty("windowAction", QStringLiteral("close"));
        minimizeButton_->setText(QString());
        maximizeButton_->setText(QString());
        closeButton_->setText(QString());
        QFont chromeButtonFont = minimizeButton_->font();
        chromeButtonFont.setBold(true);
        minimizeButton_->setFont(chromeButtonFont);
        maximizeButton_->setFont(chromeButtonFont);
        closeButton_->setFont(chromeButtonFont);
        const QSize chromeButtonSize(26, 22);
        minimizeButton_->setFixedSize(chromeButtonSize);
        maximizeButton_->setFixedSize(chromeButtonSize);
        closeButton_->setFixedSize(chromeButtonSize);
        connect(minimizeButton_, &QPushButton::clicked, this, &QWidget::showMinimized);
        connect(maximizeButton_, &QPushButton::clicked, this, [this]() {
            isMaximized() ? showNormal() : showMaximized();
        });
        connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
        auto *windowControlCard = new QFrame;
        windowControlCard->setObjectName(QStringLiteral("WindowControlCard"));
        windowControlCard->setFixedSize(100, 28);
        auto *windowControlLayout = new QHBoxLayout(windowControlCard);
        windowControlLayout->setContentsMargins(7, 0, 7, 6);
        windowControlLayout->setSpacing(4);
        windowControlLayout->addWidget(minimizeButton_, 0, Qt::AlignCenter);
        windowControlLayout->addWidget(maximizeButton_, 0, Qt::AlignCenter);
        windowControlLayout->addWidget(closeButton_, 0, Qt::AlignCenter);
        auto *chromeBalance = new QWidget;
        chromeBalance->setFixedWidth(112);
        chromeLayout->addWidget(chromeBalance);
        chromeLayout->addStretch(1);
        chromeLayout->addWidget(chromeTitle_, 0, Qt::AlignCenter);
        chromeLayout->addStretch(1);
        chromeLayout->addWidget(windowControlCard, 0, Qt::AlignVCenter);
        shellRoot->addWidget(chrome_);

        auto *content = new QWidget;
        content->setObjectName(QStringLiteral("ContentPane"));
        auto *shell = new QHBoxLayout(content);
        shell->setContentsMargins(14, 0, 14, 14);
        shell->setSpacing(14);
        shellRoot->addWidget(content, 1);

        sideBar_ = new QFrame;
        sideBar_->setObjectName(QStringLiteral("SideBar"));
        sideBar_->setFixedWidth(96);
        auto *navLayout = new QVBoxLayout(sideBar_);
        navLayout->setContentsMargins(10, 18, 10, 18);
        navLayout->setSpacing(14);
        statusNavButton_ = new AnimatedButton;
        appsNavButton_ = new AnimatedButton;
        containersNavButton_ = new AnimatedButton;
        autostartNavButton_ = new AnimatedButton;
        const auto configureNavButton = [](QPushButton *button, const QString &kind) {
            button->setObjectName(QStringLiteral("NavButton"));
            button->setProperty("navKind", kind);
            button->setProperty("navVisualOffset", -12.0);
            button->setText(QString());
            button->setFixedSize(64, 58);
        };
        configureNavButton(statusNavButton_, QStringLiteral("space"));
        configureNavButton(appsNavButton_, QStringLiteral("apps"));
        configureNavButton(containersNavButton_, QStringLiteral("container"));
        configureNavButton(autostartNavButton_, QStringLiteral("autostart"));
        statusNavButton_->setObjectName(QStringLiteral("NavSelected"));
        navLayout->addStretch(1);
        navLayout->addWidget(statusNavButton_, 0, Qt::AlignLeft);
        navLayout->addWidget(appsNavButton_, 0, Qt::AlignLeft);
        navLayout->addWidget(containersNavButton_, 0, Qt::AlignLeft);
        navLayout->addWidget(autostartNavButton_, 0, Qt::AlignLeft);
        navLayout->addStretch(1);
        shell->addWidget(sideBar_);

        auto *workspace = new QWidget;
        workspace->setObjectName(QStringLiteral("Workspace"));
        auto *root = new QVBoxLayout(workspace);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(12);
        shell->addWidget(workspace, 1);

        topBarFrame_ = new QFrame;
        topBarFrame_->setObjectName(QStringLiteral("TopBarFrame"));
        auto *header = new QVBoxLayout(topBarFrame_);
        header->setContentsMargins(18, 10, 18, 10);
        header->setSpacing(8);

        auto *topBar = new QHBoxLayout;
        topBar->setSpacing(12);
        title_ = new QLabel;
        title_->setObjectName(QStringLiteral("Title"));
        title_->setWordWrap(true);
        title_->setMinimumWidth(260);
        QFont titleFont = title_->font();
        titleFont.setPointSize(titleFont.pointSize() + 8);
        titleFont.setBold(true);
        title_->setFont(titleFont);
        topBar->addWidget(title_, 1);
        intro_ = new QLabel;
        intro_->hide();
        topScanButton_ = new AnimatedButton;
        topScanButton_->setObjectName(QStringLiteral("HeaderScanButton"));
        topScanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        topScanButton_->setIconSize(QSize(20, 20));
        applyButtonMetrics(topScanButton_, 150);
        topScanButton_->setMinimumHeight(42);
        topScanButton_->setMaximumHeight(42);
        topScanButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(topScanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);
        languageLabel_ = new QLabel;
        language_ = new GlassComboBox;
        language_->addItem(QStringLiteral("中文"), QStringLiteral("zh"));
        language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
        language_->setMinimumWidth(116);
        language_->setMinimumHeight(kButtonHeight);
        language_->setMaximumHeight(kButtonHeight);
        language_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        if (language_->view()) {
            language_->view()->setFrameShape(QFrame::NoFrame);
            language_->view()->setAutoFillBackground(false);
            language_->view()->setAttribute(Qt::WA_TranslucentBackground, true);
            language_->view()->viewport()->setAutoFillBackground(false);
            language_->view()->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
            if (QWidget *popup = language_->view()->window()) {
                popup->setAutoFillBackground(false);
                popup->setAttribute(Qt::WA_TranslucentBackground, true);
            }
        }
        connect(language_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CleanerWindow::applyLanguage);
        userLabel_ = new QLabel;
        userValue_ = new QLabel(user_);
        userValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        topBar->addWidget(topScanButton_, 0, Qt::AlignVCenter);
        topBar->addWidget(languageLabel_);
        topBar->addWidget(language_);
        topBar->addSpacing(12);
        topBar->addWidget(userLabel_);
        topBar->addWidget(userValue_);
        header->addLayout(topBar);

        auto *statusFrame = new QFrame;
        statusFrame->setObjectName(QStringLiteral("StatusFrame"));
        auto *statusLayout = new QVBoxLayout(statusFrame);
        statusLayout->setContentsMargins(12, 8, 12, 8);
        statusLayout->setSpacing(6);
        auto *statusLine = new QHBoxLayout;
        statusLine->setSpacing(8);
        statusTitle_ = new QLabel;
        statusTitle_->setObjectName(QStringLiteral("StatusTitle"));
        statusSummary_ = new QLabel;
        statusSummary_->setObjectName(QStringLiteral("StatusSummary"));
        statusSummary_->setWordWrap(true);
        statusLine->addWidget(statusTitle_);
        statusLine->addWidget(statusSummary_, 1);
        lastUpdate_ = new QLabel;
        lastUpdate_->setObjectName(QStringLiteral("LastUpdate"));
        progress_ = new QProgressBar;
        progress_->setObjectName(QStringLiteral("Progress"));
        progress_->setTextVisible(false);
        progress_->setMinimumWidth(260);
        progress_->setRange(0, 100);
        progress_->setValue(100);
        statusLayout->addLayout(statusLine);
        statusLayout->addWidget(lastUpdate_);
        statusLayout->addWidget(progress_);
        header->addWidget(statusFrame);
        root->addWidget(topBarFrame_);

        tabs_ = new QTabWidget;
        tabs_->setObjectName(QStringLiteral("MainTabs"));
        tabs_->setDocumentMode(true);
        tabs_->setIconSize(QSize(18, 18));
        tabs_->tabBar()->hide();
        connect(statusNavButton_, &QPushButton::clicked, this, [this]() {
            activeNavIndex_ = 0;
            primeTheme(0);
            if (tabs_) {
                QSignalBlocker blocker(tabs_);
                tabs_->setCurrentIndex(0);
            }
            showStatusSubPage(0);
        });
        connect(appsNavButton_, &QPushButton::clicked, this, [this]() {
            activeNavIndex_ = 1;
            primeTheme(1);
            if (tabs_) {
                QSignalBlocker blocker(tabs_);
                tabs_->setCurrentIndex(0);
            }
            showStatusSubPage(1);
        });
        connect(containersNavButton_, &QPushButton::clicked, this, [this]() {
            activeNavIndex_ = 2;
            primeTheme(2);
            if (tabs_) {
                QSignalBlocker blocker(tabs_);
                tabs_->setCurrentIndex(1);
            }
            activeReviewFilter_ = 1;
            if (state_.isEmpty()) {
                scanManual();
            } else {
                showScanResults(1);
            }
            updateMainNav(tabs_ ? tabs_->currentIndex() : 1);
        });
        connect(autostartNavButton_, &QPushButton::clicked, this, [this]() {
            activeNavIndex_ = 3;
            primeTheme(2);
            if (tabs_) {
                QSignalBlocker blocker(tabs_);
                tabs_->setCurrentIndex(1);
            }
            activeReviewFilter_ = 2;
            if (state_.isEmpty()) {
                scanManual();
            } else {
                showScanResults(2);
            }
            updateMainNav(tabs_ ? tabs_->currentIndex() : 1);
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
        applyButtonMetrics(metricsPageButton_, kPrimaryButtonWidth);
        applyButtonMetrics(appsPageButton_, kActionButtonWidth);
        statusSwitcher->addWidget(metricsPageButton_);
        statusSwitcher->addWidget(appsPageButton_);
        statusSwitcher->addStretch(1);
        auto *moduleSwitchFrame = new CardFrame;
        moduleSwitchFrame->setObjectName(QStringLiteral("SmartSummaryCard"));
        moduleSwitchFrame->setInteractive(false);
        auto *moduleSwitchLayout = new QHBoxLayout(moduleSwitchFrame);
        moduleSwitchLayout->setContentsMargins(14, 10, 14, 10);
        moduleSwitchLayout->setSpacing(10);
        moduleSwitchLayout->addLayout(statusSwitcher);
        statusPageLayout->addWidget(moduleSwitchFrame);
        moduleSwitchFrame->hide();

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

        auto *rootDetailsPage = new QWidget;
        rootDetailsPage->setObjectName(QStringLiteral("TabPage"));
        auto *rootDetailsLayout = new QVBoxLayout(rootDetailsPage);
        rootDetailsLayout->setContentsMargins(0, 0, 0, 0);
        rootDetailsLayout->setSpacing(12);

        statusStack_->addWidget(metricsPage);
        statusStack_->addWidget(appsPage);
        statusStack_->addWidget(rootDetailsPage);
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
        auto *contentLayer = new QFrame;
        contentLayer->setObjectName(QStringLiteral("ContentLayer"));
        auto *contentLayerLayout = new QGridLayout(contentLayer);
        contentLayerLayout->setContentsMargins(0, 0, 0, 0);
        contentLayerLayout->setSpacing(0);
        contentLayerLayout->addWidget(tabs_, 0, 0);
        busyOverlay_ = new QFrame;
        busyOverlay_->setObjectName(QStringLiteral("BusyOverlay"));
        busyOverlay_->hide();
        auto *busyOverlayLayout = new QVBoxLayout(busyOverlay_);
        busyOverlayLayout->setContentsMargins(0, 0, 0, 0);
        busyOverlayLayout->addStretch(1);
        contentLayerLayout->addWidget(busyOverlay_, 0, 0);
        root->addWidget(contentLayer, 1);

        auto *overallCard = new CardFrame;
        overallCard->setObjectName(QStringLiteral("RootOverviewCard"));
        overallCard->setInteractive(false);
        auto *overallLayout = new QVBoxLayout(overallCard);
        overallLayout->setContentsMargins(0, 0, 0, 0);
        overallLayout->setSpacing(0);
        overallTitle_ = new QLabel(overallCard);
        overallTitle_->setObjectName(QStringLiteral("SectionTitle"));
        overallTitle_->hide();
        auto *overallScroll = createCardList(&overallRows_, 64);
        overallRows_->setContentsMargins(0, 0, 0, 0);
        overallRows_->setSpacing(0);
        overallScroll->setMaximumHeight(72);
        overallScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        overallScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
        metricsRelation_ = new QLabel(metricsCard);
        metricsRelation_->setObjectName(QStringLiteral("Intro"));
        metricsRelation_->setWordWrap(true);
        metricsRelation_->hide();
        metricsSeries_ = new QPieSeries;
        metricsSeries_->setHoleSize(0.52);
        metricsSeries_->setPieSize(0.82);
        metricsChart_ = new QChart;
        metricsChart_->setTheme(QChart::ChartThemeDark);
        metricsChart_->addSeries(metricsSeries_);
        metricsChart_->legend()->setVisible(true);
        metricsChart_->legend()->setAlignment(Qt::AlignRight);
        metricsChart_->legend()->setLabelColor(QColor(246, 239, 255));
        metricsChart_->setBackgroundVisible(true);
        metricsChart_->setBackgroundBrush(QBrush(Qt::transparent));
        metricsChart_->setPlotAreaBackgroundVisible(false);
        metricsChart_->setPlotAreaBackgroundBrush(QBrush(Qt::transparent));
        metricsChart_->setBackgroundRoundness(0);
        metricsChart_->setMargins(QMargins(0, 0, 0, 0));
        metricsChartView_ = new QChartView(metricsChart_);
        metricsChartView_->setObjectName(QStringLiteral("MetricChart"));
        metricsChartView_->setAutoFillBackground(false);
        metricsChartView_->setAttribute(Qt::WA_TranslucentBackground, true);
        metricsChartView_->viewport()->setAutoFillBackground(false);
        metricsChartView_->viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
        metricsChartView_->setRenderHint(QPainter::Antialiasing);
        metricsChartView_->setMinimumHeight(250);
        metricsChartView_->setMaximumHeight(270);
        metricsLayout->addWidget(metricsChartView_);
        metricsLayout->addSpacing(18);
        rootDetailsButton_ = new AnimatedButton;
        rootDetailsButton_->setObjectName(QStringLiteral("ActionButton"));
        rootDetailsButton_->setMinimumHeight(52);
        rootDetailsButton_->setMaximumHeight(52);
        rootDetailsButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        rootDetailsButton_->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        rootDetailsButton_->setIconSize(QSize(18, 18));
        metricsLayout->addWidget(rootDetailsButton_, 1);
        metricsPageLayout->addWidget(metricsCard, 1);
        connect(rootDetailsButton_, &QPushButton::clicked, this, [this]() {
            if (!statusStack_) {
                return;
            }
            activeNavIndex_ = 0;
            statusStack_->setCurrentIndex(2);
            updateMainNav(tabs_ ? tabs_->currentIndex() : 0);
        });

        auto *rootDetailsHeader = new QHBoxLayout;
        rootDetailsHeader->setContentsMargins(0, 0, 0, 0);
        rootDetailsHeader->setSpacing(10);
        rootDetailsBackButton_ = new AnimatedButton;
        rootDetailsBackButton_->setObjectName(QStringLiteral("ActionButton"));
        applyButtonMetrics(rootDetailsBackButton_, kBackButtonWidth);
        rootDetailsBackButton_->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
        rootDetailsBackButton_->setIconSize(QSize(18, 18));
        rootDetailsHeader->addWidget(rootDetailsBackButton_, 0, Qt::AlignLeft);
        rootDetailsTitle_ = new QLabel;
        rootDetailsTitle_->setObjectName(QStringLiteral("SectionTitle"));
        rootDetailsHeader->addWidget(rootDetailsTitle_, 1);
        rootDetailsLayout->addLayout(rootDetailsHeader);
        auto *rootDetailsCard = new CardFrame;
        rootDetailsCard->setObjectName(QStringLiteral("CardFrame"));
        rootDetailsCard->setInteractive(false);
        auto *rootDetailsCardLayout = new QVBoxLayout(rootDetailsCard);
        rootDetailsCardLayout->setContentsMargins(16, 14, 16, 16);
        rootDetailsCardLayout->setSpacing(10);
        auto *metricsScroll = createCardList(&metricsRows_, 520);
        metricsList_ = metricsScroll->widget();
        rootDetailsCardLayout->addWidget(metricsScroll);
        rootDetailsLayout->addWidget(rootDetailsCard, 1);
        connect(rootDetailsBackButton_, &QPushButton::clicked, this, [this]() {
            showStatusSubPage(0);
        });

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
        applyButtonMetrics(backToAppsButton_, kBackButtonWidth);
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
        actionsCard->setObjectName(QStringLiteral("SmartCarePanel"));
        actionsCard->setInteractive(false);
        actionsCard->setMinimumHeight(620);
        auto *actionsCardLayout = new QVBoxLayout(actionsCard);
        actionsCardLayout->setContentsMargins(34, 28, 34, 28);
        actionsCardLayout->setSpacing(18);
        actionsTitle_ = new QLabel;
        actionsTitle_->setObjectName(QStringLiteral("SmartCareTitle"));
        actionsTitle_->setAlignment(Qt::AlignCenter);
        actionsCardLayout->addWidget(actionsTitle_);
        auto *launchVisual = new SpaceVisual;
        actionsCardLayout->addWidget(launchVisual, 0, Qt::AlignHCenter);
        auto makeSmartTask = [this](const QString &title, const QString &detail) {
            auto *tile = new CardFrame;
            tile->setObjectName(QStringLiteral("SmartTaskCard"));
            tile->setMinimumSize(210, 118);
            tile->setInteractive(true);
            auto *tileLayout = new QVBoxLayout(tile);
            tileLayout->setContentsMargins(14, 12, 14, 12);
            tileLayout->setSpacing(8);
            auto *top = new QHBoxLayout;
            top->setSpacing(8);
            auto *mark = new QLabel(QStringLiteral("✓"));
            mark->setObjectName(QStringLiteral("TaskMark"));
            mark->setAlignment(Qt::AlignCenter);
            mark->setFixedSize(22, 22);
            top->addWidget(mark);
            auto *titleLabel = makeLabel(title, QStringLiteral("RowTitle"));
            top->addWidget(titleLabel, 1);
            tileLayout->addLayout(top);
            auto *detailLabel = makeLabel(detail, QStringLiteral("PathValue"));
            detailLabel->setWordWrap(true);
            tileLayout->addWidget(detailLabel);
            tileLayout->addStretch(1);
            return tile;
        };
        auto *taskGrid = new QGridLayout;
        taskGrid->setContentsMargins(0, 0, 0, 0);
        taskGrid->setHorizontalSpacing(12);
        taskGrid->setVerticalSpacing(12);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("Kaiming 旧版本容器"),
                                          QStringLiteral("选择可安全回收的旧层版本")), 0, 0);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("自启动项"),
                                          QStringLiteral("逐项关闭可优化的启动项")), 0, 1);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("应用容器占用"),
                                          QStringLiteral("查看每个应用的容器体积")), 0, 2);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("根分区空间"),
                                          QStringLiteral("保持清理前后的空间对照")), 1, 0);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("执行计划"),
                                          QStringLiteral("确认后再移动到回滚隔离区")), 1, 1);
        taskGrid->addWidget(makeSmartTask(QStringLiteral("结果记录"),
                                          QStringLiteral("每个步骤都有状态与错误详情")), 1, 2);
        actionsCardLayout->addLayout(taskGrid);
        scanButton_ = new AnimatedButton;
        scanButton_->setObjectName(QStringLiteral("OrbButton"));
        scanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        scanButton_->setIconSize(QSize(22, 22));
        scanButton_->setFixedSize(132, 132);
        actionsCardLayout->addWidget(scanButton_, 0, Qt::AlignHCenter);
        scanLaunchLayout->addWidget(actionsCard);
        scanLaunchLayout->addStretch(1);
        connect(scanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);

        auto *scanProgressPage = new QWidget;
        scanProgressPage->setObjectName(QStringLiteral("TabPage"));
        auto *scanProgressLayout = new QVBoxLayout(scanProgressPage);
        scanProgressLayout->setContentsMargins(0, 0, 0, 0);
        scanProgressLayout->setSpacing(12);
        auto *progressCard = new CardFrame;
        progressCard->setObjectName(QStringLiteral("SmartCarePanel"));
        progressCard->setInteractive(false);
        auto *progressLayout = new QVBoxLayout(progressCard);
        progressLayout->setContentsMargins(34, 30, 34, 30);
        progressLayout->setSpacing(16);
        visual_ = new SpaceVisual;
        progressLayout->addWidget(visual_, 0, Qt::AlignHCenter);
        scanProgressTitle_ = new QLabel;
        scanProgressTitle_->setObjectName(QStringLiteral("SmartCareTitle"));
        scanProgressTitle_->setAlignment(Qt::AlignCenter);
        scanProgressDetail_ = new QLabel;
        scanProgressDetail_->setObjectName(QStringLiteral("Intro"));
        scanProgressDetail_->setWordWrap(true);
        scanProgressDetail_->setAlignment(Qt::AlignCenter);
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
        resultCard->setObjectName(QStringLiteral("SmartCarePanel"));
        resultCard->setInteractive(false);
        auto *resultLayout = new QVBoxLayout(resultCard);
        resultLayout->setContentsMargins(24, 22, 24, 24);
        resultLayout->setSpacing(14);
        resultTitle_ = new QLabel;
        resultTitle_->setObjectName(QStringLiteral("SmartCareTitle"));
        resultTitle_->setAlignment(Qt::AlignCenter);
        resultLayout->addWidget(resultTitle_);

        auto *resultContent = new QHBoxLayout;
        resultContent->setSpacing(12);

        auto *summaryCard = new CardFrame;
        summaryCard->setObjectName(QStringLiteral("SmartSummaryCard"));
        summaryCard->setInteractive(false);
        summaryCard->setMinimumWidth(300);
        summaryCard->setMaximumWidth(360);
        resultSummaryCard_ = summaryCard;
        auto *summaryLayout = new QVBoxLayout(summaryCard);
        summaryLayout->setContentsMargins(16, 14, 16, 16);
        summaryLayout->setSpacing(10);
        resultSummary_ = new QLabel;
        resultSummary_->setObjectName(QStringLiteral("Intro"));
        resultSummary_->setWordWrap(true);
        summaryLayout->addWidget(resultSummary_);
        selectionSummary_ = new QLabel;
        selectionSummary_->setObjectName(QStringLiteral("Intro"));
        selectionSummary_->setWordWrap(true);
        summaryLayout->addWidget(selectionSummary_);
        auto *selectionButtons = new QHBoxLayout;
        selectionButtons->setSpacing(8);
        selectAllButton_ = new QPushButton;
        clearSelectionButton_ = new QPushButton;
        selectAllButton_->setObjectName(QStringLiteral("TaskDetailButton"));
        clearSelectionButton_->setObjectName(QStringLiteral("TaskDetailButton"));
        applyButtonMetrics(selectAllButton_, kSecondaryButtonWidth);
        applyButtonMetrics(clearSelectionButton_, kSecondaryButtonWidth);
        selectionButtons->addWidget(selectAllButton_);
        selectionButtons->addWidget(clearSelectionButton_);
        summaryLayout->addLayout(selectionButtons);
        auto *resultActions = new QVBoxLayout;
        resultActions->setSpacing(10);
        applyOptimizationsButton_ = new AnimatedButton;
        rescanButton_ = new AnimatedButton;
        applyOptimizationsButton_->setObjectName(QStringLiteral("PrimaryButton"));
        rescanButton_->setObjectName(QStringLiteral("ActionButton"));
        applyOptimizationsButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        rescanButton_->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
        applyOptimizationsButton_->setIconSize(QSize(18, 18));
        rescanButton_->setIconSize(QSize(18, 18));
        applyButtonMetrics(applyOptimizationsButton_, kPrimaryButtonWidth, false);
        applyButtonMetrics(rescanButton_, kActionButtonWidth, false);
        resultActions->addWidget(applyOptimizationsButton_);
        resultActions->addWidget(rescanButton_);
        summaryLayout->addLayout(resultActions);
        summaryLayout->addStretch(1);
        resultContent->addWidget(summaryCard);

        auto *optimizationScroll = createCardList(&optimizationRows_, 330);
        optimizationScroll->setMinimumHeight(430);
        optimizationScrollArea_ = optimizationScroll;
        optimizationList_ = optimizationScroll->widget();
        resultContent->addWidget(optimizationScroll, 1);
        resultLayout->addLayout(resultContent, 1);
        secondaryToolbarHost_ = new QWidget;
        secondaryToolbarHost_->setObjectName(QStringLiteral("SecondaryToolbarHost"));
        auto *secondaryToolbarLayout = new QVBoxLayout(secondaryToolbarHost_);
        secondaryToolbarLayout->setContentsMargins(0, 2, 0, 0);
        secondaryToolbarLayout->setSpacing(0);
        secondaryToolbarLayout_ = secondaryToolbarLayout;
        secondaryToolbarHost_->hide();
        resultLayout->addWidget(secondaryToolbarHost_);
        scanResultLayout->addWidget(resultCard, 1);
        connect(applyOptimizationsButton_, &QPushButton::clicked, this, &CleanerWindow::applySelectedOptimizations);
        connect(rescanButton_, &QPushButton::clicked, this, &CleanerWindow::scanManual);
        connect(selectAllButton_, &QPushButton::clicked, this, &CleanerWindow::selectAllOptimizations);
        connect(clearSelectionButton_, &QPushButton::clicked, this, &CleanerWindow::clearOptimizationSelection);

        auto *actionResultPage = new QWidget;
        actionResultPage->setObjectName(QStringLiteral("TabPage"));
        auto *actionResultLayout = new QVBoxLayout(actionResultPage);
        actionResultLayout->setContentsMargins(0, 0, 0, 0);
        actionResultLayout->setSpacing(12);

        auto *actionResultHeader = new CardFrame;
        actionResultHeader->setObjectName(QStringLiteral("SmartSummaryCard"));
        actionResultHeader->setInteractive(false);
        auto *actionResultHeaderLayout = new QHBoxLayout(actionResultHeader);
        actionResultHeaderLayout->setContentsMargins(16, 12, 16, 12);
        actionResultHeaderLayout->setSpacing(12);
        auto *actionResultHint = makeLabel(language_->currentData().toString() == QStringLiteral("en")
                                               ? QStringLiteral("Review the latest operation results here, then return to the optimization page.")
                                               : QStringLiteral("在这里查看本次操作结果，然后返回优化页面。"),
                                           QStringLiteral("PathValue"));
        actionResultHint->setWordWrap(true);
        actionResultHeaderLayout->addWidget(actionResultHint, 1);
        auto *backFromActionResultButton = new QPushButton;
        backFromActionResultButton->setObjectName(QStringLiteral("TaskDetailButton"));
        backFromActionResultButton->setText(language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Back")
            : QStringLiteral("返回"));
        applyButtonMetrics(backFromActionResultButton, kBackButtonWidth);
        actionResultHeaderLayout->addWidget(backFromActionResultButton);
        actionResultLayout->addWidget(actionResultHeader);
        connect(backFromActionResultButton, &QPushButton::clicked, this, [this]() {
            showScanResults(activeReviewFilter_);
        });

        auto *planCard = new CardFrame;
        planCard->setObjectName(QStringLiteral("CardFrame"));
        planCard->setInteractive(false);
        planCard_ = planCard;
        auto *planLayout = new QVBoxLayout(planCard);
        planLayout->setContentsMargins(16, 14, 16, 16);
        planLayout->setSpacing(10);
        planTitle_ = new QLabel;
        planTitle_->setObjectName(QStringLiteral("SectionTitle"));
        planLayout->addWidget(planTitle_);
        auto *planScroll = createCardList(&planRows_, 120);
        planList_ = planScroll->widget();
        planLayout->addWidget(planScroll, 1);
        actionResultLayout->addWidget(planCard, 1);

        scanStack_->addWidget(scanLaunchPage);
        scanStack_->addWidget(scanProgressPage);
        scanStack_->addWidget(scanResultPage);
        scanStack_->addWidget(actionResultPage);

        setStyleSheet(QStringLiteral(R"(
            QWidget#AppRoot { background: transparent; color: #f8f4ff; }
            QFrame#AppShellCard {
                border-radius: 8px;
                border: 0;
                background: transparent;
            }
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
            QFrame#ChromeBar { background: transparent; min-height: 42px; }
            QLabel#ChromeTitle { color: #eadcff; font-weight: 800; font-size: 15px; }
            QFrame#WindowControlCard {
                border-radius: 8px;
                border: 1px solid rgba(255, 255, 255, 0.16);
                background: rgba(255, 255, 255, 0.08);
            }
            QFrame#SideBar {
                border-radius: 8px;
                border: 1px solid rgba(255, 255, 255, 0.10);
                background: rgba(20, 4, 72, 0.34);
            }
            QLabel#SideBrand {
                border-radius: 8px;
                color: #ffffff;
                background: qradialgradient(cx:0.42, cy:0.32, radius:0.72,
                    stop:0 #ff6ff2, stop:0.52 #b332e6, stop:1 #3a158b);
                font-weight: 900;
                font-size: 22px;
            }
            QWidget#Workspace { background: transparent; }
            QFrame#ContentLayer { background: transparent; }
            QFrame#BusyOverlay {
                border-radius: 8px;
                background: rgba(0, 0, 0, 0.32);
            }
            QFrame#TopBarFrame {
                border-radius: 8px;
                border: 1px solid rgba(255, 255, 255, 0.14);
                background: rgba(255, 255, 255, 0.08);
            }
            QLabel#Title { color: #ffffff; letter-spacing: 0px; }
            QFrame#TopBarFrame QLabel { color: #f7edff; }
            QLabel#Intro { color: #d6c9e8; font-size: 14px; line-height: 150%; }
            QFrame#StatusFrame {
                border: 1px solid rgba(255, 255, 255, 0.16);
                border-radius: 8px;
                background: rgba(255, 255, 255, 0.08);
                color: #ffffff;
            }
            QLabel#StatusTitle { color: #ffffff; font-weight: 700; }
            QLabel#StatusSummary { color: #f7edff; font-weight: 600; }
            QLabel#LastUpdate { color: #d6c9e8; }
            QFrame#NavFrame {
                border: 1px solid rgba(199, 113, 255, 0.36);
                border-radius: 8px;
                background: rgba(255, 255, 255, 0.08);
            }
            QLabel#SectionTitle {
                color: #ffffff;
                font-size: 15px;
                font-weight: 700;
            }
            QLabel#SmartCareTitle {
                color: #ffffff;
                font-size: 28px;
                font-weight: 900;
            }
            QLabel#RowTitle {
                color: #ffffff;
                font-size: 15px;
                font-weight: 800;
            }
            QFrame#InfoRow QLabel#RowTitle,
            QFrame#SmartTaskCard QLabel#RowTitle,
            QFrame#SmartSummaryCard QLabel#RowTitle,
            QFrame#OptimizationTaskRow QLabel#RowTitle,
            QFrame#AutostartActionRow QLabel#RowTitle,
            QFrame#ContainerRow QLabel#RowTitle {
                color: #ffffff;
            }
            QLabel#TaskMark {
                border-radius: 8px;
                background: #24e6b8;
                color: #102237;
                font-weight: 900;
            }
            QFrame#SmartTaskCard QLabel#PathValue,
            QFrame#InfoRow QLabel#PathValue,
            QFrame#SmartSummaryCard QLabel#PathValue,
            QFrame#BottomActionBar QLabel#PathValue {
                color: #d8c9ea;
            }
            QFrame#SmartSummaryCard QLabel#Intro,
            QFrame#OptimizationTaskRow QLabel#PathValue,
            QFrame#OptimizationTaskRow QLabel#PillLabel,
            QFrame#OptimizationTaskRow QLabel#PillValue,
            QFrame#AutostartActionRow QLabel#PillLabel,
            QFrame#AutostartActionRow QLabel#PillValue {
                color: #f7edff;
            }
            QFrame#OptimizationTaskRow QLabel#PathValue {
                color: #d8c9ea;
            }
            QFrame#ValuePill {
                border: 1px solid rgba(255, 255, 255, 0.16);
                border-radius: 9px;
                background: rgba(255, 255, 255, 0.10);
                min-height: 42px;
            }
            QLabel#PillLabel {
                color: #cfc0ea;
                font-size: 11px;
                font-weight: 700;
            }
            QLabel#PillValue {
                color: #ffffff;
                font-size: 14px;
                font-weight: 800;
            }
            QFrame#PathPill {
                border: 1px solid rgba(255, 255, 255, 0.16);
                border-radius: 9px;
                background: rgba(255, 255, 255, 0.08);
            }
            QLabel#PathValue {
                color: #d8c9ea;
                font-size: 12px;
                font-weight: 650;
                line-height: 140%;
            }
            QProgressBar#Progress {
                border: 1px solid rgba(255, 255, 255, 0.18);
                border-radius: 6px;
                background: rgba(255, 255, 255, 0.12);
                min-height: 10px;
                max-height: 10px;
            }
            QProgressBar#Progress::chunk {
                border-radius: 6px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #df35ff, stop:1 #42ebd5);
            }
            QProgressBar#Progress[busy="true"] {
                border: 1px solid rgba(255, 255, 255, 0.42);
                background: rgba(255, 255, 255, 0.18);
            }
            QProgressBar#Progress[busy="true"]::chunk {
                border-radius: 6px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #ff7af5, stop:0.46 #df35ff, stop:1 #42fff0);
            }
            QProgressBar#FlowProgress {
                border: 1px solid rgba(255, 255, 255, 0.18);
                border-radius: 8px;
                background: rgba(255, 255, 255, 0.12);
                min-height: 16px;
                max-height: 16px;
                text-align: center;
                color: transparent;
            }
            QProgressBar#FlowProgress::chunk {
                border-radius: 8px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #df35ff, stop:0.48 #a936ee, stop:1 #42ebd5);
            }
            QChartView#MetricChart {
                border: 0;
                border-radius: 8px;
                background: transparent;
            }
            QFrame#MetricRail {
                background: transparent;
            }
            QFrame#MetricRailLine {
                background: rgba(255, 255, 255, 0.22);
                border-radius: 1px;
            }
            QProgressBar#TotalMetricBar,
            QProgressBar#ChildMetricBar {
                border: 0;
                border-radius: 4px;
                background: rgba(255, 255, 255, 0.12);
                min-height: 8px;
                max-height: 8px;
            }
            QProgressBar#TotalMetricBar::chunk {
                border-radius: 4px;
                background: #df35ff;
            }
            QProgressBar#ChildMetricBar::chunk {
                border-radius: 4px;
                background: #42ebd5;
            }
            QCheckBox {
                color: #ffffff;
                font-weight: 750;
                spacing: 10px;
                min-height: 40px;
            }
            QFrame#OptimizationTaskRow QCheckBox,
            QFrame#SmartSummaryCard QCheckBox {
                color: #ffffff;
            }
            QCheckBox::indicator {
                width: 18px;
                height: 18px;
                border-radius: 5px;
                border: 1px solid rgba(255, 255, 255, 0.30);
                background: rgba(255, 255, 255, 0.12);
            }
            QCheckBox::indicator:checked {
                background: #42ebd5;
                border-color: #42ebd5;
            }
            QCheckBox::indicator:disabled {
                background: #eef2f6;
                border-color: #d9dfe7;
            }
            QCheckBox#TaskCheckBox {
                font-size: 14px;
                font-weight: 800;
            }
            QPushButton#TaskDetailButton {
                min-height: 40px;
                max-height: 40px;
                padding: 0 14px;
                border-radius: 10px;
                border: 1px solid rgba(255, 255, 255, 0.26);
                background: rgba(255, 255, 255, 0.16);
                color: #ffffff;
                font-weight: 760;
            }
            QPushButton#TaskDetailButton:hover {
                background: rgba(255, 255, 255, 0.24);
                border-color: rgba(255, 255, 255, 0.46);
            }
            QPushButton[toolbarButton="true"] {
                min-height: 50px;
                max-height: 50px;
                padding: 0 22px;
                border-radius: 12px;
                font-size: 15px;
                font-weight: 820;
            }
            QMenu {
                border: 1px solid #cbd3dd;
                border-radius: 8px;
                background: #ffffff;
                color: #1d1d1f;
                padding: 6px;
            }
            QMenu::item {
                min-height: 28px;
                padding: 4px 22px 4px 10px;
                border-radius: 6px;
            }
            QMenu::item:selected {
                background: #e8eef5;
                color: #1d1d1f;
            }
            QMenu::item:disabled {
                color: #a4a9b1;
            }
            QComboBox {
                min-height: 40px;
                max-height: 40px;
                padding: 0 30px 0 12px;
                border-radius: 10px;
                border: 1px solid rgba(255, 255, 255, 0.20);
                background: rgba(255, 255, 255, 0.12);
                color: #ffffff;
                font-weight: 600;
            }
            QComboBox:hover {
                border-color: rgba(255, 255, 255, 0.34);
                background: rgba(255, 255, 255, 0.18);
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
                border: 1px solid rgba(255, 255, 255, 0.16);
                border-radius: 10px;
                background: transparent;
                color: #ffffff;
                selection-background-color: transparent;
                selection-color: #ffffff;
                outline: 0;
                padding: 6px;
            }
            QComboBoxPrivateContainer {
                border: 1px solid rgba(255, 255, 255, 0.16);
                border-radius: 10px;
                background: transparent;
            }
            QComboBoxPrivateContainer QWidget {
                background: transparent;
            }
            QComboBox QAbstractItemView::item {
                min-height: 34px;
                padding: 0 12px;
                border-radius: 8px;
                background: transparent;
            }
            QComboBox QAbstractItemView::item:selected {
                background: rgba(255, 255, 255, 0.16);
            }
            QFrame#LanguagePopup {
                border: 1px solid rgba(255, 255, 255, 0.18);
                border-radius: 10px;
                background: rgba(24, 18, 42, 0.34);
            }
            QPushButton#LanguagePopupItem {
                min-height: 34px;
                max-height: 34px;
                padding: 0 12px;
                border: 0;
                border-radius: 8px;
                background: transparent;
                color: #ffffff;
                text-align: left;
                font-weight: 780;
            }
            QPushButton#LanguagePopupItem[current="true"],
            QPushButton#LanguagePopupItem:hover {
                background: rgba(255, 255, 255, 0.16);
            }
            QPushButton {
                min-height: 40px;
                max-height: 40px;
                padding: 0 16px;
                border-radius: 10px;
                border: 1px solid rgba(255, 255, 255, 0.18);
                background: rgba(255, 255, 255, 0.12);
                color: #ffffff;
                font-weight: 600;
            }
            QPushButton:hover { background: rgba(255, 255, 255, 0.20); border-color: rgba(255, 255, 255, 0.32); }
            QPushButton:pressed { background: rgba(255, 255, 255, 0.26); }
            QPushButton#PrimaryButton {
                color: #ffffff;
                border: 1px solid rgba(255, 255, 255, 0.24);
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #ff67dc, stop:0.48 #c82af2, stop:1 #7f44ff);
            }
            QPushButton#ActionButton {
                color: #f8f4ff;
                border: 1px solid rgba(255, 255, 255, 0.24);
                background: rgba(255, 255, 255, 0.14);
            }
            QPushButton#PrimaryButton:hover,
            QPushButton#NavSelected:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #ff83e7, stop:0.48 #df35ff, stop:1 #8e5cff);
            }
            QPushButton#OrbButton {
                min-width: 118px;
                min-height: 118px;
                max-width: 132px;
                max-height: 132px;
                padding: 0;
                border: 0;
                background: transparent;
                color: #ffffff;
                font-weight: 900;
            }
            QPushButton#NavButton {
                min-height: 58px;
                min-width: 64px;
                max-height: 58px;
                max-width: 64px;
                border-radius: 8px;
                background: transparent;
                border-color: transparent;
                color: #e8def8;
            }
            QPushButton#NavSelected {
                min-height: 58px;
                min-width: 64px;
                max-height: 58px;
                max-width: 64px;
                border-radius: 8px;
                color: #ffffff;
                border: 1px solid rgba(255, 255, 255, 0.26);
                background: #c82af2;
            }
            QPushButton:disabled {
                color: rgba(255, 255, 255, 0.42);
                background: rgba(255, 255, 255, 0.08);
                border-color: rgba(255, 255, 255, 0.10);
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
        if (topScanButton_) {
            topScanButton_->setText(text.rescan);
            topScanButton_->setToolTip(text.autoRefresh);
        }
        if (statusNavButton_) {
            statusNavButton_->setText(QString());
            statusNavButton_->setToolTip(text.metricsTitle);
        }
        if (appsNavButton_) {
            appsNavButton_->setText(QString());
            appsNavButton_->setToolTip(text.applicationsTitle);
        }
        if (containersNavButton_) {
            containersNavButton_->setText(QString());
            containersNavButton_->setToolTip(text.containerCleanup);
        }
        if (autostartNavButton_) {
            autostartNavButton_->setText(QString());
            autostartNavButton_->setToolTip(text.autostartOptimization);
        }
        if (scanButton_) {
            scanButton_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Scan")
                : QStringLiteral("扫描"));
            scanButton_->setToolTip(text.startScan);
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
        if (rootDetailsButton_) {
            rootDetailsButton_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("View Details")
                : QStringLiteral("查看明细"));
        }
        if (rootDetailsBackButton_) {
            rootDetailsBackButton_->setText(text.back);
        }
        if (rootDetailsTitle_) {
            rootDetailsTitle_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Space Usage Details")
                : QStringLiteral("空间占用明细"));
        }
        if (appsPageButton_) {
            appsPageButton_->setText(text.applicationsTitle);
        }
        if (backToAppsButton_) {
            backToAppsButton_->setText(text.back);
        }
        if (actionsTitle_) {
            actionsTitle_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Smart Care")
                : QStringLiteral("智能清理"));
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
        if (selectionSummary_) {
            updateOptimizationSelectionState();
        }
        if (planTitle_) {
            planTitle_->setText(text.resultTitle);
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Run")
                : QStringLiteral("运行"));
            applyOptimizationsButton_->setToolTip(text.optimizeSelected);
        }
        if (rescanButton_) {
            rescanButton_->setText(text.rescan);
        }
        if (selectAllButton_) {
            selectAllButton_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Select all")
                : QStringLiteral("全选"));
        }
        if (clearSelectionButton_) {
            clearSelectionButton_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Clear")
                : QStringLiteral("清空"));
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
        group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
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
        layout->addWidget(createValuePill(text.before, usage), 1, Qt::AlignVCenter);
        layout->addWidget(createValuePill(text.released, cleanable), 1, Qt::AlignVCenter);
        layout->addWidget(createValuePill(total ? text.metricStatus : text.shareOfRoot, total ? status : percent), 1, Qt::AlignVCenter);
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
            QFrame *pill = createValuePill(labels.value(i), values.at(i));
            if (values.size() == 1) {
                pill->setMinimumWidth(300);
                layout->addWidget(pill, 2, Qt::AlignVCenter);
            } else {
                layout->addWidget(pill, 1, Qt::AlignVCenter);
            }
        }
        return row;
    }

    ClickableCardFrame *createNavigationActionRow(const QString &title,
                                                  const QString &detail,
                                                  const QString &buttonText,
                                                  const std::function<void()> &action,
                                                  int buttonWidth = kActionButtonWidth)
    {
        auto *row = new ClickableCardFrame;
        row->setObjectName(QStringLiteral("InfoRow"));
        row->setInteractive(true);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 10, 14, 10);
        layout->setSpacing(12);

        auto *main = new QVBoxLayout;
        main->setSpacing(6);
        main->addWidget(makeLabel(title, QStringLiteral("RowTitle")));
        auto *detailLabel = makeLabel(detail, QStringLiteral("PathValue"));
        detailLabel->setWordWrap(true);
        main->addWidget(detailLabel);
        layout->addLayout(main, 1);

        auto *button = new QPushButton(buttonText);
        button->setObjectName(QStringLiteral("TaskDetailButton"));
        applyButtonMetrics(button, buttonWidth);
        layout->addWidget(button, 0, Qt::AlignVCenter);

        connect(row, &ClickableCardFrame::clicked, this, action);
        connect(button, &QPushButton::clicked, this, action);
        return row;
    }

    CardFrame *createSecondaryAutostartToolbar()
    {
        auto *row = new CardFrame;
        row->setObjectName(QStringLiteral("BottomActionBar"));
        row->setInteractive(false);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(18, 14, 18, 14);
        layout->setSpacing(14);

        auto *summary = makeLabel(language_->currentData().toString() == QStringLiteral("en")
                                      ? QStringLiteral("Select entries, then run the chosen disable or restore changes.")
                                      : QStringLiteral("选择启动项后，执行对应的禁用或还原变更。"),
                                  QStringLiteral("PathValue"));
        summary->setWordWrap(true);
        layout->addWidget(summary, 1);

        secondarySelectAllButton_ = new QPushButton(language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Select all")
            : QStringLiteral("全选"));
        secondaryClearSelectionButton_ = new QPushButton(language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Clear")
            : QStringLiteral("清空"));
        secondaryApplyButton_ = new AnimatedButton;
        secondaryApplyButton_->setObjectName(QStringLiteral("PrimaryButton"));
        secondaryApplyButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
        secondaryApplyButton_->setIconSize(QSize(18, 18));
        secondaryApplyButton_->setText(language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Run")
            : QStringLiteral("运行"));
        secondarySelectAllButton_->setObjectName(QStringLiteral("TaskDetailButton"));
        secondaryClearSelectionButton_->setObjectName(QStringLiteral("TaskDetailButton"));
        secondarySelectAllButton_->setProperty("toolbarButton", true);
        secondaryClearSelectionButton_->setProperty("toolbarButton", true);
        secondaryApplyButton_->setProperty("toolbarButton", true);
        applyButtonMetrics(secondarySelectAllButton_,
                           kToolbarSecondaryButtonWidth,
                           true,
                           kToolbarButtonHeight);
        applyButtonMetrics(secondaryClearSelectionButton_,
                           kToolbarSecondaryButtonWidth,
                           true,
                           kToolbarButtonHeight);
        applyButtonMetrics(secondaryApplyButton_,
                           kToolbarPrimaryButtonWidth,
                           true,
                           kToolbarButtonHeight);

        layout->addWidget(secondarySelectAllButton_, 0);
        layout->addWidget(secondaryClearSelectionButton_, 0);
        layout->addWidget(secondaryApplyButton_, 0);

        connect(secondarySelectAllButton_, &QPushButton::clicked, this, &CleanerWindow::selectAllOptimizations);
        connect(secondaryClearSelectionButton_, &QPushButton::clicked, this, &CleanerWindow::clearOptimizationSelection);
        connect(secondaryApplyButton_, &QPushButton::clicked, this, &CleanerWindow::applySelectedOptimizations);
        return row;
    }

    ClickableCardFrame *createAutostartStatusRow(const QJsonObject &item)
    {
        const Text text = t();
        const bool enabled = !item.value(QStringLiteral("disabled")).toBool();
        auto *row = new ClickableCardFrame;
        row->setObjectName(QStringLiteral("AutostartActionRow"));
        row->setInteractive(true);
        row->setContextMenuPolicy(Qt::CustomContextMenu);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 9, 14, 9);
        layout->setSpacing(10);

        auto *title = makeLabel(localName(item), QStringLiteral("RowTitle"));
        title->setToolTip(localDescription(item));
        layout->addWidget(title, 1);

        QFrame *statusPill = createValuePill(text.status, enabled ? text.active : text.disabled);
        statusPill->setFixedWidth(92);
        layout->addWidget(statusPill, 0, Qt::AlignVCenter);

        const QString detailText = language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Details")
            : QStringLiteral("详情");
        auto *detailButton = new QPushButton(detailText);
        detailButton->setObjectName(QStringLiteral("TaskDetailButton"));
        applyButtonMetrics(detailButton, kCompactButtonWidth);
        layout->addWidget(detailButton, 0, Qt::AlignVCenter);

        const QString detail = (language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Current: %1\n%2\n%3")
            : QStringLiteral("当前：%1\n%2\n%3"))
            .arg(enabled ? text.active : text.disabled,
                 localDescription(item),
                 item.value(QStringLiteral("target")).toString());
        auto showDetails = [this, item, detail]() {
            QMessageBox box(QMessageBox::Information, localName(item), detail, QMessageBox::NoButton, this);
            box.setStyleSheet(styleSheet());
            box.addButton(t().ok, QMessageBox::AcceptRole);
            box.exec();
        };
        connect(detailButton, &QPushButton::clicked, this, showDetails);
        connect(row, &ClickableCardFrame::clicked, this, showDetails);
        connect(row, &QWidget::customContextMenuRequested, this, [this, row, item, detail, detailText, showDetails](const QPoint &pos) {
            QMenu menu(row);
            menu.setStyleSheet(styleSheet());
            QAction *detailAction = menu.addAction(detailText);
            QAction *copyAction = menu.addAction(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Copy details")
                : QStringLiteral("复制详情"));
            QAction *selected = menu.exec(row->mapToGlobal(pos));
            if (selected == detailAction) {
                showDetails();
            } else if (selected == copyAction && QApplication::clipboard()) {
                QApplication::clipboard()->setText(localName(item) + QStringLiteral("\n") + detail);
            }
        });
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

    ClickableCardFrame *createOptimizationRow(QCheckBox *box, const QString &title, const QString &detail, const QString &size)
    {
        auto *row = new ClickableCardFrame;
        row->setObjectName(QStringLiteral("OptimizationTaskRow"));
        row->setInteractive(true);
        row->setContextMenuPolicy(Qt::CustomContextMenu);
        if (!box->isEnabled()) {
            row->setCursor(Qt::ArrowCursor);
        }
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 12, 14, 12);
        layout->setSpacing(12);
        box->setText(title);
        box->setToolTip(detail);
        box->setObjectName(QStringLiteral("TaskCheckBox"));

        auto *main = new QVBoxLayout;
        main->setSpacing(8);
        main->addWidget(box);
        auto *detailLabel = makeLabel(detail, QStringLiteral("PathValue"));
        main->addWidget(detailLabel);
        layout->addLayout(main, 4);

        auto *side = new QVBoxLayout;
        side->setSpacing(8);
        if (!size.isEmpty()) {
            side->addWidget(createValuePill(t().appSize, size));
        }
        const QString detailText = language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Details")
            : QStringLiteral("详情");
        auto *detailButton = new QPushButton(detailText);
        detailButton->setObjectName(QStringLiteral("TaskDetailButton"));
        applyButtonMetrics(detailButton, kCompactButtonWidth);
        side->addWidget(detailButton);
        side->addStretch(1);
        layout->addLayout(side, 1);

        auto showDetails = [this, title, detail, size]() {
            const QString body = size.isEmpty() ? detail : detail + QStringLiteral("\n\n") + t().appSize + QStringLiteral(" ") + size;
            QMessageBox box(QMessageBox::Information, title, body, QMessageBox::NoButton, this);
            box.setStyleSheet(styleSheet());
            box.addButton(t().ok, QMessageBox::AcceptRole);
            box.exec();
        };
        connect(detailButton, &QPushButton::clicked, this, showDetails);
        connect(row, &ClickableCardFrame::clicked, this, [box]() {
            if (box->isEnabled()) {
                box->setChecked(!box->isChecked());
            }
        });
        connect(row, &QWidget::customContextMenuRequested, this, [this, row, box, title, detail, detailText, showDetails](const QPoint &pos) {
            QMenu menu(row);
            menu.setStyleSheet(styleSheet());
            QAction *toggleAction = menu.addAction(box->isChecked()
                ? (language_->currentData().toString() == QStringLiteral("en") ? QStringLiteral("Deselect") : QStringLiteral("取消选择"))
                : (language_->currentData().toString() == QStringLiteral("en") ? QStringLiteral("Select") : QStringLiteral("选择")));
            toggleAction->setEnabled(box->isEnabled());
            QAction *detailAction = menu.addAction(detailText);
            QAction *copyAction = menu.addAction(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Copy details")
                : QStringLiteral("复制详情"));
            QAction *selected = menu.exec(row->mapToGlobal(pos));
            if (selected == toggleAction && box->isEnabled()) {
                box->setChecked(!box->isChecked());
            } else if (selected == detailAction) {
                showDetails();
            } else if (selected == copyAction && QApplication::clipboard()) {
                QApplication::clipboard()->setText(title + QStringLiteral("\n") + detail);
            }
        });
        return row;
    }

    ClickableCardFrame *createAutostartActionRow(QCheckBox *box,
                                                 const QString &title,
                                                 const QString &currentLabel,
                                                 const QString &actionLabel,
                                                 const QString &detail)
    {
        auto *row = new ClickableCardFrame;
        row->setObjectName(QStringLiteral("AutostartActionRow"));
        row->setInteractive(true);
        row->setContextMenuPolicy(Qt::CustomContextMenu);
        auto *layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 9, 14, 9);
        layout->setSpacing(10);

        box->setText(title);
        box->setToolTip(detail);
        box->setObjectName(QStringLiteral("TaskCheckBox"));
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        layout->addWidget(box, 1);

        QFrame *statusPill = createValuePill(t().status, currentLabel);
        QFrame *actionPill = createValuePill(language_->currentData().toString() == QStringLiteral("en")
                                                 ? QStringLiteral("Action")
                                                 : QStringLiteral("动作"),
                                             actionLabel);
        statusPill->setFixedWidth(88);
        actionPill->setFixedWidth(88);
        layout->addWidget(statusPill, 0, Qt::AlignVCenter);
        layout->addWidget(actionPill, 0, Qt::AlignVCenter);

        const QString detailText = language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Details")
            : QStringLiteral("详情");
        auto *detailButton = new QPushButton(detailText);
        detailButton->setObjectName(QStringLiteral("TaskDetailButton"));
        applyButtonMetrics(detailButton, kCompactButtonWidth);
        layout->addWidget(detailButton, 0, Qt::AlignVCenter);

        auto showDetails = [this, title, detail]() {
            QMessageBox box(QMessageBox::Information, title, detail, QMessageBox::NoButton, this);
            box.setStyleSheet(styleSheet());
            box.addButton(t().ok, QMessageBox::AcceptRole);
            box.exec();
        };
        connect(detailButton, &QPushButton::clicked, this, showDetails);
        connect(row, &ClickableCardFrame::clicked, this, [box]() {
            if (box->isEnabled()) {
                box->setChecked(!box->isChecked());
            }
        });
        connect(row, &QWidget::customContextMenuRequested, this, [this, row, box, title, detail, detailText, showDetails](const QPoint &pos) {
            QMenu menu(row);
            menu.setStyleSheet(styleSheet());
            QAction *toggleAction = menu.addAction(box->isChecked()
                ? (language_->currentData().toString() == QStringLiteral("en") ? QStringLiteral("Deselect") : QStringLiteral("取消选择"))
                : (language_->currentData().toString() == QStringLiteral("en") ? QStringLiteral("Select") : QStringLiteral("选择")));
            QAction *detailAction = menu.addAction(detailText);
            QAction *copyAction = menu.addAction(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Copy details")
                : QStringLiteral("复制详情"));
            QAction *selected = menu.exec(row->mapToGlobal(pos));
            if (selected == toggleAction && box->isEnabled()) {
                box->setChecked(!box->isChecked());
            } else if (selected == detailAction) {
                showDetails();
            } else if (selected == copyAction && QApplication::clipboard()) {
                QApplication::clipboard()->setText(title + QStringLiteral("\n") + detail);
            }
        });
        return row;
    }

    void resetMetricCards()
    {
        const Text text = t();
        clearRows(metricsRows_);
        clearMetricChart();
        const QStringList names{text.kaiming, text.ostree, text.kare, text.kareBase, text.appPayload, text.systemOther};
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
                                              {text.before, text.explainedUsage, text.systemOther},
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
            slice->setColor(QColor(79, 42, 136));
            slice->setBorderColor(QColor(143, 92, 198));
            slice->setLabelColor(QColor(246, 239, 255));
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
            QColor(216, 54, 232),
            QColor(69, 221, 196),
            QColor(121, 88, 231),
            QColor(254, 159, 73),
            QColor(255, 214, 92),
            QColor(139, 166, 255)
        };
        for (int i = 0; i < names.size() && i < values.size(); ++i) {
            const qint64 value = qMax<qint64>(0, values.at(i));
            if (value == 0) {
                continue;
            }
            QPieSlice *slice = metricsSeries_->append(names.at(i), static_cast<qreal>(value));
            const QColor color = colors.at(i % colors.size());
            slice->setColor(color);
            slice->setBorderColor(QColor(24, 7, 67));
            slice->setLabelColor(QColor(246, 239, 255));
            const QString percent = rootValue > 0
                ? QString::number(value * 100.0 / rootValue, 'f', 1) + QLatin1Char('%')
                : QStringLiteral("-");
            slice->setLabel(fmtBytes(value) + QStringLiteral(" · ") + percent);
            slice->setLabelVisible(true);
        }
        const QList<QLegendMarker *> markers = metricsChart_ ? metricsChart_->legend()->markers(metricsSeries_) : QList<QLegendMarker *>();
        int markerIndex = 0;
        for (int i = 0; i < names.size() && i < values.size() && markerIndex < markers.size(); ++i) {
            const qint64 value = qMax<qint64>(0, values.at(i));
            if (value == 0) {
                continue;
            }
            const QString percent = rootValue > 0
                ? QString::number(value * 100.0 / rootValue, 'f', 1) + QLatin1Char('%')
                : QStringLiteral("-");
            markers.at(markerIndex)->setLabel(names.at(i) + QStringLiteral("  ") + fmtBytes(value) + QStringLiteral(" · ") + percent);
            ++markerIndex;
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
        if (resultVisual_) {
            resultVisual_->setWorking(busy);
        }
        if (scanButton_) {
            scanButton_->setEnabled(!busy);
        }
        if (topScanButton_) {
            topScanButton_->setEnabled(!busy);
        }
        if (cleanOldButton_) {
            cleanOldButton_->setEnabled(!busy);
        }
        if (autostartButton_) {
            autostartButton_->setEnabled(!busy);
        }
        if (applyOptimizationsButton_) {
            if (busy) {
                applyOptimizationsButton_->setEnabled(false);
            } else {
                updateOptimizationSelectionState();
            }
        }
        if (rescanButton_) {
            rescanButton_->setEnabled(!busy);
        }
        if (progress_) {
            progress_->setProperty("busy", busy);
            progress_->style()->unpolish(progress_);
            progress_->style()->polish(progress_);
        }
        updateBusyOverlay();
        if (busy) {
            progress_->setRange(0, 100);
            progress_->setValue(scanProgressValue_ > 0 ? scanProgressValue_ : 6);
            lastUpdate_->setText(t().updating);
        } else {
            progress_->setRange(0, 100);
            progress_->setValue(100);
        }
    }

    void scanManual()
    {
        QObject *source = sender();
        if (source == topScanButton_ || source == scanButton_) {
            activeReviewFilter_ = 0;
        }
        scanInternal(true);
    }

    void beginScanProgress()
    {
        if (!scanStack_) {
            return;
        }
        scanProgressValue_ = 4;
        if (progress_) {
            progress_->setRange(0, 100);
            progress_->setValue(scanProgressValue_);
        }
        if (scanFlowProgress_) {
            scanFlowProgress_->setValue(scanProgressValue_);
        }
        if (scanProgressDetail_) {
            scanProgressDetail_->setText(t().scanProgressDetail);
        }
        if (tabs_) {
            tabs_->setCurrentIndex(1);
        }
        scanStack_->setCurrentIndex(1);
        updateMainNav(1);
        updateBusyOverlay();
        fadeIn(scanStack_->currentWidget());
        if (!scanProgressTimer_) {
            scanProgressTimer_ = new QTimer(this);
            connect(scanProgressTimer_, &QTimer::timeout, this, [this]() {
                scanProgressValue_ = qMin(92, scanProgressValue_ + (scanProgressValue_ < 55 ? 6 : 3));
                if (scanFlowProgress_) {
                    scanFlowProgress_->setValue(scanProgressValue_);
                }
                if (progress_) {
                    progress_->setValue(scanProgressValue_);
                }
            });
        }
        scanProgressTimer_->start(260);
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
        if (progress_) {
            progress_->setRange(0, 100);
            progress_->setValue(100);
        }
        updateBusyOverlay();
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
                showScanResults(activeReviewFilter_);
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

    void clearSecondaryToolbar()
    {
        if (!secondaryToolbarLayout_) {
            return;
        }
        while (QLayoutItem *item = secondaryToolbarLayout_->takeAt(0)) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
    }

    void showScanResults(int filter = -1)
    {
        if (filter >= 0) {
            activeReviewFilter_ = filter;
        }
        autostartSelectionPageOpen_ = false;
        const Text text = t();
        resultContainerBoxes_.clear();
        resultAutostartRows_.clear();
        clearRows(optimizationRows_);
        secondarySelectAllButton_ = nullptr;
        secondaryClearSelectionButton_ = nullptr;
        secondaryApplyButton_ = nullptr;
        clearSecondaryToolbar();
        clearRows(planRows_);

        const QJsonArray containers = state_.value(QStringLiteral("oldContainers")).toArray();
        const QJsonArray autostarts = state_.value(QStringLiteral("autostarts")).toArray();
        int selectableContainers = 0;
        int selectableAutostarts = 0;
        const bool showContainers = activeReviewFilter_ == 0 || activeReviewFilter_ == 1;
        const bool showAutostarts = activeReviewFilter_ == 0 || activeReviewFilter_ == 2;

        if (resultTitle_) {
            resultTitle_->setText(activeReviewFilter_ == 1 ? text.containerCleanup
                                  : activeReviewFilter_ == 2 ? text.autostartOptimization
                                                             : text.scanResultTitle);
        }
        setAutostartSecondaryLayout(false);

        if (showContainers) {
            addOptimizationCard(createInfoRow(text.containerCleanup, {text.status}, {text.selectContainers}));
            for (const QJsonValue &value : containers) {
                const QJsonObject item = value.toObject();
                auto *box = new QCheckBox;
                const bool selectable = !item.value(QStringLiteral("inUse")).toBool();
                box->setChecked(selectable);
                box->setEnabled(selectable);
                connect(box, &QCheckBox::toggled, this, &CleanerWindow::updateOptimizationSelectionState);
                if (selectable) {
                    ++selectableContainers;
                }
                const QString title = QStringLiteral("%1 / %2 / %3").arg(item.value(QStringLiteral("ref")).toString(),
                                                                         item.value(QStringLiteral("module")).toString(),
                                                                         item.value(QStringLiteral("version")).toString());
                const QString detail = selectable
                    ? QStringLiteral("%1\n%2").arg(item.value(QStringLiteral("kind")).toString(),
                                                   item.value(QStringLiteral("path")).toString())
                    : QStringLiteral("%1\n%2\n%3").arg(item.value(QStringLiteral("kind")).toString(),
                                                       item.value(QStringLiteral("path")).toString(),
                                                       language_->currentData().toString() == QStringLiteral("en")
                                                           ? QStringLiteral("Mounted or in use; this item cannot be selected safely.")
                                                           : QStringLiteral("当前已挂载或正在使用，不能安全选择。"));
                addOptimizationCard(createOptimizationRow(box,
                                                          title,
                                                          detail,
                                                          fmtBytes(jsonInt64(item, QStringLiteral("bytes")))));
                resultContainerBoxes_.append({item.value(QStringLiteral("path")).toString(), box});
            }
        }

        if (showAutostarts) {
            int activeEntries = 0;
            int disabledEntries = 0;
            for (const QJsonValue &value : autostarts) {
                if (value.toObject().value(QStringLiteral("disabled")).toBool()) {
                    ++disabledEntries;
                } else {
                    ++activeEntries;
                }
            }
            selectableAutostarts = autostarts.size();
            const QString detail = language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("%1 entries: %2 enabled, %3 disabled. Open the secondary list to disable enabled entries or restore disabled entries.")
                      .arg(autostarts.size())
                      .arg(activeEntries)
                      .arg(disabledEntries)
                : QStringLiteral("%1 个启动项：%2 个启用，%3 个禁用。进入二级列表后，可禁用已启用项，也可还原已禁用项。")
                      .arg(autostarts.size())
                      .arg(activeEntries)
                      .arg(disabledEntries);
            addOptimizationCard(createNavigationActionRow(text.autostartOptimization,
                                                          detail,
                                                          language_->currentData().toString() == QStringLiteral("en")
                                                              ? QStringLiteral("Open list")
                                                              : QStringLiteral("进入列表"),
                                                          [this]() { showAutostartSelectionPage(); },
                                                          kNavigationButtonWidth));
            if (autostarts.isEmpty()) {
                addOptimizationCard(createInfoRow(text.autostartOptimization, {text.status}, {text.noAutostarts}));
            } else {
                for (const QJsonValue &value : autostarts) {
                    addOptimizationCard(createAutostartStatusRow(value.toObject()));
                }
            }
        }

        if (selectableContainers == 0 && selectableAutostarts == 0) {
            addOptimizationCard(createInfoRow(text.optimizationItems, {text.status}, {text.noCleanable}));
        }

        if (resultSummary_) {
            resultSummary_->setText(scanSummary());
        }
        setOptimizationActionControlsVisible(activeReviewFilter_ != 2);
        updateOptimizationSelectionState();
        if (showAutostarts && resultAutostartRows_.isEmpty() && selectionSummary_) {
            selectionSummary_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Autostart entries are managed in a secondary list. Open the list to choose disable or restore actions.")
                : QStringLiteral("自启动项在二级列表中管理。进入列表后可选择禁用或还原。"));
        }
        if (showAutostarts && !showContainers && resultSummaryCard_) {
            resultSummaryCard_->hide();
        }
        if (scanStack_) {
            scanStack_->setCurrentIndex(2);
            fadeIn(scanStack_->currentWidget());
        }
    }

    void showAutostartSelectionPage()
    {
        autostartSelectionPageOpen_ = true;
        const Text text = t();
        resultContainerBoxes_.clear();
        resultAutostartRows_.clear();
        clearRows(optimizationRows_);
        secondarySelectAllButton_ = nullptr;
        secondaryClearSelectionButton_ = nullptr;
        secondaryApplyButton_ = nullptr;
        clearSecondaryToolbar();
        clearRows(planRows_);

        if (resultTitle_) {
            resultTitle_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Autostart Entries")
                : QStringLiteral("自启动项列表"));
        }
        setAutostartSecondaryLayout(true);

        addOptimizationCard(createNavigationActionRow(language_->currentData().toString() == QStringLiteral("en")
                                                          ? QStringLiteral("Back to optimization overview")
                                                          : QStringLiteral("返回优化概览"),
                                                      language_->currentData().toString() == QStringLiteral("en")
                                                          ? QStringLiteral("Return to the first-level optimization page.")
                                                          : QStringLiteral("返回上一级优化入口页面。"),
                                                      text.back,
                                                      [this]() { showScanResults(activeReviewFilter_); }));

        const QJsonArray autostarts = state_.value(QStringLiteral("autostarts")).toArray();
        if (autostarts.isEmpty()) {
            addOptimizationCard(createInfoRow(text.autostartOptimization, {text.status}, {text.noAutostarts}));
        }

        for (const QJsonValue &value : autostarts) {
            const QJsonObject item = value.toObject();
            const bool currentlyEnabled = !item.value(QStringLiteral("disabled")).toBool();
            auto *box = new QCheckBox;
            box->setChecked(false);
            connect(box, &QCheckBox::toggled, this, &CleanerWindow::updateOptimizationSelectionState);
            const QString actionLabel = language_->currentData().toString() == QStringLiteral("en")
                ? (currentlyEnabled ? QStringLiteral("Disable") : QStringLiteral("Restore"))
                : (currentlyEnabled ? QStringLiteral("禁用") : QStringLiteral("还原"));
            const QString currentLabel = currentlyEnabled ? text.active : text.disabled;
            const QString nextLabel = language_->currentData().toString() == QStringLiteral("en")
                ? (currentlyEnabled
                    ? QStringLiteral("Selected action: write a user Hidden=true override.")
                    : QStringLiteral("Selected action: remove the user override and inherit system autostart again."))
                : (currentlyEnabled
                    ? QStringLiteral("选中后：写入用户级 Hidden=true 覆盖，停止登录自启动。")
                    : QStringLiteral("选中后：移除用户级覆盖，恢复继承系统自启动项。"));
            const QString detail = (language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Current: %1\nAction: %2\n%3\n%4\n%5")
                : QStringLiteral("当前：%1\n动作：%2\n%3\n%4\n%5"))
                .arg(currentLabel,
                     actionLabel,
                     nextLabel,
                     localDescription(item),
                     item.value(QStringLiteral("target")).toString());
            addOptimizationCard(createAutostartActionRow(box,
                                                         localName(item),
                                                         currentLabel,
                                                         actionLabel,
                                                         detail));
            resultAutostartRows_.append({item.value(QStringLiteral("id")).toString(), currentlyEnabled, box});
        }

        if (secondaryToolbarLayout_) {
            secondaryToolbarLayout_->addWidget(createSecondaryAutostartToolbar());
        }

        if (resultSummary_) {
            resultSummary_->setText(scanSummary());
        }
        setOptimizationActionControlsVisible(true);
        updateOptimizationSelectionState();
        if (scanStack_) {
            scanStack_->setCurrentIndex(2);
            fadeIn(scanStack_->currentWidget());
        }
    }

    qint64 oldContainerBytesForPath(const QString &path) const
    {
        for (const QJsonValue &value : state_.value(QStringLiteral("oldContainers")).toArray()) {
            const QJsonObject item = value.toObject();
            if (item.value(QStringLiteral("path")).toString() == path) {
                return jsonInt64(item, QStringLiteral("bytes"));
            }
        }
        return 0;
    }

    void updateOptimizationSelectionState()
    {
        int availableContainers = 0;
        int selectedContainers = 0;
        int availableAutostarts = 0;
        int selectedAutostarts = 0;
        int selectedAutostartDisables = 0;
        int selectedAutostartRestores = 0;
        qint64 selectedBytes = 0;

        for (const auto &pair : resultContainerBoxes_) {
            if (!pair.second || !pair.second->isEnabled()) {
                continue;
            }
            ++availableContainers;
            if (pair.second->isChecked()) {
                ++selectedContainers;
                selectedBytes += oldContainerBytesForPath(pair.first);
            }
        }
        for (const AutostartSelectionRow &row : resultAutostartRows_) {
            if (!row.box || !row.box->isEnabled()) {
                continue;
            }
            ++availableAutostarts;
            if (row.box->isChecked()) {
                ++selectedAutostarts;
                if (row.currentlyEnabled) {
                    ++selectedAutostartDisables;
                } else {
                    ++selectedAutostartRestores;
                }
            }
        }

        if (selectionSummary_) {
            selectionSummary_->setText(language_->currentData().toString() == QStringLiteral("en")
                ? QStringLiteral("Selected %1 old containers (%2), %3 disable changes, and %4 restore changes. Available: %5 containers, %6 entries.")
                      .arg(selectedContainers)
                      .arg(fmtBytes(selectedBytes))
                      .arg(selectedAutostartDisables)
                      .arg(selectedAutostartRestores)
                      .arg(availableContainers)
                      .arg(availableAutostarts)
                : QStringLiteral("已选择 %1 个旧版本容器（%2）、%3 个禁用变更、%4 个还原变更。可选：%5 个容器、%6 个启动项。")
                      .arg(selectedContainers)
                      .arg(fmtBytes(selectedBytes))
                      .arg(selectedAutostartDisables)
                      .arg(selectedAutostartRestores)
                      .arg(availableContainers)
                      .arg(availableAutostarts));
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setEnabled(!busy_ && (selectedContainers > 0 || selectedAutostarts > 0));
        }
        if (secondaryApplyButton_) {
            secondaryApplyButton_->setEnabled(!busy_ && (selectedContainers > 0 || selectedAutostarts > 0));
        }
        const bool hasAvailable = availableContainers > 0 || availableAutostarts > 0;
        if (selectAllButton_) {
            selectAllButton_->setEnabled(!busy_ && hasAvailable);
        }
        if (secondarySelectAllButton_) {
            secondarySelectAllButton_->setEnabled(!busy_ && hasAvailable);
        }
        if (clearSelectionButton_) {
            clearSelectionButton_->setEnabled(!busy_ && hasAvailable && (selectedContainers > 0 || selectedAutostarts > 0));
        }
        if (secondaryClearSelectionButton_) {
            secondaryClearSelectionButton_->setEnabled(!busy_ && hasAvailable && (selectedContainers > 0 || selectedAutostarts > 0));
        }
    }

    void setOptimizationActionControlsVisible(bool visible)
    {
        if (selectAllButton_) {
            selectAllButton_->setVisible(visible);
        }
        if (clearSelectionButton_) {
            clearSelectionButton_->setVisible(visible);
        }
        if (applyOptimizationsButton_) {
            applyOptimizationsButton_->setVisible(visible);
        }
    }

    void setAutostartSecondaryLayout(bool secondary)
    {
        if (resultSummaryCard_) {
            resultSummaryCard_->setVisible(!secondary);
        }
        if (optimizationScrollArea_) {
            optimizationScrollArea_->setMinimumHeight(secondary ? 470 : 430);
        }
        if (secondaryToolbarHost_) {
            secondaryToolbarHost_->setVisible(secondary);
        }
    }

    void showActionResultPage()
    {
        if (!scanStack_) {
            return;
        }
        scanStack_->setCurrentIndex(3);
        updateBusyOverlay();
        fadeIn(scanStack_->currentWidget());
    }

    void setOptimizationSelection(bool checked)
    {
        for (const auto &pair : resultContainerBoxes_) {
            if (pair.second && pair.second->isEnabled()) {
                pair.second->setChecked(checked);
            }
        }
        for (const AutostartSelectionRow &row : resultAutostartRows_) {
            if (row.box && row.box->isEnabled()) {
                row.box->setChecked(checked);
            }
        }
        updateOptimizationSelectionState();
    }

    void selectAllOptimizations()
    {
        setOptimizationSelection(true);
    }

    void clearOptimizationSelection()
    {
        setOptimizationSelection(false);
    }

    void updateMetrics()
    {
        const Text text = t();
        const QJsonObject metrics = state_.value(QStringLiteral("metrics")).toObject();
        const qint64 rootValue = jsonInt64(metrics, QStringLiteral("root_used"));
        const QStringList keys{
            QStringLiteral("kaiming"),
            QStringLiteral("ostree_upper"),
            QStringLiteral("kare_upper"),
            QStringLiteral("kare_base"),
            QStringLiteral("app_payload"),
            QStringLiteral("system_other")
        };
        const QStringList names{text.kaiming, text.ostree, text.kare, text.kareBase, text.appPayload, text.systemOther};
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
                                                  {text.before, text.explainedUsage, text.systemOther},
                                                  {fmtBytes(rootValue), fmtBytes(rootValue - values.value(5)), fmtBytes(values.value(5))}));
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
        if (!statusNavButton_ || !appsNavButton_ || !containersNavButton_ || !autostartNavButton_) {
            return;
        }
        if (index == 0 && statusStack_) {
            activeNavIndex_ = statusStack_->currentIndex() == 1 ? 1 : 0;
        } else if (index == 1 && activeNavIndex_ < 2) {
            activeNavIndex_ = 2;
        }

        const QList<QPushButton *> buttons = {
            statusNavButton_,
            appsNavButton_,
            containersNavButton_,
            autostartNavButton_
        };
        for (int i = 0; i < buttons.size(); ++i) {
            QPushButton *button = buttons.at(i);
            button->setObjectName(i == activeNavIndex_ ? QStringLiteral("NavSelected") : QStringLiteral("NavButton"));
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
        }
        applyPageTheme();
    }

    void showStatusSubPage(int index)
    {
        if (!statusStack_) {
            return;
        }
        statusStack_->setCurrentIndex(index);
        if (metricsPageButton_ && appsPageButton_) {
            metricsPageButton_->setObjectName(index == 0 ? QStringLiteral("PrimaryButton") : QStringLiteral("ActionButton"));
            appsPageButton_->setObjectName(index == 1 ? QStringLiteral("PrimaryButton") : QStringLiteral("ActionButton"));
            metricsPageButton_->style()->unpolish(metricsPageButton_);
            metricsPageButton_->style()->polish(metricsPageButton_);
            appsPageButton_->style()->unpolish(appsPageButton_);
            appsPageButton_->style()->polish(appsPageButton_);
        }
        updateMainNav(tabs_ ? tabs_->currentIndex() : 0);
    }

    void primeTheme(int theme)
    {
        if (qApp) {
            qApp->setProperty("spaceGuardTheme", theme);
        }
        update();
        repaint();
        if (appShell_) {
            appShell_->setTargetTheme(theme);
        }
    }

    void applyPageTheme()
    {
        if (!appShell_ || !sideBar_ || !topBarFrame_) {
            return;
        }
        const int tabIndex = tabs_ ? tabs_->currentIndex() : 0;
        const int subIndex = statusStack_ ? statusStack_->currentIndex() : 0;
        const int theme = tabIndex == 1 ? 2 : (subIndex == 1 ? 1 : 0);
        QString shellGradient;
        QString sideGradient;
        QString topGlassGradient;

        if (tabIndex == 1) {
            shellGradient = QStringLiteral(
                "stop:0 #8f22b5, stop:0.30 #5a168f, stop:0.66 #25055f, stop:1 #0a023e");
            sideGradient = QStringLiteral(
                "stop:0 rgba(255, 118, 233, 0.28), stop:0.50 rgba(87, 21, 139, 0.28), stop:1 rgba(20, 4, 72, 0.28)");
            topGlassGradient = QStringLiteral(
                "stop:0 rgba(255, 118, 233, 0.16), stop:0.48 rgba(87, 21, 139, 0.12), stop:1 rgba(20, 4, 72, 0.10)");
        } else if (subIndex == 1) {
            shellGradient = QStringLiteral(
                "stop:0 #087580, stop:0.30 #0f5e92, stop:0.68 #143078, stop:1 #08033f");
            sideGradient = QStringLiteral(
                "stop:0 rgba(66, 235, 213, 0.24), stop:0.52 rgba(20, 72, 126, 0.30), stop:1 rgba(8, 3, 63, 0.30)");
            topGlassGradient = QStringLiteral(
                "stop:0 rgba(66, 235, 213, 0.15), stop:0.50 rgba(20, 72, 126, 0.12), stop:1 rgba(8, 3, 63, 0.10)");
        } else {
            shellGradient = QStringLiteral(
                "stop:0 #9d3210, stop:0.30 #883016, stop:0.66 #5b1256, stop:1 #17033d");
            sideGradient = QStringLiteral(
                "stop:0 rgba(255, 145, 56, 0.23), stop:0.52 rgba(116, 31, 101, 0.28), stop:1 rgba(23, 3, 61, 0.30)");
            topGlassGradient = QStringLiteral(
                "stop:0 rgba(255, 145, 56, 0.15), stop:0.50 rgba(116, 31, 101, 0.12), stop:1 rgba(23, 3, 61, 0.10)");
        }

        Q_UNUSED(shellGradient);
        if (qApp) {
            qApp->setProperty("spaceGuardTheme", theme);
        }
        appShell_->setTargetTheme(theme);
        sideBar_->setStyleSheet(QStringLiteral(
            "QFrame#SideBar {"
            "border-radius: 8px;"
            "border: 1px solid rgba(255, 255, 255, 0.10);"
            "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, %1);"
            "}").arg(sideGradient));
        topBarFrame_->setStyleSheet(QStringLiteral(
            "QFrame#TopBarFrame {"
            "border-radius: 8px;"
            "border: 1px solid rgba(255, 255, 255, 0.14);"
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1);"
            "}").arg(topGlassGradient));
        if (metricsChart_) {
            metricsChart_->setBackgroundBrush(QBrush(Qt::transparent));
            metricsChart_->setPlotAreaBackgroundBrush(QBrush(Qt::transparent));
            metricsChart_->legend()->setLabelColor(QColor(246, 239, 255));
        }
        for (CardFrame *card : findChildren<CardFrame *>()) {
            card->update();
        }
        update();
    }

    void fadeIn(QWidget *widget)
    {
        if (!widget) {
            return;
        }
        widget->setGraphicsEffect(nullptr);
        widget->update();
    }

    void pulseWidget(QWidget *widget)
    {
        if (!widget) {
            return;
        }
        auto *effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        auto *animation = new QPropertyAnimation(effect, "opacity", widget);
        animation->setDuration(220);
        animation->setStartValue(0.78);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation, &QPropertyAnimation::finished, widget, [widget]() {
            widget->setGraphicsEffect(nullptr);
        });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void updateBusyOverlay()
    {
        if (!busyOverlay_) {
            return;
        }
        const bool scanProgressVisible = scanStack_ && scanStack_->currentIndex() == 1;
        busyOverlay_->setVisible(busy_ && !scanProgressVisible);
        if (busyOverlay_->isVisible()) {
            busyOverlay_->raise();
        }
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
        auto *hint = new QLabel(language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Checked entries stay or become enabled. Clear a checked entry to disable it; check a disabled entry to restore it.")
            : QStringLiteral("勾选表示保持或恢复启用；取消勾选已启用项会禁用，勾选已禁用项会还原。"));
        hint->setObjectName(QStringLiteral("Intro"));
        hint->setWordWrap(true);
        layout->addWidget(hint);
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
                                      + (language_->currentData().toString() == QStringLiteral("en")
                                          ? (enabled ? QStringLiteral("Clear to disable this entry.") : QStringLiteral("Check to restore this entry."))
                                          : (enabled ? QStringLiteral("取消勾选可禁用此项。") : QStringLiteral("勾选可还原此项。")))
                                      + QStringLiteral("\n")
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
        QStringList enableIds;
        for (const auto &pair : resultContainerBoxes_) {
            if (pair.second && pair.second->isChecked() && pair.second->isEnabled()) {
                containerPaths << pair.first;
            }
        }
        for (const AutostartSelectionRow &row : resultAutostartRows_) {
            if (!row.box || !row.box->isChecked()) {
                continue;
            }
            if (row.currentlyEnabled) {
                disableIds << row.id;
            } else {
                enableIds << row.id;
            }
        }
        clearRows(planRows_);
        if (containerPaths.isEmpty() && disableIds.isEmpty() && enableIds.isEmpty()) {
            addPlanRow(t().planned, t().done, t().noCleanable);
            showActionResultPage();
            return;
        }
        setBusy(true);
        runSelectedAutostartOptimization(disableIds, enableIds, containerPaths);
    }

    void runSelectedAutostartOptimization(const QStringList &disableIds,
                                          const QStringList &enableIds,
                                          const QStringList &containerPaths)
    {
        if (disableIds.isEmpty() && enableIds.isEmpty()) {
            runSelectedContainerCleanup(containerPaths);
            return;
        }
        addPlanRow(t().autostartOptimization, t().running, language_->currentData().toString() == QStringLiteral("en")
            ? QStringLiteral("Apply %1 disable and %2 restore autostart changes.").arg(disableIds.size()).arg(enableIds.size())
            : QStringLiteral("应用 %1 个禁用、%2 个还原的自启动项变更。").arg(disableIds.size()).arg(enableIds.size()));
        runProcess({helper_, QStringLiteral("--manage-autostart"), QStringLiteral("--user"), user_,
                    QStringLiteral("--disable-entries"), disableIds.join(QLatin1Char(',')),
                    QStringLiteral("--enable-entries"), enableIds.join(QLatin1Char(','))},
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
            showActionResultPage();
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
        showActionResultPage();
        if (failCount > 0) {
            const QString path = writeErrorLog(QStringLiteral("partial selected optimization failure"), output);
            showErrorDialog(path);
        }
    }

    void finishSelectedOptimization()
    {
        setBusy(false);
        showActionResultPage();
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
            showActionResultPage();
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
        showActionResultPage();
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
    GlassShellFrame *appShell_ = nullptr;
    QFrame *sideBar_ = nullptr;
    QFrame *topBarFrame_ = nullptr;
    QFrame *busyOverlay_ = nullptr;
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
    SpaceVisual *resultVisual_ = nullptr;
    QTabWidget *tabs_ = nullptr;
    QStackedWidget *statusStack_ = nullptr;
    QStackedWidget *appsStack_ = nullptr;
    QStackedWidget *scanStack_ = nullptr;
    QLabel *overallTitle_ = nullptr;
    QLabel *metricsTitle_ = nullptr;
    QLabel *metricsRelation_ = nullptr;
    QLabel *rootDetailsTitle_ = nullptr;
    QLabel *appsTitle_ = nullptr;
    QLabel *appsRelation_ = nullptr;
    QLabel *detailTitle_ = nullptr;
    QLabel *actionsTitle_ = nullptr;
    QLabel *resultTitle_ = nullptr;
    QLabel *resultSummary_ = nullptr;
    QLabel *selectionSummary_ = nullptr;
    QWidget *resultSummaryCard_ = nullptr;
    QWidget *secondaryToolbarHost_ = nullptr;
    QWidget *planCard_ = nullptr;
    QLabel *planTitle_ = nullptr;
    QLabel *scanProgressTitle_ = nullptr;
    QLabel *scanProgressDetail_ = nullptr;
    QWidget *metricsList_ = nullptr;
    QWidget *appsList_ = nullptr;
    QWidget *detailList_ = nullptr;
    QWidget *planList_ = nullptr;
    QWidget *optimizationList_ = nullptr;
    QScrollArea *optimizationScrollArea_ = nullptr;
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
    QVBoxLayout *secondaryToolbarLayout_ = nullptr;
    QProgressBar *scanFlowProgress_ = nullptr;
    QTimer *scanProgressTimer_ = nullptr;
    int scanProgressValue_ = 0;
    QVector<QPair<QString, QCheckBox *>> resultContainerBoxes_;
    QVector<AutostartSelectionRow> resultAutostartRows_;
    QPushButton *heroScanButton_ = nullptr;
    QPushButton *topScanButton_ = nullptr;
    QPushButton *statusNavButton_ = nullptr;
    QPushButton *appsNavButton_ = nullptr;
    QPushButton *containersNavButton_ = nullptr;
    QPushButton *autostartNavButton_ = nullptr;
    QPushButton *metricsPageButton_ = nullptr;
    QPushButton *appsPageButton_ = nullptr;
    QPushButton *rootDetailsButton_ = nullptr;
    QPushButton *rootDetailsBackButton_ = nullptr;
    QPushButton *backToAppsButton_ = nullptr;
    QPushButton *applyOptimizationsButton_ = nullptr;
    QPushButton *rescanButton_ = nullptr;
    QPushButton *selectAllButton_ = nullptr;
    QPushButton *clearSelectionButton_ = nullptr;
    QPushButton *secondarySelectAllButton_ = nullptr;
    QPushButton *secondaryClearSelectionButton_ = nullptr;
    QPushButton *secondaryApplyButton_ = nullptr;
    QPushButton *scanButton_ = nullptr;
    QPushButton *cleanOldButton_ = nullptr;
    QPushButton *autostartButton_ = nullptr;
    QPushButton *minimizeButton_ = nullptr;
    QPushButton *maximizeButton_ = nullptr;
    QPushButton *closeButton_ = nullptr;
    int activeNavIndex_ = 0;
    int activeReviewFilter_ = 0;
    bool autostartSelectionPageOpen_ = false;
};

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kylin-space-cleaner"));
    QApplication app(argc, argv);
    QApplication::setDesktopFileName(QStringLiteral("kylin-space-cleaner"));
    QIcon appIcon;
    appIcon.addFile(QStringLiteral(":/icons/kylin-space-guard-256.png"), QSize(256, 256));
    appIcon.addFile(QStringLiteral(":/icons/kylin-space-guard.svg"));
    app.setWindowIcon(appIcon);
    CleanerWindow window;
    window.setWindowIcon(appIcon);
    window.show();
    return app.exec();
}

#include "space_cleaner.moc"
