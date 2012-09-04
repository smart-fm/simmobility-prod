#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QPushButton *button = new QPushButton("Quit");
    QObject::connect(button, SIGNAL(clicked()), &application, SLOT(quit()));
    button->show();
    return application.exec();
}
