#include "databaserepochangelog.h"

#include <QFile>
#include "sqlfile.h"

DatabaseRepoChangeLog::DatabaseRepoChangeLog(const QString &file): _file(file)
{
}


bool DatabaseRepoChangeLog::open()
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    QFile file(_file);
    if (!file.open(QIODevice::ReadOnly))
    {
        _errorString = (QObject::tr("Could not open changelog file '%1'").arg(_file));
        return false;
    }
    if (!doc.setContent(&file, true, &errorStr, &errorLine,
                                   &errorColumn))
    {
           _errorString = (QObject::tr("Changelog file parse error at line %1, column %2:\n%3")
                                    .arg(errorLine)
                                    .arg(errorColumn)
                                    .arg(errorStr));
           file.close();
           return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName().toLower() != "databaseChangeLog")
    {
        _errorString = QObject::tr("Root element in changelog file is not ""databaseChangeLog""");
        return false;
    }

    return true;
}

bool DatabaseRepoChangeLog::checkFile(const QString& file, const QString& md5)
{
    QDomElement child = doc.documentElement().firstChildElement("include");
    while (!child.isNull())
    {

        if (child.attribute("file") == file)
        {
            if(child.attribute("MD5") == md5)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        child = child.nextSiblingElement("include");
    }


    return false;
}

bool DatabaseRepoChangeLog::checkFile(const QString& file)
{
    QDomElement child = doc.documentElement().firstChildElement("include");
    while (!child.isNull())
    {

        if (child.attribute("file") == file)
        {
            return true;
        }

        child = child.nextSiblingElement("include");
    }

    return false;
}

void DatabaseRepoChangeLog::init(const DatabaseChangeLog &dbChLog, const LqFile &lqFile)
{
    doc.clear();
    QDomElement root = doc.createElement("databaseChangeLog");
    doc.appendChild(root);


    for (int i = 0; i < lqFile.count(); i++)
    {
        QString file = lqFile.fileName(i);

        if (dbChLog.checkFile(file))
        {
            QString md5 = SqlFile::MD5(file);
            appendFile(file, md5);
        }
    }
}

void DatabaseRepoChangeLog::appendFile(const QString &file, const QString &md5)
{
    QDomElement include = doc.createElement("include");
    QDomAttr afile = doc.createAttribute("file");
    QDomAttr amd5 = doc.createAttribute("MD5");
    afile.setValue(file);
    amd5.setValue(md5);
    include.setAttributeNode(afile);
    include.setAttributeNode(amd5);
    QDomElement root = doc.documentElement();
    root.appendChild(include);
}


bool DatabaseRepoChangeLog::save()
{
    QFile fl(_file);

    if (!fl.open(QIODevice::WriteOnly) )
    {
        _errorString = "Could not create changelog file '" + _file + "'";
        return false;
    }
    fl.write(doc.toByteArray());
    fl.close();

    return true;
}
