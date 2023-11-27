check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/iotnet-examples-test-suite.cc
        )
endif()

find_package(cpr REQUIRED)
link_libraries(cpr::cpr)

build_lib(
    LIBNAME iotnet
    SOURCE_FILES model/iotnet.cc
                 model/iotnet-sensor.cc
                 model/iotnet-server.cc
                 helper/iotnet-helper.cc
    HEADER_FILES model/iotnet.h
                 model/iotnet-sensor.h
                 model/iotnet-server.h
                 helper/iotnet-helper.h
    LIBRARIES_TO_LINK ${libcore}
                      cpr
    TEST_SOURCES test/iotnet-test-suite.cc
                 ${examples_as_tests_sources}
)

