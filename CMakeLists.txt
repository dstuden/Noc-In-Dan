cmake_minimum_required(VERSION 3.28)
project(NocInDan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Engine (submodule). -DTOMOS_BUILD_EDITOR=OFF skips the scene editor library.
add_subdirectory(external/Tomos ${CMAKE_BINARY_DIR}/_tomos)

add_executable(NocInDan
	src/main.cc
	src/App.cc
	src/App.hh
	src/utils/Item.cc
	src/utils/Item.hh
	src/utils/Inventory.cc
	src/utils/Inventory.hh
	src/utils/Stat.hh
	src/utils/GameCalendar.hh
	src/utils/Survival.hh
	src/utils/WorldClock.hh
	src/scripts/PlayerScript.cc
	src/scripts/PlayerScript.hh
	src/scripts/MobScript.cc
	src/scripts/MobScript.hh
	src/scripts/WorldScript.cc
	src/scripts/WorldScript.hh
	src/layers/GameLayer.cc
	src/layers/GameLayer.hh
	src/layers/UiLayer.cc
	src/layers/UiLayer.hh
	src/intro/Intro.cc
	src/intro/Intro.hh
	src/scenes/InGameScene.cc
	src/scenes/InGameScene.hh
	src/components/TiltedCameraComponent.cc
	src/components/TiltedCameraComponent.hh
	src/scripts/SunOrbitScript.cc
	src/scripts/SunOrbitScript.hh
	src/RegisterGameTypes.cc
	src/RegisterGameTypes.hh
	src/utils/Biome.hh
	src/worldgen/WorldGenConfig.cc
	src/worldgen/WorldGenConfig.hh
	src/worldgen/SpatialUnits.hh
	src/worldgen/SpawnTable.hh
	src/worldgen/WorldGenerator.cc
	src/worldgen/WorldGenerator.hh
	src/worldgen/IslandMask.cc
	src/worldgen/IslandMask.hh
	src/worldgen/WorldGroundMesh.cc
	src/worldgen/WorldGroundMesh.hh
	src/worldgen/GroundTileDef.cc
	src/worldgen/GroundTileDef.hh
	src/worldgen/GroundTileId.hh
	src/worldgen/Noise.hh
	src/worldgen/CellRects.hh
	src/worldgen/CoastWalls.cc
	src/worldgen/CoastWalls.hh
	src/worldgen/WorldMap.hh
	src/worldgen/WorldGeometry.cc
	src/worldgen/WorldGeometry.hh
	src/movement/SpeedModifierList.hh
	src/movement/LocomotorController.cc
	src/movement/LocomotorController.hh
	src/prefabs/PrefabDef.hh
	src/prefabs/PrefabCatalog.cc
	src/prefabs/PrefabCatalog.hh
	src/prefabs/PrefabBuilder.cc
	src/prefabs/PrefabBuilder.hh
)

target_link_libraries(NocInDan PRIVATE Tomos)

if(TARGET TomosEditor)
	target_link_libraries(NocInDan PRIVATE TomosEditor)
endif()

target_link_libraries(NocInDan PRIVATE tomos_build_flags)
tomos_enable_ipo(NocInDan)

# Stage assets, shaders and engine resources next to the binary.
tomos_deploy_runtime(NocInDan
	ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
