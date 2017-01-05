QT += core
QT -= gui

CONFIG += c++11

TARGET = mm
CONFIG += console
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += opencv

TEMPLATE = app

SOURCES += main.cpp \
    Object.cpp

DISTFILES += \
    google25facdaf6ed3521c.html \
    README.md

HEADERS += \
    Object.h
