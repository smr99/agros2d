PROJECT(${AGROS_BINARY})
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/pythonlab-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bson)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)

IF(WIN32)
  SET(GUI WIN32)
  ADD_DEFINITIONS(-D_AFXDLL)
  SET(CMAKE_MFC_FLAG 2)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:\"mainCRTStartup\" ")
ENDIF(WIN32)

ADD_EXECUTABLE(${PROJECT_NAME} ${GUI} main.cpp ../resources_source/resources.rc)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_QTMAIN_LIBRARY} ${AGROS_LIBRARY} ${PYTHONLAB_LIBRARY})
IF(WITH_QT5)
    QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets UiTools OpenGL)
ENDIF(WITH_QT5)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME agros2d)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
