#ifndef DATABASECHANGELOG_H
#define DATABASECHANGELOG_H

#include "database.h"
#include <QStringList>

class DatabaseChangeLog
{
public:
    DatabaseChangeLog();

    void setDatabase(Database * db);

    bool open();

    bool saveToFile(const QString& file);

    bool checkFile(const QString &file) const;

    inline QString lastError() {return _errorString;}

private:
    Database * _db;
    QStringList _files;
    QString _errorString;
};

#endif // DATABASECHANGELOG_H
