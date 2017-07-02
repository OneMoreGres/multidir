#-------------------------------------------------
#
# Project created by QtCreator 2017-03-29T21:46:46
#
#-------------------------------------------------

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

linux{
  QT += x11extras
  LIBS += -lX11
}
win32{
  LIBS += -lUser32
}

TARGET = multidir
TEMPLATE = app

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

APP_VERSION = "$$cat(version)"
DEFINES += APP_VERSION="$$APP_VERSION"

OTHER_FILES += \
    uncrustify.cfg \
    icons/README.md \
    LICENSE.md \
    translations/LICENSE_ru.md \
    README.md \
    utils/*

SOURCES += \
    main.cpp \
    dirwidget.cpp \
    globalaction.cpp \
    proxymodel.cpp \
    filesystemmodel.cpp \
    settings.cpp \
    copypaste.cpp \
    dirview.cpp \
    trash.cpp \
    openwith.cpp \
    tiledview.cpp \
    updatechecker.cpp \
    mainwindow.cpp \
    backgroundreader.cpp \
    debug.cpp \
    fileoperationwidget.cpp \
    fileoperation.cpp \
    groupwidget.cpp \
    filedelegate.cpp \
    notifier.cpp \
    groupcontrol.cpp \
    fileconflictresolver.cpp \
    utils.cpp \
    groupholder.cpp \
    shortcutmanager.cpp \
    propertieswidget.cpp

HEADERS  += \
    dirwidget.h \
    globalaction.h \
    proxymodel.h \
    filesystemmodel.h \
    settings.h \
    copypaste.h \
    dirview.h \
    constants.h \
    trash.h \
    openwith.h \
    tiledview.h \
    backport.h \
    updatechecker.h \
    mainwindow.h \
    backgroundreader.h \
    debug.h \
    fileoperationwidget.h \
    fileoperation.h \
    groupwidget.h \
    filedelegate.h \
    notifier.h \
    groupcontrol.h \
    fileconflictresolver.h \
    utils.h \
    groupholder.h \
    shortcutmanager.h \
    propertieswidget.h

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    translations/multidir_ru_RU.ts

QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = MultiDir
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
VERSION = $$APP_VERSION.0
RC_ICONS = icons/icon.ico

unix {
    PREFIX = /usr

    target.path = $$PREFIX/bin

    shortcuts.files = utils/multidir.desktop
    shortcuts.path = $$PREFIX/share/applications/
    pixmaps.files += icons/multidir.png
    pixmaps.path = $$PREFIX/share/pixmaps/
    translations.files += translations/*.qm
    translations.path = $$PREFIX/share/multidir/translations

    INSTALLS += target shortcuts pixmaps translations
}
