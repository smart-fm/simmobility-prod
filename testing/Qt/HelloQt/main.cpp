#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QLabel *label = new QLabel("Hello Qt!");
    label->show();
    return application.exec();
}
