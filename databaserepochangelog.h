#ifndef DATABASEREPOCHANGELOG_H
#define DATABASEREPOCHANGELOG_H

#include <QDomDocument>
#include "databasechangelog.h"
#include "lqfile.h"

class DatabaseRepoChangeLog
{
public:
    DatabaseRepoChangeLog(const QString &file);

    bool open();
    void init(const DatabaseChangeLog &dbChLog, const LqFile &lqFile);
    bool save();
    bool checkFile(const QString& file);
    bool checkFile(const QString& file, const QString& md5);
    void appendFile(const QString &file, const QString &md5);

    inline QString lastError() {return _errorString;}

private:
    QString _file;
    QDomDocument doc;
    QString _errorString;
};

#endif // DATABASEREPOCHANGELOG_H

