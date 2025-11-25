#
# Makefile for TssShield Driver
# This Makefile is intended for use with nmake from the
# x64 Native Tools Command Prompt for VS 2022.
#
# Note: You may need to adjust the WDK_PATH and SDK_VERSION
#       variables to match your system's installation.
#
!include <win32.mak>

TARGETNAME = TssShield
TARGET_EXT = sys
OUTDIR = x64\Release

# Base path for WDK and SDK installations
# The user specified SDK is on D:, assuming WDK is in a similar location.
KIT_PATH = D:\Windows Kits\10
SDK_VERSION = 10.0.26100.0

# WDK/SDK paths
WDK_INC_PATH = $(KIT_PATH)\Include\$(SDK_VERSION)\km
SDK_INC_PATH = $(KIT_PATH)\Include\$(SDK_VERSION)\ucrt
SHARED_INC_PATH = $(KIT_PATH)\Include\$(SDK_VERSION)\shared
WDK_LIB_PATH = $(KIT_PATH)\Lib\$(SDK_VERSION)\km\x64

# Compiler and Linker flags
CFLAGS = /nologo /W4 /WX /Od /D_AMD64_ /D_KERNEL_MODE /kernel /GS- /GR- /EHsc \
         -I"$(WDK_INC_PATH)" \
         -I"$(SDK_INC_PATH)" \
         -I"$(SHARED_INC_PATH)" \
         -I"..\Common"

LFLAGS = /nologo /driver /subsystem:native /entry:DriverEntry /out:$(OUTDIR)\$(TARGETNAME).$(TARGET_EXT) \
         /libpath:"$(WDK_LIB_PATH)"

# Libraries to link
LIBS = ntoskrnl.lib fltMgr.lib

# Target and objects
TARGET = $(OUTDIR)\$(TARGETNAME).$(TARGET_EXT)
OBJS = $(OUTDIR)\TssShield.obj

# Build rules
all: $(OUTDIR) $(TARGET)

$(OUTDIR):
    if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

$(OBJS): TssShield.c
    $(cc) $(CFLAGS) /c TssShield.c /Fo"$@"

$(TARGET): $(OBJS)
    $(link) $(LFLAGS) $(LIBS) $(OBJS)

clean:
    if exist "$(OUTDIR)" rmdir /s /q "$(OUTDIR)"
