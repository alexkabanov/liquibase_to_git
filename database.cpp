#include "database.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDir>
#include <QTextStream>
#include "sqlfile.h"


Database::Database()
{
    _db = QSqlDatabase::addDatabase("QOCI");
}

void Database::setUser(const QString & user)
{
    _db.setUserName(user);
}

void Database::setPass(const QString & pass)
{
    _db.setPassword(pass);
}

void Database::setDBName(const QString & dbName)
{
    _db.setDatabaseName(dbName);
}


bool Database::connectDB()
{
    if (!Database::_db.open()) {
        if (Database::_db.lastError().isValid()) {
            _errorString = Database::_db.lastError().text();
            return false;
        }
    }
    return true;
}

void Database::disconnectDB()
{
   Database::_db.close();
}

void Database::reconnectDB()
{
    disconnectDB();
    connectDB();
}

QString Database::lastError()
{
    return Database::_errorString;
}



bool Database::execStmt(const QString &stmt)
{
    QSqlQuery qr;
    return qr.exec(stmt);
}

bool Database::execSQLFile(const QString &fileName)
{

    SqlFile reader(fileName);

    if (!reader.open())
    {
        _errorString = reader.lastError();
        return false;
    }


    SqlFileStatment stmt;
    do
    {
        stmt = reader.nextStatment();

        if (reader.error() != SqlFile::NoError)
        {
            _errorString = reader.lastError();
            return false;
        }
        else
        {
            if (stmt.type() == SqlFileStatment::PLSQL || stmt.type() == SqlFileStatment::SQL)
            {
                QSqlQuery qr;
                if (!qr.exec(stmt.text()))
                {
                    _errorString = qr.lastError().text() + "; SQL: " + stmt.text();
                    return false;
                }
            }
        }

    } while (stmt.type() != SqlFileStatment::Unknown);


    if (!_db.commit())
    {
        _errorString = _db.lastError().text();
        return false;
    }

    return true;
}

QString Database::getDDL(const QString &type, const QString &name)
{
    if (type == "TABLE")
        return getTableDDL(name);



    return QString();

}

QString Database::getTableDDL(const QString &name)
{
    QSqlQuery qr;

    qr.prepare("select dbms_metadata.get_ddl('TABLE', :TABLE_NAME) "
               "|| dbms_metadata.get_dependent_ddl ('INDEX',:TABLE_NAME) "
               "|| dbms_metadata.get_dependent_ddl ('INDEX',:TABLE_NAME) "
               "|| dbms_metadata.get_dependent_ddl('REF_CONSTRAINT',:TABLE_NAME) "
               "|| dbms_metadata.get_dependent_ddl('TRIGGER',:TABLE_NAME) from dual");

    qr.bindValue(QString(":TABLE_NAME"), name);

    if (!qr.exec())
    {

    }

    return "";

}



bool Database::DDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    QString stmtDDLParam = "BEGIN \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'PRETTY',true); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'STORAGE',false); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SEGMENT_ATTRIBUTES',false); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'REF_CONSTRAINTS',true); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'CONSTRAINTS_AS_ALTER',true); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'CONSTRAINTS',true); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SIZE_BYTE_KEYWORD',false); \
            DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SQLTERMINATOR',TRUE); \
            END;";

    if (!qr.exec(stmtDDLParam))
    {
        _errorString = qr.lastError().text();
        return false;
    }


    if (!tablesDDLToRepo(repo))
        return false;
    if (!procsDDLToRepo(repo))
        return false;
    if (!funcsDDLToRepo(repo))
        return false;
    if (!pckgsDDLToRepo(repo))
        return false;
    if (!viewsDDLToRepo(repo))
        return false;
    if (!mviewsDDLToRepo(repo))
        return false;
    if (!sequenceDDLToRepo(repo))
        return false;
    if (!javaDDLToRepo(repo))
        return false;
    if (!dblinkDDLToRepo(repo))
        return false;
    if (!typeDDLToRepo(repo))
        return false;
    if (!jobDDLToRepo(repo))
        return false;
    if (!mviewlogDDLToRepo(repo))
        return false;
    return true;
}

bool Database::tablesDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select table_name, dbms_metadata.get_ddl('TABLE', table_name) "
               "|| case when exists(select * from USER_TRIGGERS ui where ui.table_name = ut.table_name and ui.TABLE_OWNER = user) then dbms_metadata.get_dependent_ddl ('INDEX',table_name) end "
               "|| case when exists(select * from USER_TRIGGERS utg where utg.table_name = ut.table_name and utg.TABLE_OWNER = user) then dbms_metadata.get_dependent_ddl('TRIGGER',ut.table_name) end "
               "ddl "
               "from user_tables ut");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Table, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::procsDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select object_name, dbms_metadata.get_ddl(object_type, object_name) ddl \
               from user_procedures \
              where object_type in ('PROCEDURE')");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Procedure, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}


bool Database::funcsDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select object_name, dbms_metadata.get_ddl(object_type, object_name) ddl \
               from user_procedures \
              where object_type in ('FUNCTION')");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Function, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::pckgsDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('PACKAGE', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'PACKAGE'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Package, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::viewsDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('VIEW', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'VIEW'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::View, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::mviewsDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('MATERIALIZED_VIEW', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'MATERIALIZED VIEW'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::MView, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::sequenceDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('SEQUENCE', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'SEQUENCE'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Sequence, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::javaDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('JAVA_SOURCE', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'JAVA SOURCE'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Java, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::dblinkDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('DB_LINK', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'DATABASE LINK'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::DBLink, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::typeDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('TYPE_SPEC', s.OBJECT_NAME) || \
               case when sb.OBJECT_NAME is not null then dbms_metadata.get_ddl('TYPE_BODY', sb.OBJECT_NAME) end ddl \
            from USER_OBJECTS s \
                left join USER_OBJECTS sb on sb.OBJECT_NAME = s.OBJECT_NAME and sb.OBJECT_TYPE = 'TYPE BODY' \
            where s.OBJECT_TYPE = 'TYPE' \
                and s.OBJECT_NAME not like 'SYS_%'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Type, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::jobDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.OBJECT_NAME, \
               dbms_metadata.get_ddl('PROCJOB', s.OBJECT_NAME) ddl \
           from USER_OBJECTS s \
           where s.OBJECT_TYPE = 'JOB'");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::Job, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::mviewlogDDLToRepo(DatabaseRepo &repo)
{
    QSqlQuery qr;

    qr.prepare("select s.LOG_TABLE, \
               dbms_metadata.get_ddl('MATERIALIZED_VIEW_LOG', s.LOG_TABLE) ddl \
            from USER_MVIEW_LOGS s");

    if (!qr.exec())
    {
        _errorString = qr.lastError().text();
        return false;
    }

    while (qr.next())
    {
        if (!repo.saveObject(DatabaseRepo::MViewLog, qr.value(0).toString(), qr.value(1).toString()))
        {
            _errorString = repo.lastError();
            return false;
        }
    }

    return true;
}

bool Database::dataToRepo(DatabaseRepo &repo, const QStringList &tables)
{
    for (int i=0; i < tables.count(); i++)
    {
        QSqlQuery qr;

        QString table = tables[i].trimmed();

        qr.prepare("select dbms_xmlgen.getxml('select * from '||:TABLE_NAME) xml from dual");
        qr.bindValue(":TABLE_NAME", table);

        if (!qr.exec())
        {
            _errorString = qr.lastError().text();
            return false;
        }

        while (qr.next())
        {
            if (!repo.saveData(table, qr.value(0).toString()))
            {
                _errorString = repo.lastError();
                return false;
            }
        }
    }

    return true;
}
