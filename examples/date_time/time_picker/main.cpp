#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class SecondsFormatter : public PickerColumnFormatter
{
  public:
    using PickerColumnFormatter::PickerColumnFormatter;

    QString encode(const QVariant &value) const override { return value.toString() + QStringLiteral("秒"); }
    QVariant decode(const QString &value) const override { return value.left(value.size() - 1).toInt(); }
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

        auto *datePicker1 = new DatePicker(this);
        auto *datePicker2 = new ZhDatePicker(this);
        auto *timePicker1 = new AMTimePicker(this);
        auto *timePicker2 = new TimePicker(this);
        auto *timePicker3 = new TimePicker(this, true);

        m_secondsFormatter = new SecondsFormatter(timePicker3);
        timePicker3->setColumnFormatter(2, m_secondsFormatter);

        connect(datePicker1, &DatePicker::dateChanged, this, [](const QDate &date) { qDebug() << date.toString(); });
        connect(datePicker2, &ZhDatePicker::dateChanged, this, [](const QDate &date) { qDebug() << date.toString(); });
        connect(timePicker1, &AMTimePicker::timeChanged, this, [](const QTime &time) { qDebug() << time.toString(); });
        connect(timePicker2, &TimePicker::timeChanged, this, [](const QTime &time) { qDebug() << time.toString(); });
        connect(timePicker3, &TimePicker::timeChanged, this, [](const QTime &time) { qDebug() << time.toString(); });

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(datePicker1, 0, Qt::AlignHCenter);
        layout->addWidget(datePicker2, 0, Qt::AlignHCenter);
        layout->addWidget(timePicker1, 0, Qt::AlignHCenter);
        layout->addWidget(timePicker2, 0, Qt::AlignHCenter);
        layout->addWidget(timePicker3, 0, Qt::AlignHCenter);
        resize(500, 500);
    }

  private:
    SecondsFormatter *m_secondsFormatter = nullptr;
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
