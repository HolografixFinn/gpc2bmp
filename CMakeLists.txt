# CMakeList.txt: file di progetto CMake di primo livello. Eseguire la configurazione globale
# e includere qui i sottoprogetti.
#
cmake_minimum_required (VERSION 3.8)

project (gpc2bmp VERSION 0.1 )

# Includere i sottoprogetti.
add_subdirectory ("src")
