TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    requestresponder.cpp \
    httpheader.cpp

HEADERS += \
    requestresponder.h \
    rwnetwork.h \
    httpheader.h

LIBS += -lpthread
