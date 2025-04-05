FUNC := g++
copt := -c 
OBJ_DIR := ./bin/
FLAGS := -O3 -lm -g -Werror 
SIMD_FLAGS := -I ./version2-2.02.01 -mavx2 -mfma -ffp-contract=off
RAYLIB_FLAGS := -lraylib -lGL -lpthread -ldl -lrt -lX11
MAC_RAY_FLAGS := -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
MAC_FLAGS := -std=c++17

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(CPP_FILES:.cpp=.obj)))

all:
	$(FUNC) ./main.cpp -o ./main.exe $(FLAGS)

simd:
	$(FUNC) ./main_simd.cpp -o ./main.exe $(FLAGS) $(SIMD_FLAGS)

ray:
	$(FUNC) ./main_ray.cpp -o ./main.exe $(FLAGS) $(RAYLIB_FLAGS)

mac-ray:
	$(FUNC) ./main_ray.cpp -o ./main.exe $(MAC_FLAGS) $(FLAGS) $(MAC_RAY_FLAGS)

clean:
	rm -f ./*.exe
