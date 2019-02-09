set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(atbridge aarp.c atbridge.c elap.c llap.c pcap_delay.c port.c)

target_compile_definitions(atbridge PUBLIC HAVE_ATBRIDGE)
