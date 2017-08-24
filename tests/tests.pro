TARGET = tests
TEMPLATE = app

QT += widgets

CONFIG += c++11

OTHER_FILES += \
    $$PWD/../uncrustify.cfg

INCLUDEPATH += $$PWD/../src
VPATH += $$PWD/..

SOURCES += \
    $$PWD/../src/filepermissions.cpp \
    $$PWD/../src/shellcommand.cpp \
    $$PWD/../src/notifier.cpp \
    $$PWD/../src/debug.cpp \
    main.cpp \
    filepermissions_test.cpp \
    shellcommand_test.cpp

HEADERS  += \
    catch.hpp \
    catch_ext.h
