PROJECT(${MATIO_LIBRARY})

add_custom_target(MATIO_CONFIG ALL DEPENDS matioConfig.h)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
        SOURCE    matioConfigWin.h
        COMMAND   copy ARGS ${CMAKE_CURRENT_SOURCE_DIR}/matioConfigWin.h matioConfig.h OUTPUT matioConfig.h
        TARGET    MATIO_CONFIG
        OUTPUTS   matioConfig.h)
ELSE()
  ADD_CUSTOM_COMMAND(
        SOURCE    matioConfigLinux.h
        COMMAND   cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/matioConfigLinux.h matioConfig.h OUTPUT matioConfig.h
        TARGET    MATIO_CONFIG
        OUTPUTS   matioConfig.h)
ENDIF()
ADD_CUSTOM_COMMAND(
      SOURCE    MATIO_CONFIG
      TARGET    MATIO_CONFIG
      DEPENDS   matioConfig.h)

SET(SOURCES endian.c
    inflate.c
    io.c
    mat4.c
    mat5.c
    mat73.c
    mat.c
    matvar_cell.c
    matvar_struct.c
    read_data.c
    snprintf.c
    matioConfig.h
)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
