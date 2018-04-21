TEMPLATE = app

TARGET = CQVFontEd

DEPENDPATH += .

QT += widgets opengl

QMAKE_CXXFLAGS += -std=c++14

SOURCES += \
CQVFontEd.cpp \

HEADERS += \
CQVFontEd.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../../CQUtil/include \
../../CVFont/include \
../../CImageLib/include \
../../CFont/include \
../../CFile/include \
../../CMath/include \
../../CStrUtil/include \
../../CUtil/include \
../../COS/include \
.

unix:LIBS += \
-L$$LIB_DIR \
-L../../CQUtil/lib \
-L../../CVFont/lib \
-L../../CConfig/lib \
-L../../CImageLib/lib \
-L../../CFont/lib \
-L../../CFile/lib \
-L../../CFileUtil/lib \
-L../../CMath/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-L../../CUtil/lib \
-L../../COS/lib \
-lCQUtil \
-lCVFont \
-lCConfig \
-lCImageLib \
-lCFont \
-lCFile \
-lCFileUtil \
-lCMath \
-lCStrUtil \
-lCRegExp \
-lCUtil \
-lCOS \
-ltre -lpng -ljpeg
