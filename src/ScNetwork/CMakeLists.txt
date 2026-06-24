# Define main project utility library with dependencies
sc_add_project(${SC_SDK_NETWORK} "utility library for the Sc C++ SDK")

option(SC_ENABLE_HPSOCKET "" ON)

# Preprocessor definitions for library export
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	SC_API_EXPORTS
)

if(SC_ENABLE_HPSOCKET)
    set(SC_HP_ROOT      "${CMAKE_CURRENT_SOURCE_DIR}/deps/HP-Socket")
    if(WIN32)
        set(SC_HP_VCXPROJ "${SC_HP_ROOT}/Windows/Project/HPSocketDLL-${SC_VS_YEAR}/HPSocketDLL.vcxproj")
        add_custom_target(
            HP-Socket
            COMMAND
                msbuild
                /t:Build
                /p:Configuration=Release
                /p:Platform=x64
                /m
                "${SC_HP_VCXPROJ}"
            WORKING_DIRECTORY
                "${SC_HP_ROOT}/Windows/Project"
            COMMENT
                "Compiling HP Socket..."
            VERBATIM
        )

        add_dependencies(${PROJECT_NAME} HP-Socket)
        target_include_directories(${PROJECT_NAME} PRIVATE "${SC_HP_ROOT}/Windows/Include")
        target_link_libraries(${PROJECT_NAME} PRIVATE "${SC_HP_ROOT}/Windows/Lib/HPSocket/x64/HPSocket.lib")
    endif()
endif()