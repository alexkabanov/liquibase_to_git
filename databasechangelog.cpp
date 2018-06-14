#include "databasechangelog.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDomDocument>
#include <QDomAttr>
#include <QDomElement>

#include "sqlfile.h"

DatabaseChangeLog::DatabaseChangeLog() :_db(0)
{
}

void DatabaseChangeLog::setDatabase(Database * db)
{
    _db = db;
}

bool DatabaseChangeLog::open()
{
    if (!_db)
    {
        _errorString = "No database specified";
        return false;
    }

    QSqlQuery qr(_db->QDB());

    qr.prepare("select filename from databasechangelog");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    _files.clear();

    while (qr.next())
    {
        _files << qr.value(0).toString();
    }

    return true;
}

bool DatabaseChangeLog::checkFile(const QString &file) const
{
    return (_files.indexOf(file) != -1);
}
