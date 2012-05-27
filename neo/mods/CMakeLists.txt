# Sikkmod 1.2

string(REPLACE "game/" "sikkmod/" src_sikkmod "${src_game}")
set(src_sikkmod
	${src_sikkmod}
	sikkmod/Grab.cpp
	sikkmod/physics/Force_Grab.cpp
)

add_library(sikkmod SHARED ${src_sikkmod})
set_target_properties(sikkmod PROPERTIES PREFIX "")
set_target_properties(sikkmod PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
set_target_properties(sikkmod PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/sikkmod -I${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(sikkmod PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(sikkmod PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(sikkmod PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(sikkmod PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../sikkmod")
target_link_libraries(sikkmod idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS sikkmod LIBRARY DESTINATION share/dhewm3/sikkmod)
endif()

# Sikkmod 1.1

string(REPLACE "game/" "sikkmod_v11/" src_sikkmod_v11 "${src_game}")
set(src_sikkmod_v11
	${src_sikkmod_v11}
	sikkmod_v11/Grab.cpp
	sikkmod_v11/physics/Force_Grab.cpp
)

add_library(sikkmod_v11 SHARED ${src_sikkmod_v11})
set_target_properties(sikkmod_v11 PROPERTIES PREFIX "")
set_target_properties(sikkmod_v11 PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
set_target_properties(sikkmod_v11 PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/sikkmod_v11 -I${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(sikkmod_v11 PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(sikkmod_v11 PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(sikkmod_v11 PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(sikkmod_v11 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../sikkmod_v11")
target_link_libraries(sikkmod_v11 idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS sikkmod_v11 LIBRARY DESTINATION share/dhewm3/sikkmod_v11)
endif()

# Sikkmod 1.1 for RoE

string(REPLACE "d3xp/" "sikkmodd3xp/" src_sikkmodd3xp "${src_d3xp}")
set(src_sikkmodd3xp
	${src_sikkmodd3xp}
	sikkmodd3xp/Grab.cpp
)

add_library(sikkmodd3xp SHARED ${src_sikkmodd3xp})
set_target_properties(sikkmodd3xp PROPERTIES PREFIX "")
set_target_properties(sikkmodd3xp PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
set_target_properties(sikkmodd3xp PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/sikkmodd3xp -I${CMAKE_CURRENT_SOURCE_DIR} -Wno-overloaded-virtual")
set_target_properties(sikkmodd3xp PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(sikkmodd3xp PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(sikkmodd3xp PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(sikkmodd3xp PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../sikkmodd3xp")
target_link_libraries(sikkmodd3xp idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS sikkmodd3xp LIBRARY DESTINATION share/dhewm3/sikkmodd3xp)
endif()

# Project Synergy

string(REPLACE "game/" "synergy/" src_synergy "${src_game}")

add_library(synergy SHARED ${src_synergy})
set_target_properties(synergy PROPERTIES PREFIX "")
set_target_properties(synergy PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
set_target_properties(synergy PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/synergy -I${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(synergy PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(synergy PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(synergy PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(synergy PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../synergy")
target_link_libraries(synergy idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS synergy LIBRARY DESTINATION share/dhewm3/synergy)
endif()

# Classic Doom

string(REPLACE "game/" "classicdoom/" src_classicdoom "${src_game}")

add_library(classicdoom SHARED ${src_classicdoom})
set_target_properties(classicdoom PROPERTIES PREFIX "")
set_target_properties(classicdoom PROPERTIES COMPILE_DEFINITIONS "GAME_DLL")
set_target_properties(classicdoom PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/classicdoom -I${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(classicdoom PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(classicdoom PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(classicdoom PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(classicdoom PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../classicdoom")
target_link_libraries(classicdoom idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS classicdoom LIBRARY DESTINATION share/dhewm3/classicdoom)
endif()

# Ruiner

string(REPLACE "game/" "ruiner/" src_ruiner "${src_game}")
set(src_ruiner
	${src_ruiner}
	ruiner/ai/AI_bot.cpp
	ruiner/Grabber.cpp
	ruiner/physics/Force_Grab.cpp
	ruiner/PlayerCursor.cpp
	ruiner/sourcehook/sourcehook.cpp
	ruiner/tracer.cpp
	ruiner/TrailGenerator.cpp
)

add_library(ruiner SHARED ${src_ruiner})
set_target_properties(ruiner PROPERTIES PREFIX "")
 
set_target_properties(ruiner PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_PORTALSKY;TRAIL_FX_CHAIN;TEST_TRAIL;_DENTONMOD")
set_target_properties(ruiner PROPERTIES COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/ruiner -I${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(ruiner PROPERTIES LINK_FLAGS "${ldflags}")
set_target_properties(ruiner PROPERTIES INSTALL_NAME_DIR "@executable_path")
set_target_properties(ruiner PROPERTIES OUTPUT_NAME "game${gamesuffix}")
set_target_properties(ruiner PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../ruiner")
target_link_libraries(ruiner idlib ${ZLIB_LIBRARY})
if (UNIX)
	install(TARGETS ruiner LIBRARY DESTINATION share/dhewm3/ruiner)
endif()
