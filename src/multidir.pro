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
mac{
  LIBS += -framework Carbon
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

APP_VERSION = "$$cat($$PWD/../version)"
DEFINES += APP_VERSION="$$APP_VERSION"

OTHER_FILES += \
    $$PWD/../uncrustify.cfg \
    $$PWD/../icons/README.md \
    $$PWD/../LICENSE.md \
    $$PWD/../translations/LICENSE_ru.md \
    $$PWD/../README.md \
    $$PWD/../utils/* \
    $$PWD/../styles/*

SOURCES += \
    main.cpp \
    dirwidget.cpp \
    globalaction.cpp \
    proxymodel.cpp \
    filesystemmodel.cpp \
    copypaste.cpp \
    dirview.cpp \
    trash.cpp \
    openwith.cpp \
    tiledview.cpp \
    updatechecker.cpp \
    mainwindow.cpp \
    backgroundreader.cpp \
    debug.cpp \
    fileoperation.cpp \
    groupwidget.cpp \
    filedelegate.cpp \
    notifier.cpp \
    fileconflictresolver.cpp \
    utils.cpp \
    shortcutmanager.cpp \
    propertieswidget.cpp \
    translationloader.cpp \
    filepermissions.cpp \
    filepermissiondelegate.cpp \
    filesystemcompleter.cpp \
    pathwidget.cpp \
    settingseditor.cpp \
    dirstatuswidget.cpp \
    storagemanager.cpp \
    settingsmanager.cpp \
    groupsview.cpp \
    groupsmenu.cpp \
    fileviewer.cpp \
    shellcommand.cpp \
    navigationhistory.cpp \
    styleoptionsproxy.cpp \
    styleloader.cpp \
    shellcommandwidget.cpp \
    shellcommandmodel.cpp \
    dirwidgetfactory.cpp \
    fileoperationmodel.cpp

HEADERS  += \
    dirwidget.h \
    globalaction.h \
    proxymodel.h \
    filesystemmodel.h \
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
    fileoperation.h \
    groupwidget.h \
    filedelegate.h \
    notifier.h \
    fileconflictresolver.h \
    utils.h \
    shortcutmanager.h \
    propertieswidget.h \
    translationloader.h \
    filepermissions.h \
    filepermissiondelegate.h \
    filesystemcompleter.h \
    pathwidget.h \
    settingseditor.h \
    dirstatuswidget.h \
    storagemanager.h \
    settingsmanager.h \
    groupsview.h \
    groupsmenu.h \
    fileviewer.h \
    shellcommand.h \
    navigationhistory.h \
    styleoptionsproxy.h \
    styleloader.h \
    shellcommandwidget.h \
    shellcommandmodel.h \
    dirwidgetfactory.h \
    fileoperationmodel.h

RESOURCES += \
    $$PWD/../resources.qrc

TRANSLATIONS += \
    $$PWD/../translations/multidir_ru.ts

QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = MultiDir
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
VERSION = $$APP_VERSION.0

win32 {
    RC_ICONS = $$PWD/../icons/icon.ico
}

linux {
    PREFIX = /usr

    target.path = $$PREFIX/bin

    shortcuts.files = $$PWD/../utils/multidir.desktop
    shortcuts.path = $$PREFIX/share/applications/
    pixmaps.files += $$PWD/../icons/multidir.png
    pixmaps.path = $$PREFIX/share/pixmaps/
    translations.files += $$PWD/../translations/*.qm
    translations.path = $$PREFIX/share/multidir/translations
    styles.files += $$PWD/../styles/*.css
    styles.path = $$PREFIX/share/multidir/styles

    INSTALLS += target shortcuts pixmaps translations styles
}

mac {
    ICON = $$PWD/../icons/multidir.icns

    translations.files = $$[QT_INSTALL_TRANSLATIONS]/qtbase_ru.qm $$PWD/../translations/multidir_ru.qm
    translations.path = Contents/Translations
    styles.files += $$PWD/../styles/dark.css
    styles.path = Contents/Resources/styles

    QMAKE_BUNDLE_DATA += translations styles
}
