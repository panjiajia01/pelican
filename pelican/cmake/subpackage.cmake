#===============================================================================
# subpackage.cmake
#
# Macros for handling sub-package creation
# 
# Sets: 
#   SUBPACKAGE_WORK_DIR: the work directory for sub-package module files.
#
#
# Defines the following public macros:
#   SUBPACKAGE: register a sub-package and set / get dependencies.
#   SUBPACKAGE_LIBRARY: Create the subpackage library.
#
#
# Defines the following utility macros:
#   SUBPACKAGE_GET_LIBS: Get the libraries required to link the specified 
#                        sub-package
#   SUBPACKAGE_GET_INCS: Get the includes required for the specified sub-package.
#
#
#===============================================================================

#
# Macro to register and set / get sub-package dependencies.
#
macro(SUBPACKAGE name)

    # Set the current sub-package name.
    set(subpackage_current "${name}")
    
    # Set the name of the sub-package file.
    set(subpackage_file "${SUBPACKAGE_WORK_DIR}/${name}.cmake")

    # Initialise the sub-package file with an include guard to prevent double
    # loading.
    file(WRITE ${subpackage_file}
        "# Sub-package: '${name}'\n"
        "#  This file is auto-generated by the build system - do not edit.\n"
        "if(subpackage_${name}_ADDED)\n"
        "   return()\n"
        "endif(subpackage_${name}_ADDED)\n"
        "set(subpackage_${name}_ADDED true)\n\n"
    )

    # Add include_directories() defined before this macro to the sub-package file.
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # TODO: remove this when includes are fixed.
    #message(STATUS "---- INCLUDES[${name}]: ${includes}")
    if(COMMAND GET_PROPERTY)
        get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    else(COMMAND GET_PROPERTY)
        # cmake 2.4 compatibility, just include everthing.
        set(includes ${CMAKE_INCLUDE_PATH})
    endif(COMMAND GET_PROPERTY)
    list(REMOVE_DUPLICATES includes)
    #message(STATUS "---- INCLUDES[${name}]: ${includes}")
    file(APPEND ${subpackage_file} "include_directories(${includes})\n\n")
    
    # Process packages that the sub-package depends on.
    set(subpackage_${name}_DEPS ${ARGN})

    # If any dependencies exist.    
    if(subpackage_${name}_DEPS)
        # Reverse the list of dependencies.
        list(REVERSE subpackage_${name}_DEPS)
        # Add the list of dependencies to the sub-package file.
        file(APPEND ${subpackage_file}
            "set(subpackage_${name}_DEPS ${subpackage_${name}_DEPS})\n\n"
        )
    endif(subpackage_${name}_DEPS)

    # Add the package files of dependent sub-packages to the sub-package file.
    foreach(pack ${ARGN})
        file(APPEND ${subpackage_file} 
            "include(${SUBPACKAGE_WORK_DIR}/${pack}.cmake)\n")
    endforeach(pack)

    # Loop over the sub-package depenency list, loading their subpackage module
    # files to get the required librarys.
    foreach(dep ${subpackage_${name}_DEPS})
        # Load the sub-package file.
        include(${SUBPACKAGE_WORK_DIR}/${dep}.cmake)
        # Extract the required libraries setting ${SUBPACKAGE_LIBRARIES}.
        _SUBPACKAGE_GET_LIBS(${dep})        
    endforeach(dep)
    
    # Remove duplicates from sub-package libraries ?
    # TODO?

    # Create the install target for header files of the sub-package.
    #---------------------------------------------------------------------------
    file(GLOB public_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
    install(FILES ${public_headers} DESTINATION ${INCLUDE_INSTALL_DIR}/${name})
        
    # TODO: instead of writing to the subpackge file thoughout.
    # add what is needed at the very end or just set variables and delay 
    # this to another macro.

    file(APPEND ${subpackage_file} "#-------------------------------------\n\n")

endmacro(SUBPACKAGE)



#
# Create a subpackage library.
#
macro(SUBPACKAGE_LIBRARY name)

    # If the module / sub-package is defined (set by LIBRARY_MODULE_REQUIREMENTS)
    if(subpackage_current MATCHES "${name}")

        # Add target for the dynamic library.
        add_library("${name}" SHARED ${ARGN})

        # If building a single package library link against main library.
        #if(BUILD_SINGLE_LIB)
        #    if(PROJECT_LIBS)
        #        SUBPACKAGE_ADD_LIBRARIES(${name} "${PROJECT_LIBS}")
        #        #message(STATUS "====== ${name}: ${PROJECT_LIBS}")
        #    else(PROJECT_LIBS)
        #        message(FATAL_ERROR "Error: No PROJECT_LIBS set for '${name}' subpackage.")
        #    endif(PROJECT_LIBS)
        #else(BUILD_SINGLE_LIB)
            #SUBPACKAGE_ADD_LIBRARIES(${name} "${name}")
             file(APPEND ${subpackage_file}
                "list(INSERT subpackage_${name}_LIBS 0 ${name})\n\n")
            
        #endif(BUILD_SINGLE_LIB)
        
        # Add list of object files to sub-pacakge file for use with a single
        # library build.
        _SUBPROJECT_OBJECT_FILES("${name}" "${name}_shared_objects")
        file(APPEND ${subpackage_file}
            "list(INSERT shared_libs 0 "${name}")\n\n"
            "list(INSERT shared_objects 0 ${${name}_shared_objects})\n\n"
        )

    else(subpackage_current MATCHES "${name}")
        message(FATAL_ERROR "ERROR: SUBPACKAGE_LIBRARY for '${name}' "
            " specified outside of a SUBPACKAGE context")
    endif(subpackage_current MATCHES "${name}")

endmacro(SUBPACKAGE_LIBRARY)


#
# 
#
macro(SUBPACKAGE_ADD_LIBRARIES name)

    if(subpackage_current MATCHES ${name})
        list(INSERT SUBPACKAGE_LIBRARIES 0 ${ARGN})
        list(INSERT SUBPACKAGE_${SUBPACKAGE_CURRENT}_LIBS 0 ${ARGN})
        file(APPEND ${subpackage_file}
            "list(INSERT subpackage_${name}_external_LIBS 0 ${ARGN})\n\n")
    else(subpackage_current MATCHES ${name})
        message(FATAL_ERROR "SUBPACKAGE_ADD_LIBRARIES for '${name}' "
            "specified outside of a SUBPACKAGE context")
    endif(subpackage_current MATCHES ${name})
    
endmacro(SUBPACKAGE_ADD_LIBRARIES)

#
# Macro to register and set / get sub-package dependencies.
#
macro(USE_SUBPACKAGES)

    set(subpackage_${name}_DEPS ${ARGN})
    
    # If any dependencies exist.    
    if(subpackage_${name}_DEPS)
        # Reverse the list of dependencies.
        list(REVERSE subpackage_${name}_DEPS)
    endif(subpackage_${name}_DEPS)
    
    # Add include_directories() defined before this macro to the sub-package file.
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    if(COMMAND GET_PROPERTY)
        get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    else(COMMAND GET_PROPERTY)
        # cmake 2.4 compatibility, just include everthing.
        set(includes ${CMAKE_INCLUDE_PATH})
    endif(COMMAND GET_PROPERTY)
    list(REMOVE_DUPLICATES includes)

    # Loop over the sub-package depenency list, loading their subpackage module
    # files to get the required librarys.
    foreach(dep ${subpackage_${name}_DEPS})
        # Load the sub-package file.
        include(${SUBPACKAGE_WORK_DIR}/${dep}.cmake)
        # Extract the required libraries setting ${SUBPACKAGE_LIBRARIES}.
        _SUBPACKAGE_GET_LIBS(${dep})        
    endforeach(dep)
        
    ## remove duplicates in SUBPACKAGE_LIBRARIES
    
endmacro(USE_SUBPACKAGES)




# ==============================================================================
# Private utility macros
# ==============================================================================

#
# Generates the SUBPACKAGE_LIBRARIES variable.
#
# This is carried out by loading package dependencies and their
# assoicatied library depencies.
#
macro(_SUBPACKAGE_GET_LIBS name)
    if(NOT subpackage_${name}_added)
    
        foreach(pack ${subpackage_${name}_DEPS})
            _SUBPACKAGE_GET_LIBS(${pack})
        endforeach(pack)
        
        if(subpackage_${name}_LIBS)
            list(INSERT SUBPACKAGE_LIBRARIES 0 ${subpackage_${name}_LIBS})
        endif(subpackage_${name}_LIBS)
        
        if(subpackage_${name}_external_LIBS)
            list(INSERT SUBPACKAGE_LIBRARIES 0 ${subpackage_${name}_external_LIBS})
        endif(subpackage_${name}_external_LIBS)
 
        set(subpackage_${name}_added TRUE)
        
    endif(NOT subpackage_${name}_added)
endmacro(_SUBPACKAGE_GET_LIBS)




#
# Extracts the object files for a sub-pacakge.
#
macro(_SUBPROJECT_OBJECT_FILES target outputObjectFiles)
    # This hack inspired by the bug report : http://www.cmake.org/Bug/view.php?id=5155
    #
    # CMake generators are currently of 2 types: those which build single configurations, and those
    # which build multiple configurations. These 2 types use 2 different directory structures for where
    # they put their object files. The currently recommended way to deduce which type of generator
    # we're using, is to see if CMAKE_CONFIGURATION_TYPES is empty or not. If it's empty, then it's
    # single configuration. If it's non-empty, then it's multiple configuration, and contains a list of all
    # the configurations available. We're not interested in that list, only whether it's empty or non-empty.

    if(CMAKE_CONFIGURATION_TYPES)
        # We have a multiple configuration generator. Use this directory structure.
        #
        # Note that CMAKE_BUILD_TYPE has no value when Visual Studio .sln files are generated.
        # This is because on MSVC, no build type is actually selected at generation time. The MSVC
        # user typically selects her build type after opening the .sln file. CMAKE_CFG_INTDIR expands
        # to a Visual Studio macro that will contain the right value, once Visual Studio is opened and
        # a build type is selected.
        set(STATIC_OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/${target}.dir/${CMAKE_CFG_INTDIR})
    else(CMAKE_CONFIGURATION_TYPES)
        # We have a single configuration generator. Use this directory structure:
        set(STATIC_OBJ_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${target}.dir)
    endif(CMAKE_CONFIGURATION_TYPES)

    # Now we know what directory the objects live in. Construct the actual list of objects:
    # from the sources. We cannot glob as these files do not exist yet
    get_property( target_sources TARGET ${target} PROPERTY SOURCES )
    
    foreach( sourcefile ${target_sources} )
        if(IS_ABSOLUTE ${sourcefile})
            # absolutes will appear in the top level object dir
            get_filename_component(source_name "${sourcefile}" NAME)
        else(IS_ABSOLUTE ${sourcefile})
            # relative will also be relative to the top level object dir
            set(source_name "${sourcefile}")
        endif(IS_ABSOLUTE ${sourcefile})
        list(APPEND ${outputObjectFiles} ${source_name}${CMAKE_C_OUTPUT_EXTENSION})
    endforeach( sourcefile )
    
    _ADD_DIR_PREFIX(${STATIC_OBJ_DIR}/ ${outputObjectFiles})
endmacro(_SUBPROJECT_OBJECT_FILES )



macro(_ADD_DIR_PREFIX prefix rootlist)
    set(outlist)
    foreach(root ${${rootlist}})
        if(IS_ABSOLUTE ${root})
            list(APPEND outlist ${root})
        else(IS_ABSOLUTE ${root})
            list(APPEND outlist ${prefix}${root})
        endif(IS_ABSOLUTE ${root})
    endforeach(root)
    set(${rootlist} ${outlist})
endmacro(_ADD_DIR_PREFIX)
