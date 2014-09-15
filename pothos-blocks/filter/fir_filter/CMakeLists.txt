cmake_minimum_required(VERSION 2.8.3)
project(fir_filter)

find_package(catkin REQUIRED COMPONENTS roscpp)

catkin_package(
  DEPENDS 
  CATKIN_DEPENDS
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME} 
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

link_directories(${catkin_LIBRARY_DIRS})

add_library(${PROJECT_NAME} src/filt.cpp)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})


install(TARGETS ${PROJECT_NAME} 
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
