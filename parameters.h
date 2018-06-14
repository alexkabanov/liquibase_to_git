#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <QString>

#define ARGUMENT_VALUE_MAX_SIZE 255

class Parameters
{
public:
    Parameters(int argc, char *argv[]);
    Parameters(const Parameters &parameters);

    void set(const Parameters &parameters);

    void parse();

    inline bool isSetDBName() {return (opt_db_name == 1);}
    inline bool isSetDBUser() {return (opt_db_user == 1);}
    inline bool isSetDBPassword() {return (opt_db_password == 1);}
    inline bool isSetDBRepo() {return (opt_db_repo_path == 1);}
    inline bool isSetGitPath() {return (opt_git_path == 1);}
    inline bool isSetGitUser() {return (opt_git_user == 1);}
    inline bool isSetGitEmail() {return (opt_git_email == 1);}
    inline bool isSetChangesetsFile() {return (opt_changesets_file == 1);}
    inline bool isSetTables() {return (opt_tables == 1);}
    inline bool isSetLast() {return (opt_last == 1);}

    inline QString dbName() {return _db_name;}
    inline QString dbUser() {return _db_user;}
    inline QString dbPassword() {return _db_password;}
    inline QString dbRepoPath() {return _db_repo_path;}
    inline QString gitPath() {return _git_path;}
    inline QString gitUser() {return _git_user;}
    inline QString gitEmail() {return _git_email;}
    inline QString changesetsFile() {return _changesets_file;}
    inline QString tables() {return _tables;}
    inline int last() {return _last;}

    inline QString lastError() {return _errorString;}

private:
    int _argc;
    char **_argv;

    int opt_db_name;
    int opt_db_user;
    int opt_db_password;
    int opt_git_path;
    int opt_git_url;
    int opt_git_user;
    int opt_git_email;
    int opt_db_repo_path;
    int opt_changesets_file;
    int opt_tables;
    int opt_last;

    QString _db_name;
    QString _db_user;
    QString _db_password;
    QString _git_path;
    QString _git_url;
    QString _git_user;
    QString _git_email;
    QString _db_repo_path;
    QString _changesets_file;
    QString _tables;
    int _last;

    QString _errorString;
};

#endif // PARAMETERS_H
