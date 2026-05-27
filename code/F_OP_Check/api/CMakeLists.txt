set(MODULE_NAME F_OPCheck_API)
add_library(${MODULE_NAME} INTERFACE)

target_include_directories(${MODULE_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
# Vertrag des F_OP_Check-Moduls: IFileWriter, IWriteCheck.
# Wer diese Interfaces braucht, linkt gegen fileio_api – nicht gegen die Implementierung.
