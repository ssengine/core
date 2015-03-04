CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(SSCore)

ADD_DEFINITIONS(-DBUILDING_SS_CORE)

SET(LOCAL_SOURCES_FILES
	src/script.cpp
	src/core.cpp
	src/core.h
	src/log.cpp
	src/macros.cpp
	src/modules.h

	src/math/matrix.cpp

	src/uri/uri.cpp
	src/uri/schemas.cpp
	src/uri/file_schema.cpp
	src/uri/schemas.h
	
	src/modules/mod_log.c
	src/modules/mod_uri_loader.cpp
	src/modules/mod_render2d.cpp
	
	src/render/draw_batch.h
	src/render/draw_batch.cpp
	src/render/render_resources.cpp

	src/resource/resource_load.cpp
	src/resource/resource_manager.cpp
	
	src/decoders/decoder.h
	src/decoders/decoder_win32.cpp
	src/decoders/decoder_android.cpp
	src/decoders/decoder_ios.cpp
	)

LINK_LIBRARIES(${SS_LUA_LIBNAME})
LINK_DIRECTORIES(${LUA_LIB})

ADD_LIBRARY(SSCore
	SHARED
	${LOCAL_SOURCES_FILES}
)

IF(SS_HAVE_OUTPUT_DIRECTORY)
	SET_TARGET_PROPERTIES(
		SSCore
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY $ENV{SS_BINARY_PATH}
		LIBRARY_OUTPUT_DIRECTORY $ENV{SS_BINARY_PATH}
		ARCHIVE_OUTPUT_DIRECTORY $ENV{SS_ARCHIVE_PATH}
		PDB_OUTPUT_DIRECTORY $ENV{SS_PDB_PATH}
		)
ENDIF(SS_HAVE_OUTPUT_DIRECTORY)

SET_PROPERTY(TARGET SSCore PROPERTY FOLDER "Core")
