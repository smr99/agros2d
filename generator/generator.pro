# agros2d - hp-FEM multiphysics application based on Hermes2D library
TARGET = ../agros2d_generator
OBJECTS_DIR = build
MOC_DIR = build
TEMPLATE = app

INCLUDEPATH += ../hermes2d/include
INCLUDEPATH += ../hermes_common/include

LIBS += -lagros2d_hermes2d
LIBS += -lagros2d_3dparty
LIBS += -lagros2d

HEADERS += generator.h \
    templates/weakform_h

SOURCES += main.cpp \
           generator.cpp

include(../agros2d.pri)

OTHER_FILES += \
    templates/filter_cpp.tpl \
    templates/filter_h.tpl \
    templates/localvalue_cpp.tpl \
    templates/localvalue_h.tpl \
    templates/surfaceintegral_cpp.tpl \
    templates/surfaceintegral_h.tpl \
    templates/volumeintegral_cpp.tpl \
    templates/volumeintegral_h.tpl \
    templates/interface_cpp.tpl \
    templates/interface_h.tpl \
    templates/module_pro.tpl \
    templates/weakform_cpp.tpl \
    templates/weakform_h.tpl \
    templates/weakform_pri.tpl \
    templates/module_pro.tpl \
    templates/plugins_pro.tpl \
    templates/coupling_weakform_h.tpl \
    templates/coupling_weakform_cpp.tpl \
    templates/coupling_interface_h.tpl \
    templates/coupling_interface_cpp.tpl