#include "converttask.h"
#include "lqfile.h"
#include "sqlfile.h"
#include "gitrepo.h"
#include "database.h"
#include "databaserepo.h"
#include "databasechangelog.h"
#include "databaserepochangelog.h"

#include <QTextStream>
#include <QStringList>
#include <QFileInfo>


Convertor::Convertor(QObject *parent, int argc, char *argv[]) :
    QObject(parent), _params(argc, argv)
{

}

void Convertor::setParameters(const Parameters &parameters)
{
    _params.set(parameters);
}

void Convertor::run()
{
    QTextStream ts(stdout);

    _params.parse();

    if (!_params.lastError().isEmpty())
    {
        ts << _params.lastError();
        return;
    }

    LqFile lqFile(_params.changesetsFile());

    if (!lqFile.open())
    {
        ts << lqFile.lastError() << endl;

        emit finished();
        return;
    }

    Database db;
    db.setDBName(_params.dbName());
    db.setUser(_params.dbUser());
    db.setPass(_params.dbPassword());

    if (!db.connectDB())
    {
        ts << db.lastError() << endl;

        emit finished();
        return;
    }

    _dataTables = _params.tables().split(",");

    DatabaseChangeLog dbChangeLog;
    dbChangeLog.setDatabase(&db);
    if (!dbChangeLog.open())
    {
        ts << db.lastError() << endl;
        emit finished();
        return;
    }

    GitRepo::gitInit();


    GitRepo repo(_params.gitPath(), _params.gitUser(), _params.gitEmail());

    if (!repo.init())
    {
        ts << repo.lastError() << endl;

        GitRepo::gitShutdown();
        emit finished();
        return;
    }


    DatabaseRepo dbRepo(_params.dbRepoPath());

    if (!db.DDLToRepo(dbRepo))
    {
        ts << db.lastError() << endl;

        GitRepo::gitShutdown();
        emit finished();
        return;
    }

    if (!db.dataToRepo(dbRepo, _dataTables))
    {
        ts << db.lastError() << endl;

        GitRepo::gitShutdown();
        emit finished();
        return;
    }

    DatabaseRepoChangeLog dbRepoCh(_params.dbRepoPath()+"/changelog.xml");
    QFileInfo check_file(_params.dbRepoPath()+"/changelog.xml");
    if (check_file.exists())
    {
        if (!dbRepoCh.open())
        {
            ts << dbRepoCh.lastError() << endl;

            GitRepo::gitShutdown();
            emit finished();
            return;
        }
    }
    else
    {
        dbRepoCh.init(dbChangeLog, lqFile);
        if (!dbRepoCh.save())
        {
            ts << dbRepoCh.lastError() << endl;

            GitRepo::gitShutdown();
            emit finished();
            return;
        }
    }

    if (!repo.updateIndex())
    {
        ts << repo.lastError() << endl;

        GitRepo::gitShutdown();
        emit finished();
        return;
    }

    if (!repo.commit("First"))
    {
        ts << repo.lastError() << endl;

        GitRepo::gitShutdown();
        emit finished();
        return;
    }


    for (int i=0; i<lqFile.count(); i++)
    {
        if (!dbChangeLog.checkFile(lqFile.fileName(i)))
        {
            ts << lqFile.fileName(i) << endl;

            SqlFile sqlFile(lqFile.fileName(i));

            if (!sqlFile.open())
            {
                ts << sqlFile.lastError() << endl;
                GitRepo::gitShutdown();
                emit finished();
                return;
            }

            QString comment = sqlFile.firstHumanComment();

            if (!db.execSQLFile(lqFile.fileName(i)))
            {
                ts << db.lastError() << endl;
                GitRepo::gitShutdown();
                emit finished();
                return;
            }

            if (!_params.isSetLast() || (_params.last() >= lqFile.count() - i))
            {
                if (!dbRepo.clear())
                {
                    ts << dbRepo.lastError() << endl;
                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }

                if (!db.DDLToRepo(dbRepo))
                {
                    ts << db.lastError() << endl;

                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }

                if (!db.dataToRepo(dbRepo, _dataTables))
                {
                    ts << db.lastError() << endl;

                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }

                dbRepoCh.appendFile(lqFile.fileName(i), sqlFile.MD5());
                if (!dbRepoCh.save())
                {
                    ts << dbRepoCh.lastError() << endl;

                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }

                if (!repo.updateIndex())
                {
                    ts << repo.lastError() << endl;

                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }

                if (!repo.commit(lqFile.fileName(i) + "\n" + comment))
                {
                    ts << repo.lastError() << endl;

                    GitRepo::gitShutdown();
                    emit finished();
                    return;
                }
            }
        }
    }


    GitRepo::gitShutdown();

    emit finished();
}
