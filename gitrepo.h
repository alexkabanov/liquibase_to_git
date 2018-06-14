#ifndef GITREPO_H
#define GITREPO_H

#include <git2.h>
#include <QString>
#include <QStringList>
#include <databaserepo.h>

class GitRepo
{
public:
    GitRepo(const QString &path);
    GitRepo(const QString &path, const QString &user, const QString &email);

    ~GitRepo();

    bool init();
    bool open();

    bool initialized();



    bool commit(const QString &message);
    QStringList entities();

    bool addToIndex(const QString &path);
    bool removeFromIndex(const QString &path);
    bool updateIndex();

    inline QString lastError() {return _lastError;}

    static void gitInit();
    static void gitShutdown();



private:
    git_repository *repo;

    QString _path;
    QString _user;
    QString _email;

    QString _lastError;

};

#endif // GITREPO_H
