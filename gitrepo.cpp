#include "gitrepo.h"
#include <QObject>

#include <QDir>
#include <QDirIterator>

void GitRepo::gitInit()
{
    git_libgit2_init();
}

void GitRepo::gitShutdown()
{
    git_libgit2_shutdown();
}

GitRepo::GitRepo(const QString &path) :
    repo(NULL),
    _path(path)
{
}

GitRepo::GitRepo(const QString &path, const QString &user, const QString &email) :
    repo(NULL),
    _path(path),
    _user(user),
    _email(email)

{

}

bool GitRepo::open()
{
    if (!git_repository_open(&repo, _path.toStdString().c_str()))
    {
        _lastError = QObject::tr("Could not open repository");
        return false;
    }
    return true;
}

bool GitRepo::init()
{
    if (git_repository_init(&repo, _path.toStdString().c_str(), 0))
    {
        _lastError = QObject::tr("Could not initialize repository");
        return false;
    }

    git_signature *sig;
    git_index *index;
    git_oid tree_id, commit_id;
    git_tree *tree;

    if (!_user.isEmpty() || !_email.isEmpty())
    {
        if (git_signature_now(&sig, _user.toStdString().c_str(), _email.toStdString().c_str()) < 0)
        {
            _lastError = QObject::tr("Unable to create a commit signature. User: '")+_user+QObject::tr("', email: '")+_email+QObject::tr("'");
            return false;
        }
    }
    else
    {
        if (git_signature_default(&sig, repo) < 0)
        {
            _lastError = QObject::tr("Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set");
            return false;
        }
    }

    if (git_repository_index(&index, repo) < 0)
    {
        _lastError = QObject::tr("Could not open repository index");
        return false;
    }

    if (git_index_write_tree(&tree_id, index) < 0)
    {
        _lastError = QObject::tr("Unable to write initial tree from index");
        git_index_free(index);
        return false;
    }

    git_index_free(index);

    if (git_tree_lookup(&tree, repo, &tree_id) < 0)
    {
        _lastError = QObject::tr("Could not look up tree");
        return false;
    }

    if (git_commit_create_v(
      &commit_id, repo, "HEAD", sig, sig,
      NULL, "Initial commit", tree, 0) < 0)
    {
        _lastError = QObject::tr("Could not create the initial commit");
        return false;
    }

    git_tree_free(tree);
    git_signature_free(sig);

    return true;
}

bool GitRepo::initialized()
{
    QDir dir;

    return dir.exists(_path+"/.git");
}

bool GitRepo::commit(const QString &message)
{
    git_signature *sig;
    git_oid tree_id, parent_id, commit_id;
    git_tree *tree;
    git_commit *parent;
    git_index *index;

    if (!_user.isEmpty() || !_email.isEmpty())
    {
        if (git_signature_now(&sig, _user.toStdString().c_str(), _email.toStdString().c_str()) < 0)
        {
            _lastError = QObject::tr("Unable to create a commit signature. User: '")+_user+QObject::tr("', email: '")+_email+QObject::tr("'");
            return false;
        }
    }
    else
    {
        if (git_signature_default(&sig, repo) < 0)
        {
            _lastError = QObject::tr("Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set");
            return false;
        }
    }

    if (git_repository_index(&index, repo) < 0)
    {
        _lastError = QObject::tr("Could not open repository index");
        return false;
    }
    if (git_index_write_tree(&tree_id, index) < 0)
    {
        _lastError = QObject::tr("Unable to write initial tree from index");
        git_index_free(index);
        return false;
    }

    git_index_free(index);


    if (git_tree_lookup(&tree, repo, &tree_id) < 0)
    {
        _lastError = QObject::tr("Could not look up tree");
        return false;
    }

    if (git_reference_name_to_id(&parent_id, repo, "HEAD"))
    {
        _lastError = QObject::tr("Could not look up reference \"HEAD\"");
        return false;
    }

    if (git_commit_lookup(&parent, repo, &parent_id))
    {
        _lastError = QObject::tr("Could not look up initial tree");
        return false;
    }

    /* Do the commit */
    if (git_commit_create_v(
            &commit_id,
            repo,
            "HEAD",     /* The commit will update the position of HEAD */
            sig,
            sig,
            NULL,       /* UTF-8 encoding */
                message.toUtf8().constData(),
            tree,       /* The tree from the index */
            1,          /* Only one parent */
            parent      /* No need to make a list with create_v */
        ) < 0)
    {
        _lastError = QObject::tr("Could not create commit");
        return false;
    }

    return true;
}

QStringList GitRepo::entities()
{
    QStringList res;

    git_index *index;
    unsigned int i, ecount;

    _lastError.clear();

    if (git_repository_index(&index, repo) < 0)
    {
        _lastError = QObject::tr("Could not open repository index");
        return res;
    }

    ecount = git_index_entrycount(index);

    for (i = 0; i < ecount; ++i)
    {
        const git_index_entry *e = git_index_get_byindex(index, i);

        QString path(e->path);
        res.append(path);
    }

    git_index_free(index);

    return res;
}

bool GitRepo::addToIndex(const QString &path)
{
    QDir dir(_path);

    if (!dir.exists(path))
    {
        _lastError = QObject::tr("File not found '") + path + QObject::tr("' in repository");
        return false;
    }

    QString relPath = dir.relativeFilePath(path);

    git_index *index;

    if (git_repository_index(&index, repo) < 0)
    {
        _lastError = QObject::tr("Could not open repository index");
        return false;
    }

    if (git_index_add_bypath(index, relPath.toStdString().c_str()) < 0)
    {
        _lastError = QObject::tr("Could not add '") + path + QObject::tr("' into repository index");
        git_index_free(index);
        return false;
    }

    if (git_index_write(index) < 0)
    {
        _lastError = QObject::tr("Could not write index");
        git_index_free(index);
        return false;
    }

    git_index_free(index);

    return true;
}

bool GitRepo::removeFromIndex(const QString &path)
{
    QDir dir(_path);

    QString relPath = dir.relativeFilePath(path);

    git_index *index;

    if (git_repository_index(&index, repo) < 0)
    {
        _lastError = QObject::tr("Could not open repository index");
        return false;
    }

    if (git_index_remove_bypath(index, relPath.toStdString().c_str()) < 0)
    {
        _lastError = QObject::tr("Could not remove '") + path + QObject::tr("' into repository index");
        git_index_free(index);
        return false;
    }

    if (git_index_write(index) < 0)
    {
        _lastError = QObject::tr("Could not write index");
        git_index_free(index);
        return false;
    }

    git_index_free(index);

    return true;
}

bool GitRepo::updateIndex()
{
    QStringList workingDirFiles;
    QDir dir(_path);
    QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString path = it.next();
        if (!it.fileInfo().isDir())
            workingDirFiles << dir.relativeFilePath(path);
    }

    QStringList ents = entities();

    for (int i=0; i<ents.count(); i++)
    {
        if (workingDirFiles.indexOf(ents[i]) < 0)
        {
            if (!removeFromIndex(ents[i]))
            {
                return false;
            }
        }
    }


    for (int i=0; i<workingDirFiles.count(); i++)
    {
        if (!addToIndex(workingDirFiles[i]))
        {
            return false;
        }
    }

    return true;

}


GitRepo::~GitRepo()
{
    if (repo)
        git_repository_free(repo);

}
