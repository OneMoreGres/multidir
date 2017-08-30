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

INCLUDEPATH += \
    dirview \
    fileoperation \
    filesystem \
    groupview \
    shellcommand \
    utility \
    widgets

OTHER_FILES += \
    $$PWD/../uncrustify.cfg \
    $$PWD/../icons/README.md \
    $$PWD/../LICENSE.md \
    $$PWD/../translations/LICENSE_ru.md \
    $$PWD/../README.md \
    $$PWD/../utils/* \
    $$PWD/../styles/*

SOURCES += \
    dirview/dirstatuswidget.cpp \
    dirview/dirview.cpp \
    dirview/dirwidget.cpp \
    dirview/dirwidgetfactory.cpp \
    dirview/navigationhistory.cpp \
    dirview/pathwidget.cpp \
    fileoperation/fileconflictresolver.cpp \
    fileoperation/fileoperation.cpp \
    fileoperation/fileoperationdelegate.cpp \
    fileoperation/fileoperationmodel.cpp \
    filesystem/backgroundreader.cpp \
    filesystem/filedelegate.cpp \
    filesystem/filepermissiondelegate.cpp \
    filesystem/filepermissions.cpp \
    filesystem/filesystemcompleter.cpp \
    filesystem/filesystemmodel.cpp \
    filesystem/proxymodel.cpp \
    groupview/groupsmenu.cpp \
    groupview/groupsview.cpp \
    groupview/groupwidget.cpp \
    groupview/tiledview.cpp \
    shellcommand/shellcommand.cpp \
    shellcommand/shellcommandmodel.cpp \
    shellcommand/shellcommandwidget.cpp \
    utility/copypaste.cpp \
    utility/debug.cpp \
    utility/globalaction.cpp \
    utility/notifier.cpp \
    utility/openwith.cpp \
    utility/settingsmanager.cpp \
    utility/shortcutmanager.cpp \
    utility/storagemanager.cpp \
    utility/styleloader.cpp \
    utility/styleoptionsproxy.cpp \
    utility/translationloader.cpp \
    utility/trash.cpp \
    utility/updatechecker.cpp \
    widgets/fileviewer.cpp \
    widgets/mainwindow.cpp \
    widgets/propertieswidget.cpp \
    widgets/settingseditor.cpp \
    widgets/transferdialog.cpp \
    main.cpp \
    utils.cpp

HEADERS  += \
    dirview/dirstatuswidget.h \
    dirview/dirview.h \
    dirview/dirwidget.h \
    dirview/dirwidgetfactory.h \
    dirview/navigationhistory.h \
    dirview/pathwidget.h \
    fileoperation/fileconflictresolver.h \
    fileoperation/fileoperation.h \
    fileoperation/fileoperationdelegate.h \
    fileoperation/fileoperationmodel.h \
    filesystem/backgroundreader.h \
    filesystem/filedelegate.h \
    filesystem/filepermissiondelegate.h \
    filesystem/filepermissions.h \
    filesystem/filesystemcompleter.h \
    filesystem/filesystemmodel.h \
    filesystem/proxymodel.h \
    groupview/groupsmenu.h \
    groupview/groupsview.h \
    groupview/groupwidget.h \
    groupview/tiledview.h \
    shellcommand/shellcommand.h \
    shellcommand/shellcommandmodel.h \
    shellcommand/shellcommandwidget.h \
    utility/copypaste.h \
    utility/debug.h \
    utility/globalaction.h \
    utility/notifier.h \
    utility/openwith.h \
    utility/settingsmanager.h \
    utility/shortcutmanager.h \
    utility/storagemanager.h \
    utility/styleloader.h \
    utility/styleoptionsproxy.h \
    utility/translationloader.h \
    utility/trash.h \
    utility/updatechecker.h \
    widgets/fileviewer.h \
    widgets/mainwindow.h \
    widgets/propertieswidget.h \
    widgets/settingseditor.h \
    widgets/transferdialog.h \
    backport.h \
    constants.h \
    utils.h

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
