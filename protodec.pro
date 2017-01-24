TEMPLATE = app
CONFIG  += console c++11
CONFIG  -= app_bundle
CONFIG  -= qt
win32 {
RC_FILE += winres.rc
}
SOURCES += protodec.cpp
#SOURCES += tests.cpp
#LIBS += -lgtest -lpthread
