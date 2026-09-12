CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR := .holyc-build
HOLYC := $(BUILD_DIR)/bin/holyc
TRANSLATOR_SOURCES := compiler/translator.cpp

.PHONY: all compiler examples test clean

all: examples

compiler: $(HOLYC)

$(HOLYC): compiler/main.cpp compiler/translator.hpp $(TRANSLATOR_SOURCES)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) compiler/main.cpp $(TRANSLATOR_SOURCES) -o $@

examples: compiler
	$(HOLYC) examples/Hello.HC
	$(HOLYC) examples/Graphics.HC
	$(HOLYC) tests/Include.HC -I tests/include

$(BUILD_DIR)/translator_test: tests/translator_test.cpp compiler/translator.hpp $(TRANSLATOR_SOURCES)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) tests/translator_test.cpp $(TRANSLATOR_SOURCES) -o $@

test: examples $(BUILD_DIR)/translator_test
	$(BUILD_DIR)/translator_test
	$(BUILD_DIR)/Hello
	$(BUILD_DIR)/Include
	SDL_VIDEODRIVER=dummy $(BUILD_DIR)/Graphics

clean:
	rm -rf $(BUILD_DIR)
