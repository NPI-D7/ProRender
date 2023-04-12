# NEBS: Npi Easy Build System
#---------------------------------------------------------------------------------
CXX_VERSION:= 17
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/base_rules

PORTLIBS	:=	$(PORTLIBS_PATH)/3ds
export PATH := $(PORTLIBS_PATH)/3ds/bin:$(PATH)
CTRULIB	?=	$(DEVKITPRO)/libctru
#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# GRAPHICS is a list of directories containing graphics files
# GFXBUILD is the directory where converted graphics files will be placed
#   If set to $(BUILD), it will statically link in the converted
#   files as if they were data files.
#
# NO_SMDH: if set to anything, no SMDH file is generated.
# ROMFS is the directory which contains the RomFS, relative to the Makefile (Optional)
# APP_TITLE is the name of the app stored in the SMDH file (Optional)
# APP_DESCRIPTION is the description of the app stored in the SMDH file (Optional)
# APP_AUTHOR is the author of the app stored in the SMDH file (Optional)
# ICON is the filename of the icon (.png), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.png
#     - icon.png
#     - <libctru folder>/default_icon.png

#---------------------------------------------------------------------------------
# External tools
#---------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
MAKEROM 	?= C:/devkitpro/tools/bin/makerom.exe
BANNERTOOL 	?= C:/devkitpro/tools/bin/bannertool.exe
else
MAKEROM 	?= makerom
BANNERTOOL 	?= bannertool
endif

# If on a tagged commit, use the tag instead of the commit
ifneq ($(shell echo $(shell git tag -l --points-at HEAD) | head -c 1),)
GIT_VER := $(shell git tag -l --points-at HEAD)
else
GIT_VER := $(shell git rev-parse --short HEAD)
endif
#---------------------------------------------------------------------------------
# Version number
#---------------------------------------------------------------------------------
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION_MICRO := 0
#---------------------------------------------------------------------------------
TARGET		:=	ProRender
BUILD		:=	build
PRORENDER		:=  ../prorender
SOURCES		:=	src $(PRORENDER)
DATA		:=	data
INCLUDES	:=	src $(PRORENDER)
GRAPHICS	:=	gfx
ROMFS		:=	romfs
GFXBUILD	:=	$(ROMFS)
APP_AUTHOR	:=	NPI-D7
APP_DESCRIPTION := Simple Example of ProRender!
ICON		:=	app/icon.png
BNR_IMAGE	:=  app/banner.png
BNR_AUDIO	:=	app/BannerAudio.wav
RSF_FILE	:=	app/build-cia.rsf
#---------------------------------------------------------------------------------
# I Have my own Rules xd
#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-g -Wall -Wno-psabi -O2 -mword-relocations \
			-DV_STRING=\"$(GIT_VER)\" \
			-fomit-frame-pointer -ffunction-sections \
			$(ARCH) -D__BASE_CTR__ -DAPP_TITLE=\"$(TARGET)\"

CFLAGS	+=	$(INCLUDE) -D__3DS__ -D_GNU_SOURCE=1

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++$(CXX_VERSION)

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir code.map)

LIBS	:= -lstdc++ -lm -lcitro2d -lcitro3d -lctru
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(CTRULIB)
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT	:=	$(CURDIR)/$(TARGET)
export CODE_OUTPUT	:=	$(CURDIR)/build/code.elf
export TOPDIR	:=	$(CURDIR)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
GFXFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.t3s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
#---------------------------------------------------------------------------------
ifeq ($(GFXBUILD),$(BUILD))
#---------------------------------------------------------------------------------
export T3XFILES :=  $(GFXFILES:.t3s=.t3x)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
export ROMFS_T3XFILES	:=	$(patsubst %.t3s, $(GFXBUILD)/%.t3x, $(GFXFILES))
export T3XHFILES		:=	$(patsubst %.t3s, $(BUILD)/%.h, $(GFXFILES))
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
export OFILES_SOURCES 	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o)
export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES	:=	$(PICAFILES:.v.pica=_shbin.h) $(SHLISTFILES:.shlist=_shbin.h) \
			$(addsuffix .h,$(subst .,_,$(BINFILES)))
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export _3DSXDEPS	:=	$(if $(NO_SMDH),,(OUTPUT).smdh)
ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif
ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(OUTPUT).smdh
endif
ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif
.PHONY: all clean
#---------------------------------------------------------------------------------
all: $(BUILD) $(GFXBUILD) $(DEPSDIR) $(ROMFS_T3XFILES) $(T3XHFILES)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#------------------------------------------------------------------------------
clean:
	@echo "Cleaning..."
	@rm -fr -fr $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).cia $(GFXBUILD)/*.t3x
	@rm -rf $(BUILD)
#---------------------------------------------------------------------------------
send:
	@3dslink $(TARGET).3dsx
#---------------------------------------------------------------------------------
run:
	@flatpak run org.citra_emu.citra $(TARGET).3dsx
#---------------------------------------------------------------------------------
start_debug: $(BUILD) $(CODE_OUTPUT)
	@echo "Hello"
	@$(DEVKITARM)/bin/arm-none-eabi-gdb $(CODE_OUTPUT)
#---------------------------------------------------------------------------------
cia: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile cia
#---------------------------------------------------------------------------------
3dsx: $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile 3dsx
#---------------------------------------------------------------------------------
$(GFXBUILD)/%.t3x	$(BUILD)/%.h	:	%.t3s
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(DEVKITPRO)/tools/bin/tex3ds -i $< -H $(BUILD)/$*.h -d $(DEPSDIR)/$*.d -o $(GFXBUILD)/$*.t3x
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(CODE_OUTPUT) $(OUTPUT).3dsx #$(OUTPUT).cia
	@echo "--------------------------------------------"
	@echo "Build"
	@echo "--------------------------------------------"
	@echo "Target: $(TARGET)"
	@echo "CXX Version: $(CXX_VERSION)"
	@echo "--------------------------------------------"

$(CODE_OUTPUT)	:	$(OFILES)

$(OUTPUT).3dsx : $(CODE_OUTPUT) $(OUTPUT).smdh

$(OUTPUT).cia	:	$(CODE_OUTPUT) $(OUTPUT).smdh
	@$(BANNERTOOL) makebanner -i "../$(BNR_IMAGE)" -a "../$(BNR_AUDIO)" -o "../app/banner.bin"
	@$(BANNERTOOL) makesmdh -i "$(APP_ICON)" -s "$(TARGET)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -o "../app/icon.bin"
	@$(MAKEROM) -f cia -target t -exefslogo -o "../$(TARGET).cia" -elf "$(CODE_OUTPUT)" -rsf "../$(RSF_FILE)" -banner "../app/banner.bin" -icon "../app/icon.bin" -logo "../app/logo.lz11" -DAPP_ROMFS="$(TOPDIR)/$(ROMFS)" -major $(VERSION_MAJOR) -minor $(VERSION_MINOR) -micro $(VERSION_MICRO) -DAPP_VERSION_MAJOR="$(VERSION_MAJOR)"
	@echo "Build $$(basename $(OUTPUT)).cia"
#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
#---------------------------------------------------------------------------------
.PRECIOUS	:	%.t3x
#---------------------------------------------------------------------------------
%.t3x.o	%_t3x.h :	%.t3x
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $*.shbin)
	$(eval DEPSFILE := $(DEPSDIR)/$*.shbin.d)
	echo "$(CURBIN).o: $< $1" > $(DEPSFILE)
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $*.shbin.o
endef

%.shbin.o %_shbin.h : %.v.pica %.g.pica
	@echo -e "\e[33mCompiling Shader:\e[0m \e[36m$(notdir $^)\e[0m"
	@$(call shader-as,$^)

%.shbin.o %_shbin.h : %.v.pica
	@echo -e "\e[33mCompiling Shader:\e[0m \e[36m$(notdir $<)\e[0m"
	@$(call shader-as,$<)

%.shbin.o %_shbin.h : %.shlist
	@echo -e "\e[33mCompiling Shader:\e[0m \e[36m$(notdir $<)\e[0m"
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)$(file)))
#---------------------------------------------------------------------------------
%.t3x	%.h	:	%.t3s
#---------------------------------------------------------------------------------
	@echo -e "\e[32mGenerating T3X of \e[0m \e[36m$(notdir $<)\e[0m"
	@tex3ds -i $< -H $*.h -d $*.d -o $*.t3x

-include $(DEPSDIR)/*.d
#---------------------------------------------------------------------------------
%.smdh: $(APP_ICON) $(MAKEFILE_LIST)
	@$(SILENTCMD)smdhtool --create "$(TARGET)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(APP_ICON) $(OUTPUT).smdh
	$(SILENTMSG) built $(notdir $(TARGET).smdh)
#---------------------------------------------------------------------------------
%.3dsx: %.elf
	@$(SILENTCMD)3dsxtool $(CODE_OUTPUT) ../$(TARGET).3dsx $(_3DSXFLAGS)
	$(SILENTMSG) built $(notdir $@)
#---------------------------------------------------------------------------------
%.elf:
#	@$(SILENTMSG) linking $(notdir $(CODE_OUTPUT))
	@$(SILENTCMD)$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $(CODE_OUTPUT)
	@$(SILENTCMD)$(NM) -CSn $(CODE_OUTPUT) > $(notdir code.lst)
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------