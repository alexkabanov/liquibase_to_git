#ifndef LQFILE_H
#define LQFILE_H

#include <QObject>
#include <QStringList>
#include <QString>

class LqFile
{
public:
    LqFile(const QString &fileName);

    bool open();

    int count() const;
    QString fileName(int i) const;

    inline QStringList files() {return _files;}

    inline QString lastError() {return _errorString;}

    bool saveWithMD5ToFile(const QString& file);
    
signals:
    
public slots:

private:
    QString _fileName;
    QStringList _files;
    QString _errorString;
    bool _is_open;
};

#endif // LQFILE_H
