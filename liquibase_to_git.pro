#-------------------------------------------------
#
# Project created by QtCreator 2017-09-06T16:48:59
#
#-------------------------------------------------

QT       += core
QT       += xml
QT       += sql

QT       -= gui

TARGET = liquibase_to_git
CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH += C:\dev\libgit2-0.26.0\libgit2-0.26.0\include

LIBS += -LC:\dev\libgit2-0.26.0\libgit2-0.26.0\build\Release -lgit2

TEMPLATE = app


SOURCES += main.cpp \
    converttask.cpp \
    lqfile.cpp \
    sqlfile.cpp \
    database.cpp \
    gitrepo.cpp \
    databaserepo.cpp \
    databasechangelog.cpp \
    parameters.cpp \
    databaserepochangelog.cpp

HEADERS += \
    converttask.h \
    lqfile.h \
    sqlfile.h \
    database.h \
    gitrepo.h \
    databaserepo.h \
    databasechangelog.h \
    parameters.h \
    databaserepochangelog.h
