CXX = g++
BUILD = build
MDIIP_SRC = MDIIP/github/MDIIP-TT-master

OPTIMIZE_SRC = optimize.cpp optimize_utils.cpp constraints.cpp handlers.cpp instance.cpp
VIEWER_SRC = generate_instance.cpp instance.cpp

TARGETS = $(BUILD)/optimize $(BUILD)/mostrar_instancia

# MDIIP esta en .gitignore: si no existe la carpeta, se omite
ifneq ($(wildcard $(MDIIP_SRC)/main.cpp),)
TARGETS += $(BUILD)/MDIIP
else
$(info MDIIP no encontrado en $(MDIIP_SRC), se omite)
endif

all: $(TARGETS)

$(BUILD)/optimize: $(OPTIMIZE_SRC) $(wildcard *.h) | $(BUILD)
	$(CXX) -std=c++17 -Wall -Wextra -O2 $(OPTIMIZE_SRC) -o $@

$(BUILD)/mostrar_instancia: $(VIEWER_SRC) instance.h | $(BUILD)
	$(CXX) -std=c++17 -Wall -Wextra -O2 $(VIEWER_SRC) -o $@

# Mismas flags que el Makefile original de MDIIP
$(BUILD)/MDIIP: $(wildcard $(MDIIP_SRC)/*.cpp $(MDIIP_SRC)/*.h) | $(BUILD)
	$(CXX) -std=c++1y -fmax-errors=1 -O0 -g -ggdb $(wildcard $(MDIIP_SRC)/*.cpp) -o $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
