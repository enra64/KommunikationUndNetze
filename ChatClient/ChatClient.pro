#-------------------------------------------------
#
# Project created by QtCreator 2016-05-04T15:24:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChatClient
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        chatwindow.cpp \
    basenetwork.cpp \
    lownetwork.cpp \
    highnetwork.cpp \
    message.cpp

HEADERS  += chatwindow.h \
    cnaiapi.h \
    basenetwork.h \
    lownetwork.h \
    highnetwork.h \
    compilerdistractor.h \
    message.h

LIBS += -L"$$_PRO_FILE_PWD_/" -lcnaiapi64

#LIBS += -static -cnaiapi64
#PRE_TARGETDEPS += lcnaiapi64

LIBS  += -lpthread

FORMS    += chatwindow.ui
