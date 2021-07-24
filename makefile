TARGET		:= happybasic
OBJDIR		:= build

CXXFILES	:= $(wildcard *.cpp)
OFILES		:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(CXXFILES)))

CXXFLAGS	?= -O1 -std=gnu++2a -Wno-psabi
LDFLAGS		:= -Wl,--gc-sections

ifeq ($(OS),Windows_NT)
  EXT = exe
else
  EXT = out
endif

all: $(TARGET)

debug:
	@$(MAKE) --no-print-directory CXXFLAGS='-std=gnu++2a -Wno-psabi -g'

clean:
	@rm -rf $(OBJDIR) $(TARGET).$(EXT)

re: clean all

$(OBJDIR)/%.o: %.cpp $(wildcard *.h)
	@echo $<
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@g++ $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OFILES)
	@echo linking...
	@g++ $(LDFLAGS) $^ -o $@.$(EXT)
