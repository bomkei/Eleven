CXX				:= g++-11
CC				:= gcc-11

TARGET		:= eleven
OBJDIR		:= build

CFILES		:= $(wildcard *.c)
CXXFILES	:= $(wildcard *.cpp)
OFILES		:= \
		$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(CXXFILES))) \
		$(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(CFILES)))

CFLAGS		?= -O2 -Wall
CXXFLAGS	:= $(CFLAGS) -std=gnu++23 -Wno-psabi
LDFLAGS		:= -Wl,--gc-sections

ifeq ($(OS),Windows_NT)
  EXT = exe
else
  EXT = out
endif

all: $(TARGET)

debug:
	@$(MAKE) --no-print-directory CFLAGS='-g -O2 -Wall -Wextra'

clean:
	@rm -rf $(OBJDIR) $(TARGET).$(EXT)

re: clean all

$(OBJDIR)/%.o: %.c $(wildcard *.h)
	@echo $<
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp $(wildcard *.h)
	@echo $<
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OFILES)
	@echo linking...
	@$(CXX) $(LDFLAGS) $^ -o $@.$(EXT)
