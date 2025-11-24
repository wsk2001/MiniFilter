!include <win32.mak>

TARGETNAME = TssShield.exe
OUTDIR = x64\Release

OBJS = $(OUTDIR)\TssShield.obj

LIBS = fltlib.lib psapi.lib user32.lib

all: $(OUTDIR) $(OUTDIR)\$(TARGETNAME)

$(OUTDIR):
    if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

$(OUTDIR)\TssShield.obj: TssShield.cpp
    $(cc) $(cflags) $(cvarsdll) /EHsc /I"include" /c TssShield.cpp /Fo"$(OUTDIR)\\"

$(OUTDIR)\$(TARGETNAME): $(OBJS)
    $(link) $(ldebug) $(conflags) -out:$(OUTDIR)\$(TARGETNAME) $(OBJS) $(LIBS)

clean:
    if exist "$(OUTDIR)" rmdir /s /q "$(OUTDIR)"
