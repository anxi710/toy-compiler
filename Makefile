# Flags
CXXFLAGS := -Wall -Wno-register -std=c++26

# Debug flags
DEBUG ?= 1
ifeq ($(DEBUG), 0)
CXXFLAGS += -fno-rtti -O2
else
CXXFLAGS += -g -O0 -DDEBUG
endif

# Assemble Debug
VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
CXXFLAGS += -DVERBOSE
endif

# Compilers
CXX := clang++

# Directories
TARGET_EXEC := toy_compiler
SRC_DIR := src
BUILD_DIR := build

# Source files
SRCS := $(filter-out $(SRC_DIR)/ast/accept.cpp, \
         $(shell find $(SRC_DIR) -name "*.cpp")) \
         $(SRC_DIR)/ast/accept.cpp

# Target files
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Header directories
INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I, $(INC_DIRS))

# Dependencies
DEPS := $(OBJS:.o=.d)
CPPFLAGS = $(INC_FLAGS) -MMD -MP

# Main target
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -lpthread -ldl -o $@

# C++ source
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ast/accept.o: $(SRC_DIR)/ast/accept.cpp

$(SRC_DIR)/ast/accept.cpp: $(SRC_DIR)/generate_accept.pl
	perl $< > $@

.PHONY: all verbose bear clean clean-all

all:
	$(MAKE) -j

verbose:
	@VERBOSE=1 $(MAKE) all

bear:
	@$(MAKE) clean
	bear -- $(MAKE) all

clean:
	-rm -rf $(BUILD_DIR) *.ir *.code

clean-all:
	-rm -rf $(BUILD_DIR) compile_commands.json *.ir *.code

-include $(DEPS)
