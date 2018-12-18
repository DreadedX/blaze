from distutils.sysconfig import get_python_inc
import platform
import os
import subprocess
import ycm_core

DIR_OF_THIS_SCRIPT = os.path.abspath( os.path.dirname( __file__ ) )
SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', '.m', '.mm' ]

flags = [
    '-x',
    'c++',
    '-DDEBUG',
    '-Imodules/blaze/include',
    '-Imodules/crypto/include',
    '-Imodules/flame/include',
    '-Imodules/generated/include',
    '-Imodules/iohelper/iohelper/include',
    '-Imodules/lang/include',
    '-Imodules/logger/logger/include',
    '-Imodules/lua-bind/include',
    '-Ithird_party/bigint',
    '-Ithird_party/lua',
    '-Ithird_party/headers',
    '-Ithird_party/sol2',
    '-Ithird_party/glm',
    '-Ithird_party/vulkan-headers/include',
    '-Ithird_party/glfw/include',
    '-Imodules/logger/third_party/fmt',
    '-Igame/include',
    '-Iplugin/lang',
    '-Iplugin/packager',
    '-Wall',
    '-Wextra',
    '-std=c++17',
]


def IsHeaderFile( filename ):
  extension = os.path.splitext( filename )[ 1 ]
  return extension in [ '.h', '.hxx', '.hpp', '.hh' ]


def FindCorrespondingSourceFile( filename ):
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + "../src/" + extension
      if os.path.exists( replacement_file ):
        return replacement_file
  return filename


def Settings( **kwargs ):
  if kwargs[ 'language' ] == 'cfamily':
    # If the file is a header, try to find the corresponding source file and
    # retrieve its flags from the compilation database if using one. This is
    # necessary since compilation databases don't have entries for header files.
    # In addition, use this source file as the translation unit. This makes it
    # possible to jump from a declaration in the header file to its definition
    # in the corresponding source file.
    filename = FindCorrespondingSourceFile( kwargs[ 'filename' ] )

    return {
        'flags': flags,
        'include_paths_relative_to_dir': DIR_OF_THIS_SCRIPT,
        'override_filename': filename
    }
  return {}


def GetStandardLibraryIndexInSysPath( sys_path ):
  for path in sys_path:
    if os.path.isfile( os.path.join( path, 'os.py' ) ):
      return sys_path.index( path )
  raise RuntimeError( 'Could not find standard library path in Python path.' )


def PythonSysPath( **kwargs ):
  sys_path = kwargs[ 'sys_path' ]
  return sys_path

