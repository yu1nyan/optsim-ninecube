#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(wls)

# Set C++ compiler version
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  if( CMAKE_CXX_COMPILER_VERION VERSION_GREATER 3.5 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.5 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z" )
  elseif( CMAKE_CXX_COMPILER_VERION VERSION_GREATER 3.2 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.2 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )
  else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
  endif()

elseif( CMAKE_COMPILER_IS_GNUCXX )
  if( CMAKE_CXX_COMPILER_VERION VERSION_GREATER 5.1 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 5.1 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
  elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.8)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )
  elseif( CMAKE_CXX_COMPILER_VERION VERSION_GREATER 4.7 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.7 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
  elseif( CMAKE_CXX_COMPILER_VERION VERSION_GREATER 4.3 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.3 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
  else()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03" )
  endif()

endif()

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${Geant4_INCLUDE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(wls wls.cc ${sources} ${headers})
target_link_libraries(wls ${Geant4_LIBRARIES} )

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build wls. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(wls_SCRIPTS
    wls.out
    wls.in
    init.in
#    gui.mac
#    icons.mac
#    run.png
#    Sr90.mac
    vis.mac
    compile.sh
    run.sh
	run.mac
    runtest.sh
    plot.C
    runsurface.sh
    runsurface.mac
   )

foreach(_script ${wls_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS wls DESTINATION bin)
