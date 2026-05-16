# Default generator (can be 'make' or 'ninja')
GENERATOR ?= ninja

BUILD_DIR = build

# Determine the CMake generator flag based on the variable
ifeq ($(GENERATOR), ninja)
    CMAKE_GEN_FLAG := -G Ninja
else
    CMAKE_GEN_FLAG := -G "Unix Makefiles"
endif

.PHONY: all
all: build

# Configure step: Dynamically passes the chosen generator to CMake
.PHONY: config
config:
	@echo "Configuring with CMake generator: $(GENERATOR)..."
	@cmake $(CMAKE_GEN_FLAG) -B $(BUILD_DIR) -S .

# Build step: cmake automatically knows whether to call 'make' or 'ninja' inside the build folder
.PHONY: build
build: config
	@cmake --build $(BUILD_DIR)

.PHONY: run_mdp
run_mdp: build
	@./$(BUILD_DIR)/mdp_program\

.PHONY: run_ql
run_ql: build
	@./$(BUILD_DIR)/qlearning_program

.PHONY: clean
clean:
	@echo "Cleaning up build directory..."
	@rm -rf $(BUILD_DIR)

.PHONY: format
format:
	@echo "Formatting source files..."
	@clang-format -i $(shell find . -name "*.cpp" -o -name "*.h" ! -path "./build/*")
	@echo "Formatting complete."
