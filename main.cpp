#include <QCoreApplication>
#include <QStringList>
#include <QTimer>
#include <QTextStream>

#include "converttask.h"
#include "parameters.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Convertor *convertor = new Convertor(&a, argc, argv);

    QObject::connect(convertor, SIGNAL(finished()), &a, SLOT(quit()));
    QObject::connect(convertor, SIGNAL(finished()), &a, SLOT(quit()));

    QTimer::singleShot(0, convertor, SLOT(run()));
    
    return a.exec();
}
