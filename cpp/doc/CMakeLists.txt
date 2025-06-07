# ==============================================================================
# VISTA-SDK-CPP - C++ SDK Doxygen Documentation
# ==============================================================================

# --- Doxygen Tool Detection ---
find_package(Doxygen COMPONENTS dot)

# --- Doxygen Configuration Setup ---
if(Doxygen_FOUND)
	message(STATUS "Doxygen found: ${Doxygen_EXECUTABLE}")
	message(STATUS "Doxygen Dot (from Graphviz) found: ${Doxygen_DOT_EXECUTABLE}")
	message(STATUS "Configuring Doxygen documentation...")

	# --- Project ---
	set (	DOXYGEN_PROJECT_NAME			"DNV Vista C++ SDK"			)
	set (	DOXYGEN_PROJECT_NUMBER			"0.1"						)
	set (	DOXYGEN_PROJECT_BRIEF			"Work in progress"			)

	set (	DOXYFILE_ENCODING				"UTF-8"						)
	set (	DOXYGEN_CREATE_SUBDIRS			"YES"						)
	set (	DOXYGEN_ALLOW_UNICODE_NAMES		"NO"						)
	set (	DOXYGEN_OUTPUT_LANGUAGE			"English"					)
	set (	DOXYGEN_OUTPUT_TEXT_DIRECTION	"None"						)
	set (	DOXYGEN_BRIEF_MEMBER_DESC		"YES"						)
	set (	DOXYGEN_REPEAT_BRIEF			"YES"						)
	set (	DOXYGEN_ABBREVIATE_BRIEF		"The $name class"
											"The $name widget"
											"The $name file"
											is
											provides
											specifies
											contains
											represents
											a
											an
											the							)
	set (	DOXYGEN_FULL_PATH_NAMES			"NO"						)
	set (	DOXYGEN_INHERIT_DOCS			"YES"						)
	set (	DOXYGEN_TAB_SIZE				4							)
	set (	DOXYGEN_MARKDOWN_SUPPORT		"YES"						)
	set (	DOXYGEN_TOC_INCLUDE_HEADINGS	5							)
	set (	DOXYGEN_AUTOLINK_SUPPORT		"YES"						)
	set (	DOXYGEN_SUBGROUPING				"YES"						)

	# --- Project ---
	set (	DOXYGEN_RESOLVE_UNNAMED_PARAMS	"YES"						)
	set (	DOXYGEN_INLINE_INFO				"YES"						)
	set (	DOXYGEN_SORT_MEMBER_DOCS		"YES"						)
	set (	DOXYGEN_GENERATE_TODOLIST		"YES"						)
	set (	DOXYGEN_SHOW_USED_FILES			"YES"						)
	set (	DOXYGEN_SHOW_FILES				"YES"						)
	set (	DOXYGEN_SHOW_NAMESPACES			"YES"						)

	# --- Warnings ---
	set (	DOXYGEN_QUIET					"YES"						)
	set (	DOXYGEN_WARNINGS				"YES"						)
	set (	DOXYGEN_WARN_IF_UNDOCUMENTED	"YES"						)

	# --- Input ---
	set (	DOXYGEN_EXCLUDE_PATTERNS		"*/vendor/*" 				)
	set (	DOXYGEN_RECURSIVE				"YES"						)

	# --- Source ---
	set (	DOXYGEN_SOURCE_BROWSER			"NO"						)
	set (	DOXYGEN_INLINE_SOURCES			"NO"						)
	set (	DOXYGEN_STRIP_CODE_COMMENTS		"YES"						)
	set (	DOXYGEN_REFERENCED_BY_RELATION	"NO"						)
	set (	DOXYGEN_REFERENCES_RELATION		"NO"						)
	set (	DOXYGEN_REFERENCES_LINK_SOURCE	"NO"						)
	set (	DOXYGEN_SOURCE_TOOLTIPS			"YES"						)
	set (	DOXYGEN_USE_HTAGS				"NO"						)
	set (	DOXYGEN_VERBATIM_HEADERS		"YES"						)
	set (	DOXYGEN_CLANG_ASSISTED_PARSING	"NO"						)
	set (	DOXYGEN_CLANG_ADD_INC_PATHS		"NO"						)
	set (	DOXYGEN_CLANG_OPTIONS			""							)
	set (	DOXYGEN_CLANG_DATABASE_PATH		""							)

	# --- sorting ---
	set (	DOXYGEN_ALPHABETICAL_INDEX		"YES"						)
	set (	DOXYGEN_IGNORE_PREFIX			""							)

	# --- html ---
	set (	DOXYGEN_GENERATE_HTML			"YES"						)
	set (	DOXYGEN_HTML_OUTPUT				"html"						)
	set (	DOXYGEN_HTML_FILE_EXTENSION		".html"						)
	set (	DOXYGEN_HTML_HEADER				""							)
	set (	DOXYGEN_HTML_FOOTER				""							)
	#set (	DOXYGEN_HTML_STYLESHEET         "doxygen.css"				)
	#set (	DOXYGEN_HTML_EXTRA_STYLESHEET	""							)
	#set (	DOXYGEN_HTML_EXTRA_FILES		""							)
	set (	DOXYGEN_HTML_COLORSTYLE			"LIGHT"						)
	set (	DOXYGEN_HTML_COLORSTYLE_HUE		"220"						)
	set (	DOXYGEN_HTML_COLORSTYLE_SAT		"100"						)
	set (	DOXYGEN_HTML_COLORSTYLE_GAMMA	"80"						)
	set (	DOXYGEN_HTML_TIMESTAMP			"NO"						)
	set (	DOXYGEN_HTML_DYNAMIC_MENUS		"NO"						)
	set (	DOXYGEN_HTML_DYNAMIC_SECTIONS	"NO"						)
	set (	DOXYGEN_HTML_INDEX_NUM_ENTRIES	"100"						)
	set (	DOXYGEN_GENERATE_DOCSET			"NO"						)
	#set (	DOXYGEN_DOCSET_FEEDNAME			"Doxygen generated docs"	)
	#set (	DOXYGEN_DOCSET_BUNDLE_ID		"org.doxygen.Project"		)
	#set (	DOXYGEN_DOCSET_PUBLISHER_ID		"org.doxygen.Publisher"		)
	#set (	DOXYGEN_DOCSET_PUBLISHER_NAME	"Publisher"					)
	set (	DOXYGEN_GENERATE_HTMLHELP		"NO"						)
	set (	DOXYGEN_CHM_FILE				""							)
	set (	DOXYGEN_HHC_LOCATION			""							)
	set (	DOXYGEN_GENERATE_CHI			"NO"						)
	set (	DOXYGEN_CHM_INDEX_ENCODING		""							)
	set (	DOXYGEN_BINARY_TOC				"NO"						)
	set (	DOXYGEN_TOC_EXPAND				"NO"						)
	set (	DOXYGEN_GENERATE_QHP			"NO"						)
	set (	DOXYGEN_QCH_FILE				""							)
	set (	DOXYGEN_QHP_NAMESPACE			"org.doxygen.Project"		)
	set (	DOXYGEN_QHP_VIRTUAL_FOLDER		"doc"						)
	set (	DOXYGEN_QHP_CUST_FILTER_NAME	""							)
	set (	DOXYGEN_QHP_CUST_FILTER_ATTRS	""							)
	set (	DOXYGEN_QHP_SECT_FILTER_ATTRS	""							)
	set (	DOXYGEN_QHG_LOCATION			""							)
	set (	DOXYGEN_GENERATE_ECLIPSEHELP	"NO"						)
	set (	DOXYGEN_ECLIPSE_DOC_ID			"org.doxygen.Project"		)
	set (	DOXYGEN_DISABLE_INDEX			"NO"						)
	set (	DOXYGEN_GENERATE_TREEVIEW		"YES"						)
	set (	DOXYGEN_ENUM_VALUES_PER_LINE	"1"							)
	set (	DOXYGEN_TREEVIEW_WIDTH			"250"						)
	set (	DOXYGEN_EXT_LINKS_IN_WINDOW		"NO"						)
	set (	DOXYGEN_HTML_FORMULA_FORMAT		"png"						)
	set (	DOXYGEN_FORMULA_FONTSIZE		"10"						)
	set (	DOXYGEN_FORMULA_TRANSPARENT		"YES"						)
	set (	DOXYGEN_FORMULA_MACROFILE		""							)
	set (	DOXYGEN_USE_MATHJAX				"NO"						)
	set (	DOXYGEN_MATHJAX_FORMAT			"HTML-CSS"					)
	set (	DOXYGEN_MATHJAX_RELPATH			"https://cdn.jsdelivr.net/npm/mathjax@2"	)
	set (	DOXYGEN_MATHJAX_EXTENSIONS		""							)
	set (	DOXYGEN_MATHJAX_CODEFILE		""							)
	set (	DOXYGEN_SEARCHENGINE			"YES"						)
	set (	DOXYGEN_SERVER_BASED_SEARCH		"NO"						)
	set (	DOXYGEN_EXTERNAL_SEARCH			"NO"						)
	set (	DOXYGEN_SEARCHENGINE_URL		""							)
	set (	DOXYGEN_SEARCHDATA_FILE			"searchdata.xml"			)
	set (	DOXYGEN_EXTERNAL_SEARCH_ID		""							)
	set (	DOXYGEN_EXTRA_SEARCH_MAPPINGS	""							)

	# --- Preprocessor ---
	set (	DOXYGEN_ENABLE_PREPROCESSING	"NO"						)
	set (	DOXYGEN_SEARCH_INCLUDE			"YES"						)
	set (	DOXYGEN_SKIP_FUNCTION_MACROS	"YES"						)
	set (	DOXYGEN_MACRO_EXPANSION			"YES"						)
	set (	DOXYGEN_EXPAND_ONLY_PREDEF		"YES"						)
	set (	DOXYGEN_PREDEFINED				"protected=private"			)
	set (	DOXYGEN_EXTRACT_PRIVATE			"NO"						)
	set (	DOXYGEN_EXTRACT_ALL				"YES"						)

	# --- Dot ---
	set (	DOXYGEN_CLASS_DIAGRAMS			"YES"						)
	set (	DOXYGEN_DIA_PATH				""							)
	set (	DOXYGEN_HIDE_UNDOC_RELATIONS	"YES"						)
	set (	DOXYGEN_HAVE_DOT				"YES"						) # Doxygen_DOT_FOUND from find_package will be true here
	set (	DOXYGEN_DOT_NUM_THREADS			"0"							)
	set (	DOXYGEN_DOT_FONTNAME			"Helvetica"					)
	set (	DOXYGEN_DOT_FONTSIZE			"10"						)
	set (	DOXYGEN_DOT_FONTPATH			""							)
	set (	DOXYGEN_CLASS_GRAPH				"YES"						)
	set (	DOXYGEN_COLLABORATION_GRAPH		"YES"						)
	set (	DOXYGEN_GROUP_GRAPHS			"YES"						)
	set (	DOXYGEN_UML_LOOK				"NO"						)
	set (	DOXYGEN_UML_LIMIT_NUM_FIELDS	"10"						)
	set (	DOXYGEN_DOT_UML_DETAILS			"YES"						)
	set (	DOXYGEN_DOT_WRAP_THRESHOLD		"17"						)
	set (	DOXYGEN_TEMPLATE_RELATIONS		"NO"						)
	set (	DOXYGEN_INCLUDE_GRAPH			"YES"						)
	set (	DOXYGEN_INCLUDED_BY_GRAPH		"YES"						)
	set (	DOXYGEN_CALL_GRAPH				"NO"						)
	set (	DOXYGEN_CALLER_GRAPH			"NO"						)
	set (	DOXYGEN_GRAPHICAL_HIERARCHY		"YES"						)
	set (	DOXYGEN_DIRECTORY_GRAPH			"YES"						)
	set (	DOXYGEN_DOT_IMAGE_FORMAT		"png"						)
	set (	DOXYGEN_INTERACTIVE_SVG			"NO"						)
	set (	DOXYGEN_DOT_PATH				""							) # Doxygen will use Doxygen_DOT_EXECUTABLE from find_package
	set (	DOXYGEN_DOTFILE_DIRS			""							)
	set (	DOXYGEN_MSCFILE_DIRS			""							)
	set (	DOXYGEN_DIAFILE_DIRS			""							)
	set (	DOXYGEN_PLANTUML_JAR_PATH		""							)
	set (	DOXYGEN_PLANTUML_CFG_FILE		""							)
	set (	DOXYGEN_PLANTUML_INCLUDE_PATH	""							)
	set (	DOXYGEN_DOT_GRAPH_MAX_NODES		"50"						)
	set (	DOXYGEN_MAX_DOT_GRAPH_DEPTH		"0"							)
	set (	DOXYGEN_DOT_TRANSPARENT			"NO"						)
	set (	DOXYGEN_DOT_MULTI_TARGETS		"NO"						)
	set (	DOXYGEN_GENERATE_LEGEND			"YES"						)
	set (	DOXYGEN_DOT_CLEANUP				"YES"						)

	# --- Documentation Target Definition --
	doxygen_add_docs( documentation
				${VISTA_SDK_CPP_INCLUDE_DIR}/dnv/vista/sdk/
				${VISTA_SDK_CPP_SOURCE_DIR}/dnv/vista/sdk/
				ALL
				COMMENT "Generate documentation."
	)

	if(TARGET documentation)
			add_custom_command(
				TARGET documentation
					POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/doc"
					COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/${DOXYGEN_HTML_OUTPUT}/" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/doc/html/"
					COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/index.html" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/doc/index.html"
					COMMENT "Copying Doxygen documentation to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/doc"
					VERBATIM
			)
	endif()
else()
	message(STATUS "Doxygen and/or Dot not found. Skipping documentation generation.")
endif()
