ARDUINO_CLI ?= arduino-cli
FQBN ?= arduino:avr:leonardo
SKETCH_DIR := $(CURDIR)
BUILD ?= stable
BUILD_DIR ?= $(SKETCH_DIR)/build/$(BUILD)
DIST_DIR ?= $(SKETCH_DIR)/dist
ARDUINO_DATA_DIR ?= $(SKETCH_DIR)/.arduino
PORT ?=
TARGET ?= fx
OPEN ?= xdg-open
RETROARCH ?= retroarch
ARDENS_LIBRETRO_CORE ?=
RETROARCH_CONFIG ?= $(SKETCH_DIR)/config/retroarch/arduboy-clean.cfg
FX_CART_DIR ?= $(DIST_DIR)/fx-cart
FX_CATEGORY ?= 99-Development
FX_GAME ?= 02-All-Mens-Morris
FX_BANNER ?= assets/fx/banner.png

ELF := $(BUILD_DIR)/all-mens-morris.ino.elf
HEX := $(BUILD_DIR)/all-mens-morris.ino.hex
AVR_SIZE ?= avr-size
AVR_NM ?= avr-nm

ifeq ($(BUILD),debug)
ARDUINO_BUILD_FLAGS := --build-property compiler.cpp.extra_flags="-DALL_MENS_MORRIS_DEBUG=1"
else ifeq ($(BUILD),stable)
ARDUINO_BUILD_FLAGS :=
else
$(error BUILD must be stable or debug)
endif

export ARDUINO_DIRECTORIES_DATA := $(ARDUINO_DATA_DIR)
export ARDUINO_DIRECTORIES_USER := $(SKETCH_DIR)/.arduino-sketchbook

.PHONY: all setup compile compile-debug upload upload-sketch clean hex size size-debug symbols symbols-debug check sim cloud libretro libretro-debug fx-entry

all: compile

setup:
	$(ARDUINO_CLI) core update-index
	$(ARDUINO_CLI) core install arduino:avr
	$(ARDUINO_CLI) lib install Arduboy2 Arduboy-TinyFont ArduboyTones

compile:
	$(ARDUINO_CLI) compile --fqbn $(FQBN) $(ARDUINO_BUILD_FLAGS) --build-path $(BUILD_DIR) $(SKETCH_DIR)

compile-debug:
	$(MAKE) compile BUILD=debug

upload:
ifeq ($(TARGET),fx)
	$(error TARGET=fx protects your Arduboy FX catalog. Use make fx-entry, then merge that entry into a backed-up flashcart image)
endif
	$(MAKE) upload-sketch TARGET=$(TARGET) PORT=$(PORT) CONFIRM_OVERWRITE=$(CONFIRM_OVERWRITE)

upload-sketch:
ifneq ($(CONFIRM_OVERWRITE),1)
	$(error Refusing to overwrite the main sketch. Re-run with CONFIRM_OVERWRITE=1 after confirming this is not an FX catalog update)
endif
ifndef PORT
	$(error Set PORT=/dev/ttyACM0 or another Arduboy serial device)
endif
	$(ARDUINO_CLI) upload --fqbn $(FQBN) --port $(PORT) $(SKETCH_DIR)

hex: compile
	@printf '%s\n' "$(HEX)"

size: compile
	$(AVR_SIZE) -C --mcu=atmega32u4 "$(ELF)"
	$(AVR_SIZE) -A "$(ELF)"

size-debug:
	$(MAKE) size BUILD=debug

symbols: compile
	$(AVR_NM) --print-size --size-sort --radix=d "$(ELF)"

symbols-debug:
	$(MAKE) symbols BUILD=debug

sim: compile
	@printf '%s\n' "Open this HEX in Arduboy Cloud, Ardens, or another Arduboy emulator:"
	@printf '%s\n' "$(HEX)"

cloud: compile
	@printf '%s\n' "Opening Arduboy Cloud. Load this HEX in the emulator:"
	@printf '%s\n' "$(HEX)"
	$(OPEN) "https://cloud.arduboy.com/"

libretro: compile
ifndef ARDENS_LIBRETRO_CORE
	$(error Enter nix develop first, or set ARDENS_LIBRETRO_CORE=/path/to/ardens_libretro.so)
endif
	$(RETROARCH) --appendconfig="$(RETROARCH_CONFIG)" --set-shader="" -L "$(ARDENS_LIBRETRO_CORE)" "$(HEX)"

libretro-debug:
	$(MAKE) libretro BUILD=debug

fx-entry: compile
	install -D "$(HEX)" "$(FX_CART_DIR)/$(FX_CATEGORY)/$(FX_GAME).hex"
	install -D "$(SKETCH_DIR)/$(FX_BANNER)" "$(FX_CART_DIR)/$(FX_CATEGORY)/$(FX_GAME).png"
	@printf '%s\n' "FX catalog entry prepared:"
	@printf '%s\n' "$(FX_CART_DIR)/$(FX_CATEGORY)/$(FX_GAME).hex"
	@printf '%s\n' "$(FX_CART_DIR)/$(FX_CATEGORY)/$(FX_GAME).png"
	@printf '%s\n' "Merge this into a backup of your existing FX cart before writing."

check: compile

clean:
	rm -rf "$(BUILD_DIR)" "$(DIST_DIR)"
