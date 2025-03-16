add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(math)
add_subdirectory(physics)
add_subdirectory(scene)
add_subdirectory(scripting)
add_subdirectory(ui)
add_subdirectory(utils)

target_sources(aegix-engine PRIVATE
	engine.cpp
	engine.h
)