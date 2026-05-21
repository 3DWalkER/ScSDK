# ---------------------------------------------------------------------------------------
# IDE Support
# ---------------------------------------------------------------------------------------
#[[
	Function: sc_collect_filter
	Description:
		Globs files with specific extensions from a directory and filters them 
	using a regular expression.
	
	Arguments:
		- output_var  : Name of the variable to store the filtered file list.
		- dir         : Target directory to scan.
		- file_exts   : List of file extensions to search for (e.g., "cpp;c").
		- filter_regx : Regex string used to exclude matching files.
]]
function(sc_collect_filter output_var dir file_exts filter_regx)
	set(files "")
	foreach(ext IN LISTS file_exts)
		file(GLOB temp_files "${dir}/*.${ext}")
		list(APPEND files ${temp_files})
	endforeach()

	if(filter_regx)
		list(REMOVE_ITEM files "")
		if(files)
			list(FILTER files EXCLUDE REGEX "${filter_regx}")
		endif()
	endif()

	set(${output_var} ${files} PARENT_SCOPE)
endfunction()


#[[
	Function: sc_collect_files
	Description:
		Recursively collects files from a directory and its first-level subdirectories.
	It also automatically maps them into Visual Studio solution folders (source_group).
	
	Arguments:
		- output_var  : Name of the variable to store the total collected files.
		- group_name  : Root name of the Visual Studio IDE folder.
		- dir         : Target root directory to scan.
		- file_exts   : List of file extensions to search for.
		- filter_regx : Regex string used to exclude matching files.
]]
function(sc_collect_files output_var group_name dir file_exts filter_regx)
	set(files "")
	
	file(GLOB subdirs LIST_DIRECTORIES true "${dir}/*")
	foreach(subdir ${subdirs})
		if(NOT IS_DIRECTORY "${subdir}")
			continue()
		endif()

		sc_collect_filter(temp_files "${subdir}" "${${file_exts}}" "${filter_regx}")
		list(APPEND files ${temp_files})
		if(MSVC AND temp_files)
			get_filename_component(last_name ${subdir} NAME)
			source_group("${group_name}/${last_name}" FILES ${temp_files})
		endif()
	endforeach()
	
	sc_collect_filter(temp_files "${dir}" "${${file_exts}}" "${filter_regx}")
	list(APPEND files ${temp_files})
	if(MSVC AND temp_files)
		source_group("${group_name}" FILES ${temp_files})
	endif()
	
	set(${output_var} ${files} PARENT_SCOPE)
endfunction()


#[[
	Function: sc_make_ext_regex
	Description:
		Generates a platform-specific exclusion regular expression based on file extensions.
	Filters out targeting OS files (e.g., excludes '_linux' on Windows and vice versa).
	
	Arguments:
		- output_reg_var : Name of the variable to store the generated regex string.
		- ext_var        : Variable name holding the list of file extensions.
]]
function(sc_make_ext_regex output_reg_var ext_var)
	set(exts "${${ext_var}}")
	string(REPLACE ";" "|" ext_regex "${exts}")
	set(ext_regex "(${ext_regex})")

	if(WIN32)
		set(filter_reg ".*_(linux|unix)\\.${ext_regex}$")
	else()
		set(filter_reg ".*_win\\.${ext_regex}$")
	endif()

	set(${output_reg_var} "${filter_reg}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------------------
# High-Level API (Parametrized Directory Search)
# ---------------------------------------------------------------------------------------
#[[
	Function: sc_collect_headers
	Description:
		Public API to collect header files from a specified directory.
	Defaults to project standard include path if no directory is provided.
	
	Arguments:
		- search_dir : (Optional) The directory to search. 
]]
function(sc_collect_headers)
	set(SC_HEADER_EXTENSIONS "h" "hpp")
	
	if(ARGN)
		set(target_dir "${ARGV0}")
	else()
		set(target_dir "${SC_PROJECT_DIR}/include/${SC_INCLUDE_DIR}")
	endif()

	sc_make_ext_regex(SC_HEADER_FILTER_REG SC_HEADER_EXTENSIONS)
	sc_collect_files(temp_headers "Header Files" "${target_dir}" SC_HEADER_EXTENSIONS "${SC_HEADER_FILTER_REG}")
	
	set(SC_HEADERS_ALL "${temp_headers}" PARENT_SCOPE)
endfunction()


#[[
	Function: sc_collect_sources
	Description:
		Public API to collect source files from a specified directory.
	Applies platform-specific filters and defaults to standard src directory.
	
	Arguments:
		- search_dir : (Optional) The directory to search.
]]
function(sc_collect_sources)
	set(SC_SOURCE_EXTENSIONS "c" "cpp")
	
	if(ARGN)
		set(target_dir "${ARGV0}")
	else()
		set(target_dir "${SC_PROJECT_DIR}/src")
	endif()

	sc_make_ext_regex(SC_SOURCE_FILTER_REG SC_SOURCE_EXTENSIONS)
	sc_collect_files(temp_sources "Source Files" "${target_dir}" SC_SOURCE_EXTENSIONS "${SC_SOURCE_FILTER_REG}")
	
	set(SC_SOURCES_ALL "${temp_sources}" PARENT_SCOPE)
endfunction()


#[[
	Function: sc_collect_all_files
	Description:
		A unified wrapper that simultaneously collects both header and source files.
	Supports custom directories or falls back to default paths if arguments are omitted.
	
	Arguments:
		- headers_dir : (Optional) The directory to search for headers.
		- sources_dir : (Optional) The directory to search for sources.
]]
function(sc_collect_all_files)
	if(ARGN)
		sc_collect_headers("${ARGV0}")
		sc_collect_sources("${ARGV1}")
	else()
		sc_collect_headers("")
		sc_collect_sources("")
	endif()

	set(SC_HEADERS_ALL "${SC_HEADERS_ALL}" PARENT_SCOPE)
	set(SC_SOURCES_ALL "${SC_SOURCES_ALL}" PARENT_SCOPE)
endfunction()
