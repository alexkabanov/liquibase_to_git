#ifndef SQLFILE_H
#define SQLFILE_H

#include <QFile>
#include <QString>

struct BlockPattern {
    QRegExp wholePtr;
    QRegExp startPtr;
    QRegExp endPtr;
};

class SqlFileStatment
{
public:

    enum StatmentType {
        Unknown = 0,
        SingleLineComment = 1,
        MultiLineComment = 2,
        PLSQL = 3,
        SQL = 4,
        SQLPlusCommand = 5,
        StringConst = 6
    };
    inline SqlFileStatment() : _text(), _type(SqlFileStatment::Unknown) {}

    inline SqlFileStatment(const QString & str, StatmentType type) : _text(str), _type(type) {}

    SqlFileStatment &operator=(const SqlFileStatment &);


    inline StatmentType type() const {return _type;}
    inline QString text() const {return _text;}

private:
    QString _text;
    StatmentType _type;
};



class SqlFile
{
public:

    enum SqlFileError {
        NoError = 0,
        UnknownError = 1,
        FileError = 2,
        SyntaxError = 3
    };

    SqlFile(const QString & name);

    bool open();
    void seek(qint64 pos);

    QString firstHumanComment();
    QString MD5();

    SqlFileStatment nextStatment();

    inline SqlFileError error() {return _error;}
    inline QString lastError() {return _errorString;}

    static QString MD5(const QString &file);
    
signals:
    
public slots:

protected:
    void setErrorString(const QString &err);
    void unsetError();
    static int findFirst(const QString& str, const QVector<QRegExp> &ptr, int startPosition, int &foundPosition, int &foundLength);
    static int findFirstBlock(const QString& str, const QVector<BlockPattern> &ptrs, int startPosition, int &foundPosition, int &foundLength);
    static int findEndSqlBlock(const QString& str, int startPosition);

private:
    int _currPos;
    QString _content;
    QString _fileName;
    SqlFileError _error;
    QString _errorString;
};

#endif // SQLFILE_H
