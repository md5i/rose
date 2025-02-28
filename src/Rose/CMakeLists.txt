# This header file is only for things in the ::Rose namespace. If
# you're adding something that's not in ::Rose you're doing it wrong!

# Namespaces within ::Rose (see top comment). Every namespace will
# have a directory listed here, and every directory here is a
# namespace.
add_subdirectory(BinaryAnalysis)
add_subdirectory(Color)
add_subdirectory(CommandLine)
add_subdirectory(Diagnostics)
add_subdirectory(FileSystem)
add_subdirectory(StringUtility)
add_subdirectory(AST)

# needed to properly define ROSE_UTIL_API
add_definitions(-DROSE_UTIL_EXPORTS)

# .C files for the ::Rose namespace (see top comment)
add_library(roseNamespace OBJECT
  FormattedTable.C
  Initialize.C
  Progress.C
  SourceLocation.C)

add_dependencies(roseNamespace rosetta_generated)

# Public header files for the ::Rose namespace (see top comment)
install(FILES
  BinaryAnalysis.h
  BitFlags.h
  BitOps.h
  Color.h
  CommandLine.h
  Constants.h
  Diagnostics.h
  Exception.h
  FileSystem.h
  FormatRestorer.h
  FormattedTable.h
  GraphUtility.h
  Initialize.h
  Progress.h
  RecursionCounter.h
  SourceLocation.h
  StringUtility.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose)
