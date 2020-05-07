cmake_minimum_required(VERSION 3.16)
project(eLibrary)

add_definitions("-Wall")

include_directories(./Source)
include_directories(./Source/libgraphics)
include_directories(./Source/sqlite3)
include_directories(./Source/model)
include_directories(./Source/template)
include_directories(./Source/view)

aux_source_directory(./Source DIR_SRC)
aux_source_directory(./Source/libgraphics DIR_LIBGRAPHICS)
aux_source_directory(./Source/model DIR_MODEL)
aux_source_directory(./Source/template DIR_TEMPLATE)
aux_source_directory(./Source/view DIR_VIEW)
aux_source_directory(./Source/sqlite3 DIR_SQLITE3)

add_executable(main ${DIR_SRC} ${DIR_MODEL} ${DIR_TEMPLATE} ${DIR_VIEW} ${DIR_LIBGRAPHICS} ${DIR_SQLITE3} ${LIB})