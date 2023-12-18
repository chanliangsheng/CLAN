QT       += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Parameter.cpp \
    cardiolipin/src/CL.cpp \
    cardiolipin/src/ClSpecificStructure.cpp \
    cardiolipin/src/DLCL.cpp \
    cardiolipin/src/DlclSpecificStructure.cpp \
    cardiolipin/src/MLCL.cpp \
    cardiolipin/src/MlclSpecificStructure.cpp \
    cardiolipin/src/SpecificStructure.cpp \
    cardiolipin/src/cardiolipin.cpp \
    cardiolipin/src/fa.cpp \
    cardiolipin/src/pa.cpp \
    database/src/database.cpp \
    database/src/databaserecord.cpp \
    main.cpp \
    mainwindow.cpp \
    mzml/src/base64.cpp \
    mzml/src/headgroup.cpp \
    mzml/src/ms1.cpp \
    mzml/src/ms2.cpp \
    mzml/src/mzml.cpp \
    workflow/src/FragmentCombiner.cpp \
    workflow/src/FragmentFinder.cpp \
    workflow/src/HeadgroupFinder.cpp \
    workflow/src/MS1LibraryMatcher.cpp \
    workflow/src/MSLevelMatcher.cpp \
    workflow/src/RtFilter.cpp \
    workflow/src/workflow.cpp


HEADERS += \
    Parameter.h \
    cardiolipin/include/CL.h \
    cardiolipin/include/ClSpecificStructure.h \
    cardiolipin/include/DLCL.h \
    cardiolipin/include/DlclSpecificStructure.h \
    cardiolipin/include/MLCL.h \
    cardiolipin/include/MlclSpecificStructure.h \
    cardiolipin/include/SpecificStructure.h \
    cardiolipin/include/cardiolipin.h \
    cardiolipin/include/fa.h \
    cardiolipin/include/pa.h \
    database/include/database.h \
    database/include/databaserecord.h \
    mainwindow.h \
    mzml/include/base64.h \
    mzml/include/headgroup.h \
    mzml/include/ms1.h \
    mzml/include/ms2.h \
    mzml/include/mzml.h \
    workflow/include/FragmentCombiner.h \
    workflow/include/FragmentFinder.h \
    workflow/include/HeadgroupFinder.h \
    workflow/include/MS1LibraryMatcher.h \
    workflow/include/MSLevelMatcher.h \
    workflow/include/RtFilter.h \
    workflow/include/workflow.h


FORMS += \
    Parameter.ui \
    mainwindow.ui

INCLUDEPATH += $$PWD/mzml/include
INCLUDEPATH += $$PWD/database/include
INCLUDEPATH += $$PWD/cardiolipin/include
INCLUDEPATH += $$PWD/workflow/include

# 导入第三方库的头文件和动态库地址
INCLUDEPATH += 3rdparty/include
LIBS += -L $$PWD/3rdparty/lib -l tinyxml2
LIBS += -lz

# 把库文件加入到运行时的文件夹中
MY_PWD = $$PWD
OUTPUT_PWD = $$OUT_PWD
# 将变量MY_PWD中的正斜杠替换为反斜杠
MY_PWD_WIN = $$replace(MY_PWD, /, \\)
OUTPUT_PWD_WIN = $$replace(OUTPUT_PWD, /, \\)

QMAKE_PRE_LINK = xcopy $$MY_PWD_WIN\3rdparty\lib\tinyxml2.dll $$OUTPUT_PWD_WIN\debug /y

# 复制db文件夹到编译后项目中
MY_DB_PWD = $$MY_PWD_WIN\database.db
OUT_DB_PWD = $$OUTPUT_PWD_WIN\

copy_db = $$system("echo d| xcopy $${MY_DB_PWD} $${OUT_DB_PWD}")

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
