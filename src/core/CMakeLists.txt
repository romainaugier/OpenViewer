# CORE #

file(GLOB_RECURSE src_files *.cpp)
file(GLOB_RECURSE h_files *.h)

add_library(core ${src_files} ${h_files})

target_link_libraries(core OpenImageIO::OpenImageIO)
target_link_libraries(core wuffs utils)
target_link_libraries(core IlmBase::Iex IlmBase::Half IlmBase::Imath IlmBase::IexMath)
target_link_libraries(core OpenEXR::IlmImf OpenEXR::IlmImfUtil OpenEXR::IlmImfConfig)