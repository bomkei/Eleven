CXX				:= clang++
CC				:= clang

TARGET		:= eleven.out
OBJDIR		:= build

CFILES		:= $(wildcard *.c)
CXXFILES	:= $(wildcard *.cpp)
OFILES		:= \
		$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(CXXFILES))) \
		$(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(CFILES)))

CFLAGS		?= -O2 -Wall
CXXFLAGS	:= $(CFLAGS) -std=gnu++20 -Wno-psabi
LDFLAGS		:= -Wl,--gc-sections

all: $(TARGET)

debug:
	@$(MAKE) --no-print-directory CFLAGS='-O2 -Wall -Wextra -g'

clean:
	@rm -rf $(OBJDIR) $(TARGET)

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
	@$(CXX) $(LDFLAGS) $^ -o $@
