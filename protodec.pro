TEMPLATE = app
CONFIG  += console c++11
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
