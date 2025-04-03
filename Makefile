FUNC := g++
copt := -c 
OBJ_DIR := ./bin/
FLAGS := -O3 -lm -g -Werror 
SIMD_FLAGS := -I ./version2-2.02.01 -mavx2 -mfma -ffp-contract=off

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(CPP_FILES:.cpp=.obj)))

all:
	$(FUNC) ./main.cpp -o ./main.exe $(FLAGS)

simd:
	$(FUNC) ./main.cpp -o ./main.exe $(FLAGS) $(SIMD_FLAGS)

clean:
	rm -f ./*.exe
