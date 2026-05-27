# Define main project utility library with dependencies
sc_add_qt_project(${SC_SDK_QT_UTILS} "utility library for the Sc C++ SDK"
	QT_COMPONENTS Widgets Sql
)

# Preprocessor definitions for library export and fmtlib symbol visibility
target_compile_definitions(${PROJECT_NAME} PRIVATE QU_API_EXPORTS)