macro(sc_load_external_deps)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force static libraries for dependencies" FORCE)

	file(GLOB subdirs LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/deps/*")
	foreach(subdir ${subdirs})
		add_subdirectory(${subdir} EXCLUDE_FROM_ALL)
		get_property(sub_targets DIRECTORY ${subdir} PROPERTY BUILDSYSTEM_TARGETS)
		foreach(target ${sub_targets})
			set_property(TARGET ${target} PROPERTY FOLDER "Deps/${PROJECT_NAME}")
		endforeach()
	endforeach()
endmacro()


#[[
	Macro: sc_setup_install
	Description: 
		Handles target export installation and automatically copies associated 
	public header files while explicitly filtering out private header files.
	
	Arguments:
		- inc_dir : Relative path suffix inside 'include/' directory to install from.
]]
macro(sc_setup_install inc_dir)
	if(SC_SIMPLE_INSTALL)
		install(TARGETS ${PROJECT_NAME}
			EXPORT "${PROJECT_NAME}-targets"
			ARCHIVE DESTINATION ${SC_ARCHIVE_DIRECTORY}
			LIBRARY DESTINATION ${SC_LIBRARY_DIRECTORY}
			RUNTIME DESTINATION ${SC_BINARY_DIRECTORY}
		)
	endif()

	install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/${inc_dir}" 
		DESTINATION ${SC_INCLUDE_DIRECTORY}
		COMPONENT Development
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*_p.h" EXCLUDE
	)
endmacro()


#[[
	Macro: sc_do_packaging
	Description: 
		Generates and deploys the canonical CMake Package Configuration files 
	(*-config.cmake, *-config-version.cmake, and *-targets.cmake) 
	to enable downstream client integration via find_package().
]]
macro(sc_do_packaging)
	if(SC_SIMPLE_INSTALL)
		write_basic_package_version_file(
		    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
		    VERSION ${PROJECT_VERSION}
		    COMPATIBILITY AnyNewerVersion
		)

		configure_file(
			"${SC_SDK_NATIVE_ROOT}/toolchains/cmake_project_config.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
			@ONLY
		)

		set(ConfigPackageLocation "${SC_LIBRARY_DIRECTORY}/cmake")
		install(EXPORT "${PROJECT_NAME}-targets"
			FILE "${PROJECT_NAME}-targets.cmake"
			NAMESPACE ${PROJECT_NAME}::
			DESTINATION ${ConfigPackageLocation}
        )

		install(
            FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
            DESTINATION ${ConfigPackageLocation}
            COMPONENT Devel
		)
	endif()
endmacro()