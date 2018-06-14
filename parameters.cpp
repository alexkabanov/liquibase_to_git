#include "parameters.h"
#include <getopt.h>

Parameters::Parameters(int argc, char *argv[]) :
    _argc(argc),
    _argv(argv),

    opt_db_name(0),
    opt_db_user(0),
    opt_db_password(0),
    opt_git_path(0),
    opt_git_url(0),
    opt_db_repo_path(0),
    opt_changesets_file(0)
{
}

Parameters::Parameters(const Parameters &parameters)
{
    set(parameters);
}

void Parameters::set(const Parameters &parameters)
{
    _argc = parameters._argc;
    _argv = parameters._argv;

    opt_db_name = parameters.opt_db_name;
    opt_db_user = parameters.opt_db_user;
    opt_db_password = parameters.opt_db_password;
    opt_git_path = parameters.opt_git_path;
    opt_git_url = parameters.opt_git_url;
    opt_git_user = parameters.opt_git_user;
    opt_git_email = parameters.opt_git_email;
    opt_db_repo_path = parameters.opt_db_repo_path;
    opt_changesets_file = parameters.opt_changesets_file;
    opt_tables = parameters.opt_tables;

    _db_name = parameters._db_name;
    _db_user = parameters._db_user;
    _db_password = parameters._db_password;
    _git_path = parameters._git_path;
    _git_url = parameters._git_url;
    _git_user = parameters._git_user;
    _git_email = parameters._git_email;
    _db_repo_path = parameters._db_repo_path;
    _changesets_file = parameters._changesets_file;
    _tables = parameters._tables;
    _errorString = parameters._errorString;
}

void Parameters::parse()
{
    int c;

    while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"dbname", required_argument, &opt_db_name, 1},
          {"dbuser", required_argument, &opt_db_user, 1},
          {"dbpass", required_argument, &opt_db_password, 1},
          {"gitpath", required_argument, &opt_git_path, 1},
          {"gituser", required_argument, &opt_git_user, 1},
          {"gitemail", required_argument, &opt_git_email, 1},
          {"dbrepo", required_argument, &opt_db_repo_path, 1},
          {"chfile", required_argument, &opt_changesets_file, 1},
          {"tables", required_argument, &opt_tables, 1},
          {"last", required_argument, &opt_last, 1},

          //{"gitpath", required_argument, 0, 'g'},
          //{"giturl",  required_argument, 0, 'b'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (_argc, _argv, "g:b:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          if (strcmp("dbname", long_options[option_index].name) == 0)
            if (optarg)
                _db_name = QString::fromLatin1(optarg);

          if (strcmp("dbuser", long_options[option_index].name) == 0)
            if (optarg)
                _db_user = QString::fromLatin1(optarg);

          if (strcmp("dbpass", long_options[option_index].name) == 0)
            if (optarg)
                _db_password = QString::fromLatin1(optarg);

          if (strcmp("gitpath", long_options[option_index].name) == 0)
            if (optarg)
                _git_path = QString::fromLatin1(optarg);

          if (strcmp("dbrepo", long_options[option_index].name) == 0)
            if (optarg)
                _db_repo_path = QString::fromLatin1(optarg);

          if (strcmp("chfile", long_options[option_index].name) == 0)
            if (optarg)
                _changesets_file = QString::fromLatin1(optarg);

          if (strcmp("tables", long_options[option_index].name) == 0)
            if (optarg)
                _tables = QString::fromLatin1(optarg);

          if (strcmp("gituser", long_options[option_index].name) == 0)
            if (optarg)
                _git_user = QString::fromLatin1(optarg);

          if (strcmp("gitemail", long_options[option_index].name) == 0)
            if (optarg)
                _git_email = QString::fromLatin1(optarg);

          if (strcmp("last", long_options[option_index].name) == 0)
            if (optarg)
            {
                bool ok;
                _last = QString::fromLatin1(optarg).toInt(&ok);
                if (!ok)
                {
                    _errorString = "parameter \"last\" must be an integer";
                }
            }

        break;
        //case 'c':
          //printf ("option -c with value `%s'\n", optarg);
          //break;

        //case '?':
          /* getopt_long already printed an error message. */
          //break;

        default:

          _errorString = "Using: liquibase_to_git [parameters]\n"
                  "Arguments\n"
                  "  --dbname=VALUE       Database TNS or connection string\n"
                  "  --dbuser=VALUE       Database username\n"
                  "  --dbpass=VALUE       Database password\n"
                  "  --gitpath=VALUE      Git repository\n"
                  "  --gituser=VALUE      Git user.name\n"
                  "  --gitemail=VALUE     Git user.email\n"
                  "  --dbrepo=VALUE       Path to SQL-files\n"
                  "  --chfile=VALUE       Liquebase XML-file\n"
                  "  --tables=VALUE       Data from tables\n"
                  "  --last=N             Only the last N scripts will be added to the repository\n";
          break;
        }
    }

}

