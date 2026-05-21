# Define main project utility library with dependencies
sc_add_qt_project(${SC_SDK_FRAMELESS} "frameless" "Qt frameless helper"
	QT_COMPONENTS Widgets
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
	${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS}
	${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS}
)

# Preprocessor definitions for library export and fmtlib symbol visibility
target_compile_definitions(${PROJECT_NAME} PRIVATE SC_FRAMELESS_EXPORTS)