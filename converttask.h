#ifndef CONVERTTASK_H
#define CONVERTTASK_H

#include <QObject>
#include <QStringList>
#include "parameters.h"

class Convertor : public QObject
{
    Q_OBJECT
public:
    explicit Convertor(QObject *parent, int argc, char *argv[]);

    void setParameters(const Parameters &parameters);
    
signals:
    
public slots:
    void run();

signals:
    void finished();

private:

    QStringList _dataTables;

    Parameters _params;
};

#endif // CONVERTTASK_H

