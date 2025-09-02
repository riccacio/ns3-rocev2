set(BLIB_LIBNAME rocev2)

build_lib_example(
  TARGET NAME main
  SOURCE_FILES main.cc
   LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${libinternet}
    ${libpoint-to-point} 
)