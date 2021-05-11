 #
 #             ORXONOX - the hottest 3D action shooter ever to exist
 #                             > www.orxonox.net <
 #
 #        This program is free software; you can redistribute it and/or
 #         modify it under the terms of the GNU General Public License
 #        as published by the Free Software Foundation; either version 2
 #            of the License, or (at your option) any later version.
 #
 #       This program is distributed in the hope that it will be useful,
 #        but WITHOUT ANY WARRANTY; without even the implied warranty of
 #        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #                 GNU General Public License for more details.
 #
 #   You should have received a copy of the GNU General Public License along
 #      with this program; if not, write to the Free Software Foundation,
 #     Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 #
 #
 #  Author:
 #    Reto Grieder
 #  Description:
 #    Several functions that help organising the source tree.
 #    [ADD/SET]_SOURCE_FILES - Writes source files to the cache by force and
 #                             adds the current directory.
 #                             Also compiles multiple source files into a single
 #                             one by including them
 #                             Use COMPILATION_[BEGIN|END] in
 #                             [ADD|SET]_SOURCE_FILES and specify the name of
 #                             the new source file after COMPILATION_BEGIN
 #    GET_ALL_HEADER_FILES   - Finds all header files recursively.
 #    GENERATE_SOURCE_GROUPS - Set Visual Studio source groups.
 #

FUNCTION(PREPARE_SOURCE_FILES)
  SET(_fullpath_sources)
  FOREACH(_file ${ARGN})
    IF(_file STREQUAL "COMPILATION_BEGIN")
      SET(_compile TRUE)
      # Next file is the name of the compilation
      SET(_get_name TRUE)
    ELSEIF(_get_name)
      SET(_get_name FALSE)
      SET(_compilation_name ${_file})
    ELSEIF(_file STREQUAL "COMPILATION_END")
      IF(NOT _compilation_name)
        MESSAGE(FATAL_ERROR "No name provided for source file compilation")
      ENDIF()
      IF(NOT DISABLE_COMPILATIONS)
        SET(_compilation_file ${CMAKE_CURRENT_BINARY_DIR}/${_compilation_name})
        SET(_include_string)
        FOREACH(_file2 ${_compilation})
          SET(_include_string "${_include_string}#include \"${_file2}\"\n")
        ENDFOREACH(_file2)
        IF(EXISTS ${_compilation_file})
          FILE(READ ${_compilation_file} _include_string_file)
        ENDIF()
        IF(NOT _include_string STREQUAL "${_include_string_file}")
          FILE(WRITE ${_compilation_file} "${_include_string}")
        ENDIF()
        LIST(APPEND _fullpath_sources ${_compilation_file})
        # MSVC hack that excludes the compilations from the intellisense database
        # (There is a bug with the "-" instead of "/". Only works for "Zm#" argument)
        # 2nd Note: Exploiting this results in a strange separation of the compilation
        # file, causing the compiler not to use multi processing --> slower compiling.
        #IF(MSVC)
        #    SET_SOURCE_FILES_PROPERTIES(${_compilation_file} PROPERTIES COMPILE_FLAGS "-Zm1000")
        #ENDIF()
      ENDIF()
      SET(_compilation_name)
      SET(_compilation)
      SET(_compile FALSE)
    ELSE()
      # Prefix the full path
      GET_SOURCE_FILE_PROPERTY(_filepath ${_file} LOCATION)
      LIST(APPEND _fullpath_sources ${_filepath})
      IF(_compile AND NOT DISABLE_COMPILATIONS)
        LIST(APPEND _compilation ${_filepath})
        LIST(APPEND _fullpath_sources "H")
      ENDIF()
    ENDIF()
  ENDFOREACH(_file)
  SET(_fullpath_sources ${_fullpath_sources} PARENT_SCOPE)
ENDFUNCTION(PREPARE_SOURCE_FILES)


# Adds source files with the full path to a list
FUNCTION(ADD_SOURCE_FILES _varname)
  PREPARE_SOURCE_FILES(${ARGN})
  # Write into the cache to avoid variable scoping in subdirs
  SET(${_varname} ${${_varname}} ${_fullpath_sources} CACHE INTERNAL "Do not edit")
ENDFUNCTION(ADD_SOURCE_FILES)


# Sets source files with the full path
FUNCTION(SET_SOURCE_FILES _varname)
  PREPARE_SOURCE_FILES(${ARGN})
  # Write into the cache to avoid variable scoping in subdirs
  SET(${_varname} ${_fullpath_sources} CACHE INTERNAL "Do not edit")
ENDFUNCTION(SET_SOURCE_FILES)


# Search the entire directory tree for header files and add them to a variable
MACRO(GET_ALL_HEADER_FILES _target_varname)
  FILE(GLOB_RECURSE ${_target_varname} ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
ENDMACRO(GET_ALL_HEADER_FILES)


# Generate source groups according to the directory structure
FUNCTION(GENERATE_SOURCE_GROUPS)

  FOREACH(_file ${ARGN})
    GET_SOURCE_FILE_PROPERTY(_full_filepath ${_file} LOCATION)
    FILE(RELATIVE_PATH _relative_path ${CMAKE_CURRENT_SOURCE_DIR} ${_full_filepath})
    IF(NOT _relative_path MATCHES "^\\.\\.")
      GET_FILENAME_COMPONENT(_relative_path ${_relative_path} PATH)
      STRING(REPLACE "/" "\\\\" _group_path "${_relative_path}")
      SOURCE_GROUP("Source\\${_group_path}" FILES ${_file})
    ELSE()
      # File is being generated in the binary directory
      SOURCE_GROUP("Generated" FILES ${_file})
    ENDIF()
  ENDFOREACH(_file)

ENDFUNCTION(GENERATE_SOURCE_GROUPS)
