#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class CustomMessageBox : public MessageBoxBase
{
  public:
    explicit CustomMessageBox(QWidget *parent = nullptr) : MessageBoxBase(parent)
    {
        auto *title = new SubtitleLabel(QStringLiteral("打开 URL"), widget());
        m_urlLineEdit = new LineEdit(widget());
        m_warningLabel = new CaptionLabel(QStringLiteral("The url is invalid"), widget());

        m_urlLineEdit->setPlaceholderText(QStringLiteral("输入文件、流或者播放列表的 URL"));
        m_urlLineEdit->setClearButtonEnabled(true);
        m_warningLabel->setTextColor(QColor(QStringLiteral("#cf1010")), QColor(255, 28, 32));
        m_warningLabel->hide();

        viewLayout()->addWidget(title);
        viewLayout()->addWidget(m_urlLineEdit);
        viewLayout()->addWidget(m_warningLabel);

        yesButton()->setText(QStringLiteral("打开"));
        cancelButton()->setText(QStringLiteral("取消"));
        widget()->setMinimumWidth(350);
    }

    QString url() const { return m_urlLineEdit->text(); }

  protected:
    bool validate() const override
    {
        const bool valid = m_urlLineEdit->text().toLower().startsWith(QStringLiteral("http://"));
        m_warningLabel->setHidden(valid);
        m_urlLineEdit->setError(!valid);
        return valid;
    }

  private:
    LineEdit *m_urlLineEdit = nullptr;
    CaptionLabel *m_warningLabel = nullptr;
};

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *dialogButton = new PrimaryPushButton(QStringLiteral("Click Me"), this);
        auto *customButton = new PushButton(QStringLiteral("打开 URL"), this);

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(dialogButton, 0, Qt::AlignCenter);
        layout->addWidget(customButton, 0, Qt::AlignCenter);
        resize(950, 500);

        connect(dialogButton, &QPushButton::clicked, this, [this]() {
            Dialog dialog(QStringLiteral("Are you sure you want to delete the folder?"),
                          QStringLiteral("If you delete the \"Music\" folder from the list, the folder will no "
                                         "longer appear in the list, but will not be deleted."),
                          this);
            if (dialog.exec() == QDialog::Accepted) {
                qDebug() << "Yes button is pressed";
            } else {
                qDebug() << "Cancel button is pressed";
            }
        });

        connect(customButton, &QPushButton::clicked, this, [this]() {
            CustomMessageBox dialog(this);
            if (dialog.exec() == QDialog::Accepted) {
                qDebug() << dialog.url();
            }
        });
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo w;
    w.show();
    return QApplication::exec();
}
