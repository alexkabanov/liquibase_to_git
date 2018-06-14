#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>

#include "databaserepo.h"

class Database
{
public:
    Database();

    void setUser(const QString & user);
    void setPass(const QString & pass);
    void setDBName(const QString & dbName);

    bool connectDB();
    void disconnectDB();
    void reconnectDB();

    QString lastError();

    bool execStmt(const QString &stmt);
    bool execSQLFile(const QString &fileName);

    QString getDDL(const QString &type, const QString &name);
    QString getTableDDL(const QString &name);

    bool DDLToRepo(DatabaseRepo &repo);
    bool dataToRepo(DatabaseRepo &repo, const QStringList &tables);
    bool tablesDDLToRepo(DatabaseRepo &repo);
    bool procsDDLToRepo(DatabaseRepo &repo);
    bool funcsDDLToRepo(DatabaseRepo &repo);
    bool pckgsDDLToRepo(DatabaseRepo &repo);
    bool viewsDDLToRepo(DatabaseRepo &repo);
    bool mviewsDDLToRepo(DatabaseRepo &repo);
    bool sequenceDDLToRepo(DatabaseRepo &repo);
    bool javaDDLToRepo(DatabaseRepo &repo);
    bool dblinkDDLToRepo(DatabaseRepo &repo);
    bool typeDDLToRepo(DatabaseRepo &repo);
    bool jobDDLToRepo(DatabaseRepo &repo);
    bool mviewlogDDLToRepo(DatabaseRepo &repo);

    inline QSqlDatabase QDB() {return _db;}

private:
    QSqlDatabase _db;
    QString _errorString;

};

#endif // DATABASE_H
