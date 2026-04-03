CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
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
	if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	if exist $(TARGET) del $(TARGET)

.PHONY: all clean