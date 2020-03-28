SHELL = /bin/sh

PKG_CONFIG ?= pkg-config
GUI_LIBS += lv2 x11 cairo
LV2_LIBS += lv2 sndfile
ifneq ($(shell $(PKG_CONFIG) --exists fontconfig || echo no), no)
  GUI_LIBS += fontconfig
  GUIPPFLAGS += -DPKG_HAVE_FONTCONFIG
endif

CXX ?= g++
INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= $(INSTALL) -m644
STRIP ?= strip

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

CPPFLAGS += -DPIC
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
LDFLAGS += -shared -Wl,-z,relro,-z,now
STRIPFLAGS += -s --strip-program=$(STRIP)

DSPFLAGS =
GUIPPFLAGS += -DPUGL_HAVE_CAIRO

DSPFLAGS += `$(PKG_CONFIG) --cflags --libs $(LV2_LIBS)`
GUIFLAGS += `$(PKG_CONFIG) --cflags --libs $(GUI_LIBS)`

BUNDLE = BHarvestr.lv2
DSP = BHarvestr
DSP_SRC = ./src/BHarvestr.cpp
GUI = BHarvestr_GUI
GUI_SRC = ./src/BHarvestrGUI.cpp
OBJ_EXT = .so
DSP_OBJ = $(DSP)$(OBJ_EXT)
GUI_OBJ = $(GUI)$(OBJ_EXT)
B_OBJECTS = $(addprefix $(BUNDLE)/, $(DSP_OBJ) $(GUI_OBJ))

ROOTFILES = \
	manifest.ttl \
	BHarvestr.ttl \
	sine440.wav \
	LICENSE

INCFILES = \
	inc/surface.png \
	inc/Global_settings.png \
	inc/Patch_settings.png \
	inc/LFO1.png \
	inc/LFO2.png \
	inc/LFO3.png \
	inc/LFO4.png \
	inc/Seq1.png \
	inc/Seq2.png \
	inc/Seq3.png \
	inc/Seq4.png \
	inc/Rnd1.png \
	inc/Rnd2.png \
	inc/Rnd3.png \
	inc/Rnd4.png \
	inc/Env1.png \
	inc/Env2.png \
	inc/Env3.png \
	inc/Env4.png

B_FILES = $(addprefix $(BUNDLE)/, $(ROOTFILES) $(INCFILES))

GUI_INCL = \
	src/ValueSelect.cpp \
	src/ShapeWidget.cpp \
	src/BWidgets/FileChooser.cpp \
	src/BWidgets/HPianoRoll.cpp \
	src/BWidgets/PianoWidget.cpp \
	src/BWidgets/ImageIcon.cpp \
	src/BWidgets/Icon.cpp \
	src/BWidgets/PopupListBox.cpp \
	src/BWidgets/ListBox.cpp \
	src/BWidgets/ChoiceBox.cpp \
	src/BWidgets/ItemBox.cpp \
	src/BWidgets/Text.cpp \
	src/BWidgets/UpButton.cpp \
	src/BWidgets/DownButton.cpp \
	src/BWidgets/TextButton.cpp \
	src/BWidgets/ToggleButton.cpp \
	src/BWidgets/Button.cpp \
	src/BWidgets/VScale.cpp \
	src/BWidgets/HScale.cpp \
	src/BWidgets/DrawingSurface.cpp \
	src/BWidgets/DialValue.cpp \
	src/BWidgets/Dial.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BItems.cpp \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/pugl_x11_cairo.c \
	src/BWidgets/pugl/pugl_x11.c \
	src/BUtilities/to_string.cpp

ifeq ($(shell $(PKG_CONFIG) --exists sndfile || echo no), no)
  $(error libsndfile not found. Please install libsndfile first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists lv2 || echo no), no)
  $(error LV2 not found. Please install LV2 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists x11 || echo no), no)
  $(error X11 not found. Please install X11 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists cairo || echo no), no)
  $(error Cairo not found. Please install cairo first.)
endif

$(BUNDLE): clean $(DSP_OBJ) $(GUI_OBJ)
	@cp $(ROOTFILES) $(BUNDLE)
	@mkdir -p $(BUNDLE)/inc
	@cp $(INCFILES) $(BUNDLE)/inc

all: $(BUNDLE)

$(DSP_OBJ): $(DSP_SRC)
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)
	@echo \ done.

$(GUI_OBJ): $(GUI_SRC)
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ $(CPPFLAGS) $(GUIPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

install-strip:
	@echo -n "Install (stripped)" $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@$(INSTALL) -d $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_PROGRAM) -m755 $(STRIPFLAGS) $(B_OBJECTS) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@$(INSTALL_DATA) $(B_FILES) $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

uninstall:
	@echo -n Uninstall $(BUNDLE)...
	@rm -f $(addprefix $(DESTDIR)$(LV2DIR)/$(BUNDLE)/, $(FILES))
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(GUI_OBJ)
	@rm -f $(DESTDIR)$(LV2DIR)/$(BUNDLE)/$(DSP_OBJ)
	-@rmdir $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@echo \ done.

clean:
	@rm -rf $(BUNDLE)

.PHONY: all install install-strip uninstall clean
