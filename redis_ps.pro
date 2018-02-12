TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


DESTDIR = ./bin

OBJECTS_DIR = ./tmp/obj
UI_DIR = ./tmp/ui
MOC_DIR = ./tmp/moc


include (src/src.pri)

# libs
# hiredis
INCLUDEPATH += $$PWD/libs/hiredis/include
unix:!macx: LIBS += -L$$PWD/libs/hiredis/lib/ -lhiredis
unix:!macx: PRE_TARGETDEPS += $$PWD/libs/hiredis/lib/libhiredis.a

LIBS += -pthread

INCLUDEPATH += $$PWD/libs/libevent/include
unix:!macx: LIBS += -L$$PWD/libs/libevent/lib/ -levent
unix:!macx: PRE_TARGETDEPS += $$PWD/libs/libevent/lib/libevent.a
