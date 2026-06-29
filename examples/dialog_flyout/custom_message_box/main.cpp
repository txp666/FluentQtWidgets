#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class CustomMessageBox : public MessageBoxBase
{
  public:
    explicit CustomMessageBox(QWidget *parent = nullptr) : MessageBoxBase(parent)
    {
        m_titleLabel = new SubtitleLabel(QStringLiteral("Open URL"), this);
        m_urlLineEdit = new LineEdit(this);
        m_warningLabel = new CaptionLabel(QStringLiteral("The url is invalid"), this);

        m_urlLineEdit->setPlaceholderText(QStringLiteral("Enter the URL of a file, stream, or playlist"));
        m_urlLineEdit->setClearButtonEnabled(true);
        m_warningLabel->setTextColor(QColor(QStringLiteral("#cf1010")), QColor(255, 28, 32));
        m_warningLabel->hide();

        viewLayout()->addWidget(m_titleLabel);
        viewLayout()->addWidget(m_urlLineEdit);
        viewLayout()->addWidget(m_warningLabel);

        yesButton()->setText(QStringLiteral("Open"));
        cancelButton()->setText(QStringLiteral("Cancel"));
        widget()->setMinimumWidth(350);
    }

    QString url() const { return m_urlLineEdit ? m_urlLineEdit->text() : QString(); }

  protected:
    bool validate() const override
    {
        const bool valid = m_urlLineEdit && m_urlLineEdit->text().toLower().startsWith(QStringLiteral("http://"));
        if (m_warningLabel) {
            m_warningLabel->setHidden(valid);
        }
        if (m_urlLineEdit) {
            m_urlLineEdit->setError(!valid);
        }
        return valid;
    }

  private:
    SubtitleLabel *m_titleLabel = nullptr;
    LineEdit *m_urlLineEdit = nullptr;
    CaptionLabel *m_warningLabel = nullptr;
};

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *button = new PushButton(QStringLiteral("Open URL"), this);
        auto *layout = new QHBoxLayout(this);
        layout->addWidget(button, 0, Qt::AlignCenter);
        connect(button, &QPushButton::clicked, this, [this]() {
            CustomMessageBox dialog(this);
            if (dialog.exec()) {
                qInfo("%s", qPrintable(dialog.url()));
            }
        });
        resize(600, 600);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    Demo demo;
    demo.show();
    return app.exec();
}
