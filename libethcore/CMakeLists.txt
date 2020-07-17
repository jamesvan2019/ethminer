set(SOURCES
	EthashAux.h EthashAux.cpp
	ProgPowAux.h ProgPowAux.cpp BlockHeader.h BlockHeader.cpp Exceptions.h
	Farm.cpp Farm.h
	Miner.h Miner.cpp
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(BEFORE ..)

add_library(ethcore ${SOURCES})
target_link_libraries(ethcore PUBLIC devcore ethash PRIVATE hwmon ethash-legacy)

if(ETHASHCL)
	target_link_libraries(ethcore PRIVATE ethash-cl)
endif()
if(METAL)
	target_link_libraries(ethcore PRIVATE ethash-metal)
endif()
if(ETHASHCUDA)
	target_link_libraries(ethcore PUBLIC ethash-cuda)
endif()
if(ETHASHCPU)
	target_link_libraries(ethcore PUBLIC ethash-cpu)
endif()
