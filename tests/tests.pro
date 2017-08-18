TARGET = tests
TEMPLATE = app

QT += widgets

CONFIG += c++11

OTHER_FILES += \
    ../uncrustify.cfg

INCLUDEPATH += $$PWD/../src
VPATH += $$PWD/..

SOURCES += \
    main.cpp \
    src/filepermissions.cpp \
    src/shellcommand.cpp \
    src/notifier.cpp \
    src/debug.cpp \
    filepermissions_test.cpp \
    shellcommand_test.cpp

HEADERS  += \
    catch.hpp
