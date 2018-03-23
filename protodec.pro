TEMPLATE = app
CONFIG += console
CONFIG += c++11
#DEFINES += DEBUG
#CONFIG  += unittest
CONFIG  -= app_bundle
CONFIG  -= qt
win32 {
RC_FILE += winres.rc
}
CONFIG(unittest) {
  SOURCES += tests.cpp
  LIBS += -lgtest -lpthread
} else {
  SOURCES += protodec.cpp
}
