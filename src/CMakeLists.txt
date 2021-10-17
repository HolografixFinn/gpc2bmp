# CMakeList.txt: progetto CMake per gpc2bmp. Includere l'origine e definire
# qui la logica specifica del progetto.
#
cmake_minimum_required (VERSION 3.8)

SET(SOURCE
  gpc2bmp.cpp
  GPC.cpp
  utils/DataHelper.cpp
  utils/BmpHelper.cpp
)

SET(HEADERS
  gpc2bmp.h
  inc/GPC.h
  utils/inc/DataHelper.h
  utils/inc/BmpHelper.h
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (gpc2bmp ${SOURCE} ${HEADERS})
# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
