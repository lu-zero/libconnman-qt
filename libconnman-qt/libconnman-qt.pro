TEMPLATE     = lib
VERSION      = 0.3.0
CONFIG      += qt debug
CONFIG      += create_pc create_prl
QT          += core dbus
QT          -= gui
TARGET       = $$qtLibraryTarget(connman-qt4)

OBJECTS_DIR += build
MOC_DIR     += build

isEmpty(PREFIX) {
  PREFIX=/usr
}

#system(qdbusxml2cpp -c Manager -p manager -N connman-manager.xml)
system(qdbusxml2cpp -c Service -p service -N connman-service.xml)
system(qdbusxml2cpp -c Technology -p technology -N connman-technology.xml)

HEADERS += manager.h \
    service.h \
    technology.h \
    networkmanager.h \
    networktechnology.h \
    networkservice.h \
    commondbustypes.h \
    clockproxy.h \
    clockmodel.h \
    debug.h

SOURCES += \
    networkmanager.cpp \
    networktechnology.cpp \
    networkservice.cpp \
    manager.cpp \
    service.cpp \
    technology.cpp \
    clockproxy.cpp \
    clockmodel.cpp \
    commondbustypes.cpp \
    debug.cpp

target.path = $$INSTALL_ROOT$$PREFIX/lib

headers.files = $$HEADERS
headers.path = $$INSTALL_ROOT$$PREFIX/include/connman-qt

QMAKE_PKGCONFIG_DESCRIPTION = Qt Connman Library
QMAKE_PKGCONFIG_INCDIR = $$headers.path
pkgconfig.path = $$INSTALL_ROOT$$PREFIX/lib/pkgconfig
pkgconfig.files = connman-qt4.pc

INSTALLS += target headers pkgconfig
