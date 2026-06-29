#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QCoreApplication>
#include <QtCore/Qt>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

using namespace FluentQt;

namespace {

class BackgroundLabel : public QLabel
{
  public:
    explicit BackgroundLabel(QWidget *parent = nullptr) : QLabel(parent)
    {
        m_pixmap.load(QStringLiteral(":/window/login/resource/images/background.jpg"));
        setMinimumWidth(340);
    }

  protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        if (m_pixmap.isNull()) {
            painter.fillRect(rect(), QColor(32, 32, 32));
            return;
        }

        const QSize scaledSize = m_pixmap.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
        const QPixmap scaled = m_pixmap.scaled(scaledSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const QPoint topLeft((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
        painter.drawPixmap(topLeft, scaled);
    }

  private:
    QPixmap m_pixmap;
};

class LoginWindow : public FluentWidget
{
  public:
    explicit LoginWindow(QWidget *parent = nullptr) : FluentWidget(parent)
    {
        setWindowTitle(QStringLiteral("FluentQtWidgets"));
        setWindowIcon(QIcon(QStringLiteral(":/window/login/resource/images/logo.png")));
        titleBar()->raise();

        auto *rootLayout = new QHBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        rootLayout->addWidget(new BackgroundLabel(this), 1);

        auto *panel = new QWidget(this);
        panel->setObjectName(QStringLiteral("LoginPanel"));
        panel->setMinimumWidth(360);
        panel->setMaximumWidth(360);
        panel->setStyleSheet(QStringLiteral(
            "QWidget#LoginPanel{background: palette(window);}"
            "QWidget#LoginPanel BodyLabel{font-size:13px;}"));
        rootLayout->addWidget(panel);

        auto *layout = new QVBoxLayout(panel);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(9);
        layout->addStretch(1);

        auto *logo = new QLabel(panel);
        logo->setFixedSize(100, 100);
        logo->setPixmap(QPixmap(QStringLiteral(":/window/login/resource/images/logo.png"))
                            .scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logo->setAlignment(Qt::AlignCenter);
        layout->addWidget(logo, 0, Qt::AlignHCenter);
        layout->addSpacing(15);

        auto *grid = new QGridLayout;
        grid->setHorizontalSpacing(4);
        grid->setVerticalSpacing(9);
        auto *hostLabel = new BodyLabel(tr("Host"), panel);
        auto *portLabel = new BodyLabel(tr("Port"), panel);
        auto *hostEdit = new LineEdit(panel);
        auto *portEdit = new LineEdit(panel);
        hostEdit->setPlaceholderText(QStringLiteral("ftp.example.com"));
        portEdit->setText(QStringLiteral("21"));
        hostEdit->setClearButtonEnabled(true);
        portEdit->setClearButtonEnabled(true);
        grid->addWidget(hostLabel, 0, 0);
        grid->addWidget(portLabel, 0, 1);
        grid->addWidget(hostEdit, 1, 0);
        grid->addWidget(portEdit, 1, 1);
        grid->setColumnStretch(0, 2);
        grid->setColumnStretch(1, 1);
        layout->addLayout(grid);

        auto *userLabel = new BodyLabel(tr("Username"), panel);
        auto *userEdit = new LineEdit(panel);
        userEdit->setPlaceholderText(QStringLiteral("example@example.com"));
        userEdit->setClearButtonEnabled(true);
        layout->addWidget(userLabel);
        layout->addWidget(userEdit);

        auto *passwordLabel = new BodyLabel(tr("Password"), panel);
        auto *passwordEdit = new LineEdit(panel);
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText(QStringLiteral("............"));
        passwordEdit->setClearButtonEnabled(true);
        layout->addWidget(passwordLabel);
        layout->addWidget(passwordEdit);

        layout->addSpacing(5);
        auto *remember = new CheckBox(tr("Remember password"), panel);
        remember->setChecked(true);
        layout->addWidget(remember);

        layout->addSpacing(5);
        auto *loginButton = new PrimaryPushButton(tr("Login"), panel);
        layout->addWidget(loginButton);

        layout->addSpacing(6);
        auto *forgotButton = new HyperlinkButton(QStringLiteral(FQW_REPOSITORY_URL),
                                                 tr("Forgot password"), panel);
        layout->addWidget(forgotButton);
        layout->addStretch(1);

        resize(1000, 650);
    }
};

} // namespace

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    LoginWindow window;
    window.show();
    return app.exec();
}
