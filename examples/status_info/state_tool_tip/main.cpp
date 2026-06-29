#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QPointer>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *button = new PushButton(QStringLiteral("Click Me"), this);
        button->move(360, 225);
        connect(button, &QPushButton::clicked, this, &Demo::onButtonClicked);
        resize(800, 300);
    }

  private:
    void onButtonClicked()
    {
        if (m_stateToolTip) {
            m_stateToolTip->setContent(QStringLiteral("模型训练完成啦 😆"));
            m_stateToolTip->setState(true);
            m_stateToolTip = nullptr;
        } else {
            m_stateToolTip = new StateToolTip(QStringLiteral("正在训练模型"),
                                              QStringLiteral("客官请耐心等待哦~~"), this);
            m_stateToolTip->move(510, 30);
            m_stateToolTip->show();
        }
    }

    QPointer<StateToolTip> m_stateToolTip;
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
