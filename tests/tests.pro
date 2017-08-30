TARGET = tests
TEMPLATE = app

QT += widgets

CONFIG += c++11

OTHER_FILES += \
    $$PWD/../uncrustify.cfg

INCLUDEPATH += \
    $$PWD/../src \
    $$PWD/../src/dirview \
    $$PWD/../src/fileoperation \
    $$PWD/../src/filesystem \
    $$PWD/../src/groupview \
    $$PWD/../src/shellcommand \
    $$PWD/../src/utility \
    $$PWD/../src/widgets

VPATH += $$PWD/../src

SOURCES += \
    filesystem/filepermissions.cpp \
    shellcommand/shellcommand.cpp \
    utility/notifier.cpp \
    utility/debug.cpp \
    main.cpp \
    filepermissions_test.cpp \
    shellcommand_test.cpp

HEADERS  += \
    catch.hpp \
    catch_ext.h
