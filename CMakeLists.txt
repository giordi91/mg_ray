cmake_minimum_required(VERSION 3.6.0)

project(mg_ray)      

	include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/middleware)
	SET(RAY_INCLUDES  ../src/threading.h 
	                  ../src/world.h 
					  ../src/hitable.h
					  )
	
	SET(RAY_CPP ../src/main.cpp 
				#../src/world.cpp 
	            #../src/hitable.cpp
				)
    #adding the executable
    add_executable(${PROJECT_NAME} ${RAY_CPP} ${RAY_INCLUDES})
	#disabling c runtime warnings, mainly coming from sbt
	add_definitions( -D_CRT_SECURE_NO_WARNINGS)
