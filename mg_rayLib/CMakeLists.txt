
cmake_minimum_required(VERSION 3.6.0)

project(mg_rayLib)   



    #looking for  files
    file(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.h")

    message (STATUS "found mg_rayLib files")
    foreach(dir ${SOURCE_FILES})
          message(STATUS "dir='${dir}'")
    endforeach()
    #looking for the header files
    #file(GLOB_RECURSE INCLUDES_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

    #message (STATUS "found includes files")
    #foreach(dir ${INCLUDES_FILES})
    #     message(STATUS "dir='${dir}'")
    #endforeach()

    #setupping llvm 
    #include_directories(${LLVM_INCLUDE_DIRS})
    #add_definitions(${LLVM_DEFINITIONS})

    include_directories(${CMAKE_SOURCE_DIR} )
    #defining standard compiling flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		add_definitions(-D_UNICODE -DUNICODE)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324 /wd4146 /wd4458 /wd4267 /wd4100 /wd4244 /wd4141 /wd4291 /wd4624 ")
		#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
		#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	endif()

	MACRO(SOURCE_GROUP_BY_FOLDER target)
  SET(SOURCE_GROUP_DELIMITER "/")
  SET(last_dir "")
  SET(files "")
  FOREACH(file ${SOURCE_FILES})
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}/{target}" ${file})
    GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
    IF (NOT "${dir}" STREQUAL "${last_dir}")
      IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
      ENDIF (files)
      SET(files "")
    ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
    SET(files ${files} ${file})
    SET(last_dir "${dir}")
  ENDFOREACH(file)
  IF (files)
    SOURCE_GROUP("${last_dir}" FILES ${files})
  ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)

SOURCE_GROUP_BY_FOLDER(mg_rayLib)
    #adding the executable
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDES_FILES})


    # Link against LLVM libraries
    #target_link_libraries(${PROJECT_NAME} ${llvm_libs})

    #enabling clang tidy
    #enable_clang_tidy_for_project()