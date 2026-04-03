CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -D_GLIBCXX_USE_CXX11_ABI=0
LDFLAGS = -static-libgcc -static-libstdc++
TARGET = coflash.exe
SRCDIR = src
OBJDIR = obj

SOURCES = $(SRCDIR)/coflash.cpp \
          $(SRCDIR)/pyocd_cmd_builder.cpp

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean