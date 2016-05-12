TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    header.cpp \
    requestresponder.cpp

HEADERS += \
    header.h \
    requestresponder.h \
    enums.h

LIBS += -lpthread
