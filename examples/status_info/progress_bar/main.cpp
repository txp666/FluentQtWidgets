#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *progressBar = new ProgressBar(this);
        m_indeterminate = new IndeterminateProgressBar(this);
        m_progressBar = progressBar;
        m_button = new ToggleToolButton(icon(FluentIcon::Pause), this);

        progressBar->setValue(50);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->addWidget(progressBar);
        layout->addWidget(m_indeterminate);
        layout->addWidget(m_button, 0, Qt::AlignHCenter);
        resize(400, 400);

        connect(m_button, &QAbstractButton::clicked, this, &Demo::onButtonClicked);
    }

  private:
    void onButtonClicked()
    {
        if (m_indeterminate->isStarted()) {
            m_indeterminate->pause();
            m_progressBar->pause();
            m_button->setIcon(icon(FluentIcon::Play));
        } else {
            m_indeterminate->resume();
            m_progressBar->resume();
            m_button->setIcon(icon(FluentIcon::Pause));
        }
    }

    ProgressBar *m_progressBar = nullptr;
    IndeterminateProgressBar *m_indeterminate = nullptr;
    ToggleToolButton *m_button = nullptr;
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
