#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_button = new ToggleToolButton(icon(FluentIcon::Pause), this);
        auto *spinner = new IndeterminateProgressRing(this);
        m_progressRing = new ProgressRing(this);
        auto *spinBox = new SpinBox(this);

        m_progressRing->setValue(50);
        m_progressRing->setTextVisible(true);
        m_progressRing->setFixedSize(80, 80);

        spinBox->setRange(0, 100);
        spinBox->setValue(50);
        connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), m_progressRing, &QProgressBar::setValue);

        auto *row = new QHBoxLayout;
        row->addWidget(m_progressRing, 0, Qt::AlignHCenter);
        row->addWidget(spinBox, 0, Qt::AlignHCenter);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(30, 30, 30, 30);
        layout->addLayout(row);
        layout->addWidget(spinner, 0, Qt::AlignHCenter);
        layout->addWidget(m_button, 0, Qt::AlignHCenter);
        resize(400, 400);

        connect(m_button, &QAbstractButton::clicked, this, &Demo::onButtonClicked);
    }

  private:
    void onButtonClicked()
    {
        if (!m_progressRing->isPaused()) {
            m_progressRing->pause();
            m_button->setIcon(icon(FluentIcon::Play));
        } else {
            m_progressRing->resume();
            m_button->setIcon(icon(FluentIcon::Pause));
        }
    }

    ProgressRing *m_progressRing = nullptr;
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
