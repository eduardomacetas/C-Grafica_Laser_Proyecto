QT += core
QT -= gui

CONFIG += c++11

TARGET = laser3d
CONFIG += console
CONFIG -= app_bundle


CONFIG += link_pkgconfig
PKGCONFIG += opencv

CONFIG += link_pkgconfig
PKGCONFIG += opencv

TEMPLATE = app

SOURCES += main.cpp \
    recognition.cpp \
    object.cpp

HEADERS += \
    recognition.h \
    object.h
