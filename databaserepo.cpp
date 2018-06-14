#include "databaserepo.h"

#include <QDirIterator>
#include <QTextStream>

DatabaseRepo::DatabaseRepo(const QString &path)
{
    _path = path;
}

bool DatabaseRepo::saveObject(ObjectType type, const QString &name, const QString &ddl)
{
    if (name.isEmpty())
    {
        _errorString = "Database object name can not be empty";
        return false;
    }

    QString objectPath = _path + "/" + typeDirrectoryById(type) + "/";
    QDir dir;

    QFileInfo fi(objectPath + "/" + name + ".sql");

    QString fpath = fi.path();

    if (!dir.exists(fpath))
    {
        if (!dir.mkpath(fpath))
        {
            _errorString = "Could not create directory '" + fpath + "'";
            return false;
        }
    }

    QFile file(fi.filePath());

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        _errorString = "Could not create file '" + fi.filePath() + "'";
        return false;
    }

    QTextStream out(&file);

    out << ddl;

    return true;
}

bool DatabaseRepo::saveData(const QString &name, const QString &ddl)
{
    if (name.isEmpty())
    {
        _errorString = "Database data name can not be empty";
        return false;
    }

    QString dataPath = _path+"/data/";
    QDir dir;

    if (!dir.exists(dataPath))
    {
        if (!dir.mkpath(dataPath))
        {
            _errorString = "Could not create directory '" + dataPath + "'";
            return false;
        }
    }

    QFile file(dataPath + name + ".xml");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        _errorString = "Could not create file '" + dataPath + name + ".xml'";
        return false;
    }

    QTextStream out(&file);

    out << ddl;

    return true;
}

bool DatabaseRepo::clear()
{
    QDir dir(_path);
    QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString path = it.next();
        if (!it.fileInfo().isDir())
        {
            if (!dir.remove(path))
            {
                _errorString = "Could not empty the repository. Error while deleting file '"+path+"'";
                return false;
            }
        }
    }

    return true;
}

QStringList DatabaseRepo::allObjects()
{
    QStringList res;
    QDir dir(_path);
    QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString path = it.next();
        if (!it.fileInfo().isDir())
            res << dir.relativeFilePath(path);
    }

    return res;
}


QString DatabaseRepo::typeDirrectoryById(DatabaseRepo::ObjectType objType)
{
    if (objType == Table)
        return "tables";
    else if (objType == View)
        return "views";
    else if (objType == Procedure)
        return "procedures";
    else if (objType == Function)
        return "functions";
    else if (objType == Package)
        return "packages";
    else if (objType == MView)
        return "mview";
    else if (objType == MViewLog)
        return "mviewlog";
    else if (objType == Sequence)
        return "sequence";
    else if (objType == Java)
        return "java";
    else if (objType == DBLink)
        return "dblink";
    else if (objType == Type)
        return "type";
    else if (objType == Job)
        return "job";
    else if (objType == Unknown)
        return "other";
    return "other";
}

DatabaseRepo::ObjectType DatabaseRepo::typeIdByName(const QString &type)
{
    if (type == "TABLE")
        return Table;
    else if (type == "VIEW")
        return View;
    else if (type == "PROCEDURE")
        return Procedure;
    else if (type == "FUNCTION")
        return Function;
    else if (type == "PACKAGE")
        return Package;
    return Unknown;
}
