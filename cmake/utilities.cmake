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
]]
macro(sc_setup_install)
	if(SC_SIMPLE_INSTALL)
		install(TARGETS ${PROJECT_NAME}
			EXPORT "${PROJECT_NAME}-targets"
			ARCHIVE DESTINATION ${SC_ARCHIVE_DIRECTORY}
			LIBRARY DESTINATION ${SC_LIBRARY_DIRECTORY}
			RUNTIME DESTINATION ${SC_BINARY_DIRECTORY}
		)
	endif()

	install(DIRECTORY "${SC_INCLUDE_DIR}" 
		DESTINATION ${SC_INCLUDE_DIRECTORY}
		COMPONENT Development
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*_p.h" EXCLUDE
		PATTERN "*_p_*.h" EXCLUDE
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


#[[
	Function: sc_get_unique_subdir
	Description:
		Ensures there is one and only one subdirectory under the target directory,
		and returns its absolute path.
		If 0 or multiple subdirectories are found, it triggers a FATAL_ERROR.
	Arguments:
		- output_var : Name of the variable to store the unique subdirectory's absolute path.
		- target_dir : The target root directory to scan (e.g., "${CMAKE_CURRENT_SOURCE_DIR}/include").
]]
function(sc_get_unique_subdir output_var target_dir)
	file(GLOB search_result LIST_DIRECTORIES true "${target_dir}/*")

	set(subdirs "")
	foreach(item ${search_result})
		if(IS_DIRECTORY "${item}")
			list(APPEND subdirs "${item}")
		endif()
	endforeach()

	list(LENGTH subdirs dir_count)
	if(dir_count EQUAL 0)
		message(FATAL_ERROR "Error: No subdirectory found under '${target_dir}'!")
	elseif(dir_count GREATER 1)
		message(FATAL_ERROR "Error: Expected exactly 1 subdirectory under '${target_dir}', but found ${dir_count}:\n${subdirs}")
	endif()

	list(GET subdirs 0 unique_path)

	set(${output_var} "${unique_path}" PARENT_SCOPE)
endfunction()
