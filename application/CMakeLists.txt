
cmake_minimum_required(VERSION 3.6.0)

project(mg_rayApplication)   



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

    #adding the executable
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDES_FILES})
	target_link_libraries(${PROJECT_NAME} mg_rayLib)

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		#specific MSVC CRAP
		set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS"
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)"	
		)
		#making this start up project in visual studio
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mg_rayApplication)
	endif()


	#using this to copy the mesh objs into output directory
	add_custom_command(TARGET mg_rayApplication PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/mg_rayLib/rendering/data $<TARGET_FILE_DIR:mg_rayApplication>)




    # Link against LLVM libraries
    #target_link_libraries(${PROJECT_NAME} ${llvm_libs})

    #enabling clang tidy
    #enable_clang_tidy_for_project()