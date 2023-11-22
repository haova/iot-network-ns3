include_directories(./include) # CXXFLAGS -I counterpart
link_directories(./lib) # LINKFLAGS -L counterpart
link_libraries(cpr) # LINKFLAGS -l or LIB counterpart

# Target definition after changing settings
build_exec(
  EXECNAME main
  SOURCE_FILES main.cc
               server-app.cc
  HEADER_FILES server-app.h
  LIBRARIES_TO_LINK ${ns3-libs}
                    curl
                    cpr
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_OUTPUT_DIRECTORY}/scratch/ns3-kit
)
