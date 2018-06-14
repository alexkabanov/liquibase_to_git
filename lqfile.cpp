#include "lqfile.h"
#include <QFile>
#include <QDomDocument>
#include <QDomAttr>
#include <QDomElement>
#include "sqlfile.h"

LqFile::LqFile(const QString &fileName) : _fileName(fileName)
{
    _is_open = false;
}

bool LqFile::open() {

    QDomDocument doc;

    QString errorStr;
    int errorLine;
    int errorColumn;

    QFile file(_fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        _errorString = (QObject::tr("Could not open liquibase file '%1'").arg(_fileName));
        return false;
    }
    if (!doc.setContent(&file, true, &errorStr, &errorLine,
                                   &errorColumn))
    {
           _errorString = (QObject::tr("Liquibase file parse error at line %1, column %2:\n%3")
                                    .arg(errorLine)
                                    .arg(errorColumn)
                                    .arg(errorStr));
           file.close();
           return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName().toLower() != "databasechangelog")
    {
        _errorString = QObject::tr("Root element in liquibase file is not ""databaseChangeLog""");
        return false;
    }

    QDomElement child = root.firstChildElement("include");
    while (!child.isNull())
    {

        _files.append(child.attribute("file"));

        child = child.nextSiblingElement("include");
    }

    _is_open = true;

    return true;
}

int LqFile::count() const
{
    return _files.count();
}

QString LqFile::fileName(int i) const
{
    return _files[i];
}


