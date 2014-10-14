FIRMWARE_NAME := SE-506CB_TS01
FIRMWARE_URL  := "http://www.tsstodd.com/TotalLib/popup/Download.asp?path=fwdownload&lang=eng&fname=SE-506CB_TS01.exe"

PATCH_ADDR := 0xC9600
PATCH_BIN := bin/patch.bin
PATCH_SRC := patch.s

FIRMWARE_BIN := bin/$(FIRMWARE_NAME).bin
BACKDOOR_BIN := bin/firmware-with-backdoor.bin
REMOTE_PYLIB := remote.so

# External tools we need
GCC_ARM      := arm-none-eabi-gcc
OBJCOPY_ARM  := arm-none-eabi-objcopy
OBJDUMP_ARM  := arm-none-eabi-objdump
HEXDUMP_TOOL := xxd
UNZIP_TOOL   := unzip
CURL_TOOL    := curl
SHASUM_TOOL  := shasum
PYTHON       := python

# Coastermelt tools
FLASHER_DIR   := ../flasher
CHECKSUM_TOOL := $(PYTHON) $(FLASHER_DIR)/checksum.py
FLASHER_TOOL  := $(FLASHER_DIR)/mtflash


# Compile the backdoored firmware image, from the official TS01 firmware
all: $(BACKDOOR_BIN) $(REMOTE_PYLIB)

# FLASH THE MODIFIED FIRMWARE
# This is a super crazy thing to do, could brick your device, you've been warned!
flash: $(FLASHER_TOOL) $(BACKDOOR_BIN)
	$(FLASHER_TOOL) (BACKDOOR_BIN)

# Disassemble a chunk of code around the patched location in the firmware
disassemble: $(BACKDOOR_BIN) $(PATCH_BIN)
	( \
	  $(HEXDUMP_TOOL) $(PATCH_BIN) ;\
	  $(OBJDUMP_ARM) -D -b binary -m arm7tdmi -M force-thumb --start-address=$(PATCH_ADDR) $(BACKDOOR_BIN) | head -n 200 \
	) | less


# Build the "remote" python module with distutils
$(REMOTE_PYLIB): remote.cpp
	$(PYTHON) setup.py build_ext --inplace

# We can compile the flasher if necessary
$(FLASHER_TOOL):
	make -C $(FLASHER_DIR)

# Download the original firmware from TSST's web site (slow, only happens once)
bin/$(FIRMWARE_NAME).exe: bin/firmware.shasum
	echo --- Downloading original firmware image
	$(CURL_TOOL) $(FIRMWARE_URL) > $@

# The EXE is a self-extracting ZIP actually. Grab the .BIN file and check its shasum
$(FIRMWARE_BIN): bin/$(FIRMWARE_NAME).exe
	cd bin; $(UNZIP_TOOL) -o $(FIRMWARE_NAME).exe $(FIRMWARE_NAME).bin || touch $(FIRMWARE_NAME).bin
	cd bin; $(SHASUM_TOOL) -c firmware.shasum

# Insert our binary patch at PATCH_ADDR in the binary, and patch the checksum
$(BACKDOOR_BIN): $(PATCH_BIN) $(FIRMWARE_BIN)
	cp $(FIRMWARE_BIN) $(BACKDOOR_BIN)
	(dd if=$(FIRMWARE_BIN) bs=1 count=$(PATCH_ADDR); cat $(PATCH_BIN)) | dd of=$(BACKDOOR_BIN) conv=notrunc
	$(CHECKSUM_TOOL) --fix $@

# Use GCC_ARM to assemble the patch into a plain Thumb binary
$(PATCH_BIN): $(PATCH_SRC)
	$(GCC_ARM) -nostdlib -nostdinc -o $@ $<
	$(OBJCOPY_ARM) $@ -O binary


clean:
	rm -f $(FIRMWARE_BIN) $(BACKDOOR_BIN) $(REMOTE_PYLIB)
	$(PYTHON) setup.py clean

.PHONY: clean all flash test disassemble pyscsi
.DELETE_ON_ERROR:
