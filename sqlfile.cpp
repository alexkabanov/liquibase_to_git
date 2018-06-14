#include "sqlfile.h"
#include <QByteArray>
#include <QRegExp>
#include <QVector>
#include <QTextCodec>
#include <QTextStream>
#include <QCryptographicHash>

SqlFileStatment & SqlFileStatment::operator=(const SqlFileStatment &stmt)
{

    this->_type = stmt.type();
    this->_text = stmt.text();
    return *this;
}



SqlFile::SqlFile(const QString & name) :
    _currPos(0),
    _fileName(name)
{
    unsetError();
}

bool SqlFile::open()
{
    QFile file(_fileName);

    _content.clear();
    _currPos = 0;
    unsetError();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        _error = SqlFile::FileError;
        setErrorString(file.errorString());
        return false;
    }

    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName( "CP1251" ));
    _content = in.readAll();
    if (file.error() != QFile::NoError)
    {
        _error = SqlFile::FileError;
        setErrorString(file.errorString());
        return false;
    }

    return true;
}

void SqlFile::setErrorString(const QString &err)
{
    _errorString = err;
}

void SqlFile::unsetError()
{
    _error = NoError;
    _errorString.clear();
}



class RegExpStartStmt
{

public:

    inline RegExpStartStmt() :
        _exp(),
        _type(SqlFileStatment::Unknown)
    {
    }

    inline RegExpStartStmt(const QRegExp & regexp, SqlFileStatment::StatmentType type) :
        _exp(regexp),
        _type(type)
    {
    }

    inline QRegExp exp() {return _exp;}
    inline SqlFileStatment::StatmentType type() {return _type;}

private:
    QRegExp _exp;
    SqlFileStatment::StatmentType _type;
};

void SqlFile::SqlFile::seek(qint64 pos)
{
    _currPos = pos;
}

int SqlFile::findFirst(const QString& str, const QVector<QRegExp> &ptr, int startPosition, int &foundPosition, int &foundLength)
{

    int pos;
    int ptrIdx = -1;
    int length = 0;
    int minFoundPos = str.length();

    for (int i = 0; i < ptr.count(); i++)
    {
        QRegExp rx = ptr[i];
        pos = rx.indexIn(str, startPosition);
        if (pos != -1 && pos < minFoundPos)
        {
            ptrIdx = i;
            length = rx.matchedLength();
            minFoundPos = pos;
        }
    }

    if (ptrIdx != -1)
    {
        foundPosition = minFoundPos;
        foundLength = length;
    }

    return ptrIdx;
}

int SqlFile::findFirstBlock(const QString& str, const QVector<BlockPattern> &ptrs, int startPosition, int &foundPosition, int &foundLength)
{
    QVector<QRegExp> beginPtrs;

    for (int i = 0; i < ptrs.count(); i++)
    {
        if (!ptrs[i].wholePtr.isEmpty())
            beginPtrs << ptrs[i].wholePtr;
        else
            beginPtrs << ptrs[i].startPtr;
    }

    int beginPos, beginLength;

    int ptrIdx = findFirst(str, beginPtrs, startPosition, beginPos, beginLength);

    if (ptrIdx == -1)
        return ptrIdx;

    foundPosition = beginPos;

    if (ptrs[ptrIdx].wholePtr.isEmpty())
    {
        QRegExp rx = ptrs[ptrIdx].endPtr;
        int endPos = rx.indexIn(str, beginPos+beginLength);
        if (endPos != -1)
        {
            int endLength = rx.matchedLength();
            foundLength = endPos+endLength - beginPos;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        foundLength = beginLength;
    }

    return ptrIdx;
}

int SqlFile::findEndSqlBlock(const QString& str, int startPosition)
{
    QVector<BlockPattern> SqlPtrs;

    SqlPtrs << BlockPattern{QRegExp("(;|$|\\n[\\f\\r\\t\\v]*/[\\f\\r\\t\\v]*\\n)"), QRegExp(), QRegExp()};
    SqlPtrs << BlockPattern{QRegExp("--[^\\n]*"), QRegExp(), QRegExp()};
    SqlPtrs << BlockPattern{QRegExp(), QRegExp("/\\*"), QRegExp("\\*/")};
    SqlPtrs << BlockPattern{QRegExp(), QRegExp("\""), QRegExp("\"")};
    SqlPtrs << BlockPattern{QRegExp(), QRegExp("'"), QRegExp("'")};

    int idx;

    int pos = startPosition;

    int endSqlBlock;

    do
    {
        int blockPosition, blockLength;
        idx = findFirstBlock(str, SqlPtrs, pos, blockPosition, blockLength);

        pos = blockPosition + blockLength;

        endSqlBlock = blockPosition;

    } while (idx != -1 && idx != 0);

    if (idx == 0)
    {
       return endSqlBlock;
    }

    return -1;
}

SqlFileStatment SqlFile::nextStatment()
{
    unsetError();

    QVector<RegExpStartStmt> stmtStartRegExp;
    stmtStartRegExp << RegExpStartStmt(QRegExp("--[^\\n]*"), SqlFileStatment::SingleLineComment);
    stmtStartRegExp << RegExpStartStmt(QRegExp("/\\*"), SqlFileStatment::MultiLineComment);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*create(\\s*or\\s*replace)?\\s*(function|library|package|procedure|trigger|type)", Qt::CaseInsensitive), SqlFileStatment::PLSQL);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*create(\\s*or\\s*replace)?(\\s*and\\s*resolve)?(\\s*and\\s*compile)?(\\s*force)?\\s*java", Qt::CaseInsensitive), SqlFileStatment::PLSQL);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*begin", Qt::CaseInsensitive), SqlFileStatment::PLSQL);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*declare", Qt::CaseInsensitive), SqlFileStatment::PLSQL);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*alter|analyze|associate|audit|call|comment|commit|create|delete|disassociate|drop|explain|flashback|grant|insert|lock|merge|noaudit|purge|rename|revoke|rollback|savepoint|select|set\\s*constraint|set\\s*role|set\\s*transaction|truncate|update", Qt::CaseInsensitive), SqlFileStatment::SQL);
    stmtStartRegExp << RegExpStartStmt(QRegExp("[\\f\\r\\t\\v]*\\w+\\n", Qt::CaseInsensitive), SqlFileStatment::SQLPlusCommand);

    int minFoundPos = _content.length();
    SqlFileStatment::StatmentType foundType = SqlFileStatment::Unknown;
    int foundLength = 0;

    SqlFileStatment res;

    QRegExp fw("\\w", Qt::CaseInsensitive);
    int firstWord = fw.indexIn(_content, _currPos);
    if (firstWord == -1)
    {
        return res;
    }

    int pos;

    do
    {
        foundType = SqlFileStatment::Unknown;
        minFoundPos = _content.length();
        foundLength = 0;

        for (int i = 0; i < stmtStartRegExp.count(); i++)
        {
            QRegExp rx = stmtStartRegExp[i].exp();
            pos = rx.indexIn(_content, _currPos);
            if (pos != -1 && pos < minFoundPos)
            {
                foundType = stmtStartRegExp[i].type();
                foundLength = rx.matchedLength();
                minFoundPos = pos;
            }
        }
        if (foundType == SqlFileStatment::SQLPlusCommand)
        {
            _currPos = minFoundPos + foundLength;
        }

    } while (foundType == SqlFileStatment::SQLPlusCommand);

    if (foundType == SqlFileStatment::SingleLineComment)
    {
        QRegExp slEx("--[^\\n]*");
        pos = slEx.indexIn(_content, _currPos);
        if (pos != -1)
        {
            _currPos = minFoundPos + slEx.matchedLength();

            return SqlFileStatment(slEx.cap(0), foundType);
        }
        else
        {
            _error = SqlFile::SyntaxError;
            setErrorString(QObject::tr("Ошибка определения границ однострочного коментария"));
            return res;
        }
    }
    else if (foundType == SqlFileStatment::MultiLineComment)
    {
        QRegExp eEx("\\*/");
        int startPos = minFoundPos;

        int endPos = eEx.indexIn(_content, startPos + foundLength);
        if (endPos != -1)
        {
            _currPos = endPos + eEx.matchedLength();
            return SqlFileStatment(_content.mid(startPos, _currPos-startPos), foundType);
        }
        else
        {
            _error = SqlFile::SyntaxError;
            setErrorString(QObject::tr("Не найдено окончание многострочного коментария"));
            return res;
        }
    }
    else if (foundType == SqlFileStatment::PLSQL)
    {
        QRegExp eEx("\\n[\\f\\r\\t\\v]*/[\\f\\r\\t\\v]*\\n");
        int startPos = minFoundPos;

        int endPos = eEx.indexIn(_content, startPos + foundLength);
        if (endPos != -1)
        {
            _currPos = endPos;

            return SqlFileStatment(_content.mid(startPos, _currPos-startPos), foundType);
        }
        else
        {
            _error = SqlFile::SyntaxError;
            setErrorString(QObject::tr("Не найдено окончание PLSQL блока"));
            return res;
        }
    }
    else if (foundType == SqlFileStatment::SQL)
    {
        int startPos = minFoundPos;

        int endPos = findEndSqlBlock(_content, startPos + foundLength);
        if (endPos != -1)
        {
            _currPos = endPos;

            return SqlFileStatment(_content.mid(startPos, _currPos-startPos), foundType);
        }
        else
        {
            _error = SqlFile::SyntaxError;
            setErrorString(QObject::tr("Не найдено окончание SQL блока"));
            return res;
        }
    }

    return res;
}

QString SqlFile::firstHumanComment()
{
    int prevPos = _currPos;

    QString res;

    _currPos = 0;


    SqlFileStatment st;
    do
    {
        st = nextStatment();
        res = st.text();

        QRegExp rxLQSql("\\s*-{2,}\\s*liquibase\\s+formatted\\s+sql");
        QRegExp rxLQChangeset("\\s*-{2,}\\s*changeset");

        if (rxLQSql.indexIn(res) < 0 && rxLQChangeset.indexIn(res) < 0)
        {
            _currPos = prevPos;

            QRegExp rxClear("(\\s*-{2,}\\s*)|(^\\s*/\\*\\s*)|(\\s*\\*/\\s*$)");
            res.replace(rxClear, "");
            return res;
        }
    }
    while (st.type() == SqlFileStatment::SingleLineComment
           || st.type() == SqlFileStatment::MultiLineComment);

    _currPos = prevPos;
    return QString();
}

QString SqlFile::MD5()
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(_content.toLatin1());

    return hash.result().toHex();
}


QString SqlFile::MD5(const QString &file)
{
    QFile f(file);

    if (f.open(QFile::ReadOnly))
    {
        QCryptographicHash hash(QCryptographicHash::Md5);

        QByteArray buff = f.readAll();
        hash.addData(buff);
        {
            return hash.result().toHex();
        }
    }
    return QString();

}
