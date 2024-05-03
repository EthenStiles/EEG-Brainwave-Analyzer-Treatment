QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    battery.cpp \
    datetimemanager.cpp \
    electrode.cpp \
    graphwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    reversestopwatch.cpp \
    wave.cpp

HEADERS += \
    battery.h \
    datetimemanager.h \
    electrode.h \
    graphwindow.h \
    mainwindow.h \
    reversestopwatch.h \
    wave.h

FORMS += \
    graphwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
