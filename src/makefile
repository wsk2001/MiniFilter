#
# MiniFilter makefile for Windows Driver Kit (WDK)
#

# This is a basic makefile structure for building with WDK
# For actual building, use Visual Studio with WDK or msbuild

TARGETNAME=MiniFilter
TARGETTYPE=DRIVER

# Target architecture
# Can be x86, x64, ARM, ARM64
TARGETARCH=x64

# Driver type
DRIVERTYPE=WDM

# Includes
INCLUDES=$(DDK_INC_PATH)

# Sources
SOURCES=MiniFilter.c

# Libraries
TARGETLIBS=$(DDK_LIB_PATH)\fltmgr.lib

# Build options
MSC_WARNING_LEVEL=/W4 /WX

# Precompiled headers
# PRECOMPILED_INCLUDE=MiniFilter.h

# Debug/Release specific options
!IF "$(DDKBUILDENV)"=="chk"
# Debug build
MSC_OPTIMIZATION=/Od /Zi
!ELSE
# Release build
MSC_OPTIMIZATION=/O2
!ENDIF
