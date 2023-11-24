build_lib_example(
    NAME iotnet-example
    SOURCE_FILES iotnet-example.cc
    LIBRARIES_TO_LINK ${libiotnet} ${netanim}
)

