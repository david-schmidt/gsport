set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(tfe tfe.c tfearch.c tfesupp.c)

target_compile_definitions(tfe PUBLIC HAVE_TFE)

