#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/Qt>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(300, 300);

    auto *layout = new QVBoxLayout(&window);
    layout->setAlignment(Qt::AlignCenter);

    const QStringList items = {QStringLiteral("Shoko"), QStringLiteral("Nishimiya"),
                               QStringLiteral("Aiko"), QStringLiteral("Yanai")};

    auto *comboBox = new AcrylicComboBox(&window);
    comboBox->addItems(items);
    comboBox->setCurrentIndex(0);
    layout->addWidget(comboBox, 0, Qt::AlignCenter);

    auto *editableComboBox = new AcrylicEditableComboBox(&window);
    editableComboBox->addItems(items);
    editableComboBox->setCurrentIndex(0);
    editableComboBox->setCompleter(new QCompleter(items, editableComboBox));
    layout->addWidget(editableComboBox, 0, Qt::AlignCenter);

    window.show();
    return app.exec();
}
