TARGET = tests
TEMPLATE = app

CONFIG += c++11

OTHER_FILES += \
    ../uncrustify.cfg

INCLUDEPATH += $$PWD/../src
VPATH += $$PWD/..

SOURCES += \
    main.cpp \
    src/filepermissions.cpp \
    filepermissions_test.cpp

HEADERS  += \
    catch.hpp \
    src/filepermissions.h
