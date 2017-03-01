PROJECT(${AGROS_SOLVER_EXTERNAL})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/matio)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/paralution/src)

include_directories(${PROJECT_BINARY_DIR}/../hermes/hermes2d/include/function)
include_directories(${PROJECT_BINARY_DIR}/../hermes/hermes2d/include/mesh)
include_directories(${PROJECT_BINARY_DIR}/../hermes/hermes2d/include/space)

SET(SOURCES main.cpp)

#SET(HEADERS agros_solver.h)

message(${HERMES_COMMON_LIBRARY})

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_LIBRARY} ${HERMES_COMMON_LIBRARY} ${MATIO_LIBRARY})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

