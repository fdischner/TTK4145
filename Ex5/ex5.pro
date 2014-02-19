#-------------------------------------------------
#
# Project created by QtCreator 2014-02-11T18:45:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ex5
TEMPLATE = app
QMAKE_CFLAGS += -std=c99
LIBS += -lcomedi -lm

SOURCES += main.cpp\
    io.c \
    elev.c \
    elevator.cpp \
    control.cpp

HEADERS  += channels.h \
    io.h \
    elev.h \
    elevator.h \
    control.h

FORMS    +=
