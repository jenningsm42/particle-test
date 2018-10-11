LIBS=-lsfml-graphics -lsfml-window -lsfml-system
CXX := g++
SRC_DIR := ./particles
OBJ_DIR := ./obj
OUT_DIR := ./build
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) \

all: simulator

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c $< -o $@

simulator: $(OBJ_FILES)
	@echo '** Building the simulator'
	@mkdir -p $(OUT_DIR)
	$(CXX) -o $(OUT_DIR)/$@ $^ $(LIBS)

clean:
	@echo '** Cleaning project'
	$(RM) -r $(OUT_DIR)/* $(OBJ_DIR)/*

.PHONY: all simulator clean
