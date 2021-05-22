include(../../library_dirname.pri)

isEmpty(QBSLIBDIR) {
    QBSLIBDIR = $${OUT_PWD}/../../../$${QBS_LIBRARY_DIRNAME}
}

QBSPKGCONFIG_LIBNAME=qbspkgconfig

unix {
    LIBS += -L$${QBSLIBDIR} -l$${QBSPKGCONFIG_LIBNAME}
}

win32 {
    CONFIG(debug, debug|release) {
        QBSPKGCONFIG_LIB = $${QBSPKGCONFIG_LIBNAME}d
    }
    CONFIG(release, debug|release) {
        QBSPKGCONFIG_LIB = $${QBSPKGCONFIG_LIBNAME}
    }
    msvc {
        LIBS += /LIBPATH:$$QBSLIBDIR
        QBSPKGCONFIG_LIB = $${QBSPKGCONFIG_LIB}.lib
        LIBS += Shell32.lib
    } else {
        LIBS += -L$${QBSLIBDIR}
        QBSPKGCONFIG_LIB = lib$${QBSPKGCONFIG_LIB}
    }
    LIBS += $${QBSPKGCONFIG_LIB}
}

INCLUDEPATH += \
    $$PWD

CONFIG += depend_includepath

CONFIG(static, static|shared) {
    DEFINES += QBS_STATIC_LIB
}

DEFINES += \
    QBS_PC_WITH_QT_SUPPORT=1
