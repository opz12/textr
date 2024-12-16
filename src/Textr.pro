
QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Textr
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11


SOURCES += \
    code_highlighters/highlighter.cpp \
    code_highlighters/chighlighter.cpp \
    code_highlighters/cpphighlighter.cpp \
    code_highlighters/javahighlighter.cpp \
    code_highlighters/pythonhighlighter.cpp \
    main.cpp \
    mainwindow.cpp \
    finddialog.cpp \
    editor.cpp \
    metricreporter.cpp \
    settings.cpp \
    utilityfunctions.cpp \
    searchhistory.cpp \
    gotodialog.cpp \
    tabbededitor.cpp \
    language.cpp

HEADERS += \
    code_highlighters/highlighter.h \
    code_highlighters/chighlighter.h \
    code_highlighters/cpphighlighter.h \
    code_highlighters/javahighlighter.h \
    code_highlighters/pythonhighlighter.h \
    mainwindow.h \
    documentmetrics.h \
    finddialog.h \
    editor.h \
    linenumberarea.h \
    metricreporter.h \
    settings.h \
    utilityfunctions.h \
    searchhistory.h \
    gotodialog.h \
    tabbededitor.h \
    language.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=

RC_FILE = texteditor.rc


