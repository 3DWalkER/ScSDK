#[[
	Macro: sc_obtain_project_version
	Description:
		Retrieves the project version string. It prioritizes the latest Git tag 
		via 'git describe'. If Git is unavailable or fails, it attempts to read 
		from a local 'VERSION' file. If both fail, it defaults to '1.0.0'.
	
	Arguments:
		None.
]]
macro(sc_obtain_project_version)
	# Check if the directory is a valid git repository before executing git commands
	if (GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
		execute_process(
			COMMAND ${GIT_EXECUTABLE} --git-dir=${CMAKE_CURRENT_SOURCE_DIR}/.git describe --abbrev=0 --tags
			OUTPUT_VARIABLE VERSION_STRING
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
        )

		execute_process(
			COMMAND ${GIT_EXECUTABLE} --git-dir=${CMAKE_CURRENT_SOURCE_DIR}/.git rev-parse HEAD
			RESULT_VARIABLE git_result
			OUTPUT_VARIABLE GIT_HASH
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_QUIET
        )

		if (NOT git_result)
			message(STATUS "Building git hash: ${GIT_HASH}")
        endif()
	endif()

	if (NOT VERSION_STRING)
		set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
		if(EXISTS "${VERSION_FILE}")
			file(READ "${VERSION_FILE}" VERSION_STRING)
		else()
			set(VERSION_STRING "1.0.0")
		endif()
	endif()

	set(PROJECT_VERSION "${VERSION_STRING}")
endmacro()


#[[
	Macro: sc_add_project
	Description:
		Initializes a standard project, automatically resolves its version, defines 
		a library target (SHARED or STATIC), and configures common properties including 
		output name, compile definitions, target include directories, dependency linking, 
		and installation settings.
	
	Arguments:
		- name        : Name of the project and the generated target.
		- inc_dir     : The include directory path for public/interface headers.
		- description : Brief description of the project.
		- [DEPENDS]   : Optional, variadic list of target dependencies or extra libraries to link against.
]]
macro(sc_add_project name description)
	# Parse dependencies while keeping backwards compatibility with raw ARGN arguments
	cmake_parse_arguments(PROJ "" "" "DEPENDS" ${ARGN})
	if(PROJ_DEPENDS)
		set(PROJECT_LIBS "${PROJ_DEPENDS}")
	else()
		set(PROJECT_LIBS "${PROJ_UNPARSED_ARGUMENTS}")
	endif()
	
	set(SC_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
	sc_get_unique_subdir(SC_INCLUDE_DIR "${SC_PROJECT_DIR}/include")
	
	# Specify the project suffix under Debug configuration
	if (NOT DEFINED CMAKE_DEBUG_POSTFIX)
		set(CMAKE_DEBUG_POSTFIX "d")
	endif()

	# Enforce rigid standard policies locally inside the function scope
	# CMP0028: Double colons (::) in target names strictly represent ALIAS or IMPORTED targets.
	# CMP0048: The project() command natively manages and updates VERSION variables.
	# CMP0054: The if() command will not implicitly dereference quoted arguments or keywords.
	# CMP0056: The try_compile() source-file signature correctly honors CMAKE_EXE_LINKER_FLAGS.
	foreach(policy_id CMP0028 CMP0048 CMP0054 CMP0056)
        if(POLICY ${policy_id})
            cmake_policy(SET ${policy_id} NEW)
        endif()
    endforeach()
	
	# Resolve versioning properties
	sc_obtain_project_version()

	project(${name} VERSION "${PROJECT_VERSION}" LANGUAGES CXX C)
	
	# Collect source and header files
	sc_collect_all_files()

	# Automatically define library targets
	if(SC_BUILD_SHARED_LIBS)
		add_library(${PROJECT_NAME} SHARED ${SC_SOURCES_ALL} ${SC_HEADERS_ALL})
	else()
		add_library(${PROJECT_NAME} STATIC ${SC_SOURCES_ALL} ${SC_HEADERS_ALL})
	endif()
	
	# Set the actual output file name
	string(TOLOWER ${name} SC_OUTPUT_NAME)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		OUTPUT_NAME "${SC_OUTPUT_NAME}"
		DESCRIPTION "${PROJECT_DESCRIPTION}"
	)
	
	# Inject clean build/install interface paths safely into the target property
	target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
	
	# Link dependencies strictly as PRIVATE to hide implementation details
	if(PROJECT_LIBS)
		foreach(lib IN LISTS PROJECT_LIBS)
			# Imported targets (like Qt::Core) or raw paths cannot be wrapped in BUILD_INTERFACE
			if(TARGET ${lib})
				get_target_property(_is_imported ${lib} IMPORTED)
				if(_is_imported)
					target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
				else()
					target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:${lib}>)
				endif()
			else()
				target_link_libraries(${PROJECT_NAME} PRIVATE ${lib})
			endif()
		endforeach()
	endif()

	sc_setup_install()

	sc_do_packaging()
endmacro()


#[[
	Macro: sc_add_test
	Description:
		Defines a test executable target, configures its source files,
		links requested libraries, and sets up MSVC-specific runtime environments.
	
	Arguments:
		- name        : The name of the test project and output executable target.
		- description : A brief text description of the test purpose (unused but reserved).
		- [DEPENDS]   : Optional, variadic list of target dependencies (SHARED targets are automatically resolved to PATH).
]]
macro(sc_add_test name description)	
	# Initialize the project with the specified name and languages
	project(${name} LANGUAGES CXX C)
	
	# Collect source and header files
	sc_collect_all_files("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

	add_executable(${PROJECT_NAME} ${SC_SOURCES_ALL} ${SC_HEADERS_ALL})

	# Parse dependencies while keeping backwards compatibility with raw ARGN arguments
	cmake_parse_arguments(TEST "" "" "DEPENDS" ${ARGN})
	if(TEST_DEPENDS)
		set(ALL_DEPENDS "${TEST_DEPENDS}")
	else()
		set(ALL_DEPENDS "${TEST_UNPARSED_ARGUMENTS}")
	endif()
	
	if(ALL_DEPENDS)
		target_link_libraries(${PROJECT_NAME} PRIVATE ${ALL_DEPENDS})
	endif()
	
	# Apply Windows/MSVC specific configurations
	if(MSVC)
		set(ENV_PATH_STR "")
		
		# Smart Resolution: Iterate through all dependencies and extract DLL paths automatically
		foreach(dep ${ALL_DEPENDS})
			if(TARGET ${dep})
				get_target_property(TARGET_TYPE ${dep} TYPE)
				# Append to PATH only if the target is a shared library or module
				if(TARGET_TYPE STREQUAL "SHARED_LIBRARY" OR TARGET_TYPE STREQUAL "MODULE_LIBRARY")
					set(ENV_PATH_STR "${ENV_PATH_STR}$<TARGET_FILE_DIR:${dep}>;")
				endif()
			endif()
		endforeach()
		
		# Append the utility DLL directory to the Windows PATH for the Visual Studio debugger
		set_target_properties(${PROJECT_NAME} PROPERTIES
			VS_DEBUGGER_ENVIRONMENT "PATH=${ENV_PATH_STR}%PATH%"
			WIN32_EXECUTABLE TRUE
		)
		
		# Force the linker to use standard main entry point even with WIN32_EXECUTABLE enabled
		target_link_options(${PROJECT_NAME} PRIVATE "/ENTRY:mainCRTStartup")
	endif()
endmacro()


#[[
	Macro: sc_initialize_qt
	Description:
		Initializes Qt configuration for a specific target by detecting the Qt version (Qt5/Qt6),
		enabling Qt automatic compilation tools (AUTOMOC/AUTOUIC/AUTORCC), and linking required components.
	
	Arguments:
		- qt_libs : A list of required Qt components/modules (e.g., Core;Widgets;Gui).
]]
macro(sc_initialize_qt qt_libs)
	set(QT_LIBS "${qt_libs}")
    if(QT_LIBS)
        find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_LIBS})
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_LIBS})
    endif()
	
	set_target_properties(${PROJECT_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
	
	# Setup structured global AUTOMOC filter directories for CMake 3.29+
	set_property(GLOBAL PROPERTY AUTOMOC_SOURCE_GROUP "Generated Files/Qt")
	set_property(GLOBAL PROPERTY AUTOUIC_SOURCE_GROUP "Generated Files/Qt")
	
	if(QT_LIBS)
		foreach(_comp IN LISTS QT_LIBS)
			target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::${_comp})
		endforeach()
    endif()
endmacro()


#[[
	Macro: sc_add_qt_project
	Description:
		High-level API that creates a standard project target and automatically configures
		the necessary Qt build environment, automatic tools, and dependency linkings.
	
	Arguments:
		- name          : The name of the project and output target.
		- description   : A brief text description of the project purpose.
		- [DEPENDS]     : Optional, variadic list of non-Qt library targets.
		- [QT_COMPONENTS]: Optional, variadic list of requested Qt modules (e.g., Core Widgets).
]]
macro(sc_add_qt_project name description)
	# Parse arguments to separate standard dependencies and Qt components
	cmake_parse_arguments(QT_PROJ "" "" "DEPENDS;QT_COMPONENTS" ${ARGN})

	# Forward to standard project macro
	if(QT_PROJ_DEPENDS)
		sc_add_project(${name} ${description} DEPENDS ${QT_PROJ_DEPENDS})
	else()
		# Fallback to old behavior if DEPENDS keyword is missing
		sc_add_project(${name} ${description} DEPENDS ${QT_PROJ_UNPARSED_ARGUMENTS})
	endif()
	
	# Trigger Qt initialization if modules are specified
	if(QT_PROJ_QT_COMPONENTS)
		sc_initialize_qt("${QT_PROJ_QT_COMPONENTS}")
	endif()
endmacro()


#[[
	Macro: sc_add_qt_test
	Description:
		Creates a structured Qt test executable, automatically handles DLL environment paths 
		for MSVC debugging, enables AUTOMOC/AUTOUIC/AUTORCC, and links dependencies.
	
	Arguments:
		- name          	: The name of the test project and output executable target.
		- description   	: A brief text description of the test purpose.
		- [DEPENDS]     	: Optional, variadic list of target dependencies (SHARED targets are automatically resolved to PATH).
		- [QT_COMPONENTS]	: Optional, variadic list of required Qt modules (e.g., Core Widgets Test).
]]
macro(sc_add_qt_test name description)
   # Parse standard incoming parameters using structured keywords
    cmake_parse_arguments(QT_TEST "" "" "DEPENDS;QT_COMPONENTS" ${ARGN})

	# Extract implicit dependencies if explicit DEPENDS keyword wasn't supplied
	if(NOT QT_TEST_DEPENDS AND QT_TEST_UNPARSED_ARGUMENTS)
		set(QT_TEST_DEPENDS ${QT_TEST_UNPARSED_ARGUMENTS})
	endif()

	# Initialize base test executable via regular test macro
    sc_add_test(${name} "${description}" 
        DEPENDS ${QT_TEST_DEPENDS}
    )
	
    # Initialize specialized Qt environment if modules are populated
    if(QT_TEST_QT_COMPONENTS)
        sc_initialize_qt("${QT_TEST_QT_COMPONENTS}")
    endif()
endmacro()
