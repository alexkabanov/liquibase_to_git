#ifndef DATABASEREPO_H
#define DATABASEREPO_H

#include <QString>
#include <QStringList>

class DatabaseRepo
{
public:

    enum ObjectType {
        Unknown = 0,
        Table = 1,
        View = 2,
        Procedure = 3,
        Function = 4,
        Package = 5,
        MView = 6,
        MViewLog = 7,
        Sequence = 8,
        Java = 9,
        DBLink = 10,
        Type = 11,
        Job = 12
    };

    DatabaseRepo(const QString &path);

    bool saveObject(ObjectType type, const QString &name, const QString &ddl);
    bool saveData(const QString &name, const QString &ddl);

    QStringList allObjects();
    bool clear();

    bool initChangelog();

    static QString typeDirrectoryById(ObjectType objType);
    static ObjectType typeIdByName(const QString &type);

    inline QString path() {return _path;}
    inline QString lastError() {return _errorString;}

private:
    QString _path;
    QString _errorString;
};

#endif // DATABASEREPO_H
