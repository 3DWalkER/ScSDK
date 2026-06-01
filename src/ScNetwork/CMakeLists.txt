# Define main project utility library with dependencies
sc_add_project(${SC_SDK_NETWORK} "utility library for the Sc C++ SDK")

option(SC_ENABLE_HPSOCKET "" ON)

# Preprocessor definitions for library export
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
	SC_API_EXPORTS
	$<$<BOOL:${SC_ENABLE_HPSOCKET}>:SC_ENABLE_HPSOCKET>
)

if(SC_ENABLE_HPSOCKET)
    set(HP_ROOT        "I:/Program/1/ScSDK/src/ScNetwork/deps/HP-Socket/HP-Socket/Windows/Project")
    set(HP_VCXPROJ     "${HP_ROOT}/HPSocket-2022.sln")

    # 1. 定义 VS 项目路径
    set(HP_SOCKET_VCXPROJ
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/HP-Socket/HP-Socket/Windows/Project/HPSocket-2022/HPSocketDLL.vcxproj"
    )

    # 2. 自定义目标：只做一件事——调用 msbuild
    add_custom_target(
        HP-Socket
        COMMAND
            msbuild
            /t:Build
            /p:Configuration=Release
            /p:Platform=x64
            /m
            "${HP_VCXPROJ}"
        WORKING_DIRECTORY
            "${HP_ROOT}"
        COMMENT
            "=== 正在编译 HP-Socket ==="
        VERBATIM
    )

    # 3. 让你的主项目依赖它（但默认不会自动跑）
    add_dependencies(${PROJECT_NAME} HP-Socket)

    target_include_directories(${PROJECT_NAME} PRIVATE "I:/Program/1/ScSDK/src/ScNetwork/deps/HP-Socket/HP-Socket/Windows/Include")

    target_link_libraries(${PROJECT_NAME} PRIVATE "I:/Program/1/ScSDK/src/ScNetwork/deps/HP-Socket/HP-Socket/Windows/Lib/HPSocket/x64/HPSocket.lib")
endif()