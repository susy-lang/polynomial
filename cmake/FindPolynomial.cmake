# Find Polynomial
#
# Find the polynomial includes and library
#
# This module defines
#  Polynomial_XXX_LIBRARIES, the libraries needed to use polynomial.
#  POLYNOMIAL_INCLUDE_DIRS

include(SofUtils)
set(LIBS polynomial;lll;polsvmasm)

set(Polynomial_INCLUDE_DIRS "${POL_DIR}")

# if the project is a subset of main cpp-sophon project
# use same pattern for variables as Boost uses
if ((DEFINED polynomial_VERSION) OR (DEFINED cpp-sophon_VERSION))

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)
		set ("Polynomial_${L}_LIBRARIES" ${l})
	endforeach()

else()

	foreach (l ${LIBS})
		string(TOUPPER ${l} L)
		find_library(Polynomial_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
			PATH_SUFFIXES "lib${l}" "${l}" "lib${l}/Debug" "lib${l}/Release"
			NO_DEFAULT_PATH
		)

		set(Polynomial_${L}_LIBRARIES ${Polynomial_${L}_LIBRARY})

		if (DEFINED MSVC)
			find_library(Polynomial_${L}_LIBRARY_DEBUG
				NAMES ${l}
				PATHS ${CMAKE_LIBRARY_PATH}
				PATH_SUFFIXES "lib${l}/Debug" 
				NO_DEFAULT_PATH
			)
			sof_check_library_link(Polynomial_${L})
		endif()
	endforeach()

endif()
