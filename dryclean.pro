QT       += core gui widgets sql

CONFIG += c++17

# The name of the resulting executable
TARGET = dryclean
TEMPLATE = app

# Source files
SOURCES += \
    src/main.cpp \
    src/model/database.cpp \
    src/controller/authcontroller.cpp \
    src/controller/datacontroller.cpp \
    src/view/authwindow.cpp \
    src/view/mainwindow.cpp \
    src/view/dialogs.cpp

# Header files
HEADERS += \
    src/model/database.h \
    src/model/entities.h \
    src/controller/authcontroller.h \
    src/controller/datacontroller.h \
    src/view/authwindow.h \
    src/view/mainwindow.h \
    src/view/dialogs.h
