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
    src/main.cpp \
    src/dirwidget.cpp \
    src/globalaction.cpp \
    src/proxymodel.cpp \
    src/filesystemmodel.cpp \
    src/copypaste.cpp \
    src/dirview.cpp \
    src/trash.cpp \
    src/openwith.cpp \
    src/tiledview.cpp \
    src/updatechecker.cpp \
    src/mainwindow.cpp \
    src/backgroundreader.cpp \
    src/debug.cpp \
    src/fileoperationwidget.cpp \
    src/fileoperation.cpp \
    src/groupwidget.cpp \
    src/filedelegate.cpp \
    src/notifier.cpp \
    src/fileconflictresolver.cpp \
    src/utils.cpp \
    src/shortcutmanager.cpp \
    src/propertieswidget.cpp \
    src/translationloader.cpp \
    src/filepermissions.cpp \
    src/filepermissiondelegate.cpp \
    src/filesystemcompleter.cpp \
    src/pathwidget.cpp \
    src/settingseditor.cpp \
    src/dirstatuswidget.cpp \
    src/storagemanager.cpp \
    src/settingsmanager.cpp \
    src/groupsview.cpp \
    src/groupsmenu.cpp \
    src/fileviewer.cpp \
    src/shellcommand.cpp

HEADERS  += \
    src/dirwidget.h \
    src/globalaction.h \
    src/proxymodel.h \
    src/filesystemmodel.h \
    src/copypaste.h \
    src/dirview.h \
    src/constants.h \
    src/trash.h \
    src/openwith.h \
    src/tiledview.h \
    src/backport.h \
    src/updatechecker.h \
    src/mainwindow.h \
    src/backgroundreader.h \
    src/debug.h \
    src/fileoperationwidget.h \
    src/fileoperation.h \
    src/groupwidget.h \
    src/filedelegate.h \
    src/notifier.h \
    src/fileconflictresolver.h \
    src/utils.h \
    src/shortcutmanager.h \
    src/propertieswidget.h \
    src/translationloader.h \
    src/filepermissions.h \
    src/filepermissiondelegate.h \
    src/filesystemcompleter.h \
    src/pathwidget.h \
    src/settingseditor.h \
    src/dirstatuswidget.h \
    src/storagemanager.h \
    src/settingsmanager.h \
    src/groupsview.h \
    src/groupsmenu.h \
    src/fileviewer.h \
    src/shellcommand.h

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    translations/multidir_ru.ts

QMAKE_TARGET_COMPANY = Gres
QMAKE_TARGET_PRODUCT = MultiDir
QMAKE_TARGET_COPYRIGHT = Copyright (c) Gres
VERSION = $$APP_VERSION.0

win32 {
    RC_ICONS = icons/icon.ico
}

linux {
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

mac {
    ICON = icons/multidir.icns

    translations.files = $$[QT_INSTALL_TRANSLATIONS]/qtbase_ru.qm translations/multidir_ru.qm
    translations.path = Contents/Translations
    QMAKE_BUNDLE_DATA += translations
}
