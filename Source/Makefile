
OPTIMIZE := -O3
CXXFLAGS := $(shell sdl-config --cflags) -DHAVE_MIKMOD $(OPTIMIZE)
LIBS   := $(shell sdl-config --libs) -lSDL_image -lmikmod

TARGET := CandyCrisis

OBJECTS :=	\
	blitter.o \
	control.o \
	font.o \
	gameticks.o \
	graphics.o \
	graymonitor.o \
	grays.o \
	gworld.o \
	hiscore.o \
	keyselect.o \
	level.o \
	main.o \
	midi.o \
	moving.o \
	MTypes.o \
	next.o \
	opponent.o \
	pause.o \
	players.o \
	prefs.o \
	random.o \
	score.o \
	SDLU.o \
	soundfx.o \
	tutorial.o \
	tweak.o \
	victory.o \
	zap.o

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) $(TARGET)
