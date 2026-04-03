CXX = g++
CXXFLAGS = -std=c++11 -O0 -g3 -Wall -c -fmessage-length=0 
TARGET = coflash.exe
SRCDIR = src
OBJDIR = obj

SOURCES = $(SRCDIR)/coflash.cpp \
          $(SRCDIR)/pyocd_cmd_builder.cpp

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean