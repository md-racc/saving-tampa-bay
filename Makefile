CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

SRCDIR   := src
BUILDDIR := bin
TARGET   := $(BUILDDIR)/router

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
