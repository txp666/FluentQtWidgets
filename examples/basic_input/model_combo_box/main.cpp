#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
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

        auto *comboBox = new ModelComboBox(this);
        comboBox->setPlaceholderText(QStringLiteral("选择一个脑婆"));
        comboBox->addItems({QStringLiteral("shoko 🥰"), QStringLiteral("西宫硝子"), QStringLiteral("宝多六花"),
                            QStringLiteral("小鸟游六花")});
        comboBox->setCurrentIndex(-1);
        connect(comboBox, &ModelComboBox::currentTextChanged, this,
                [](const QString &text) { qDebug() << text; });

        auto *layout = new QHBoxLayout(this);
        layout->addWidget(comboBox, 0, Qt::AlignCenter);

        resize(500, 500);
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
