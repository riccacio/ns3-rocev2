set(BLIB_LIBNAME rocev2)

build_lib_example(
  TARGET NAME rocev2-skeleton
  SOURCE_FILES rocev2-skeleton.cc
   LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpoint-to-point} 
)