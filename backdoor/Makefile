FIRMWARE_EXE := SE-506CB_TS01.exe
FIRMWARE_BIN := SE-506CB_TS01.bin
FIRMWARE_URL := "http://www.tsstodd.com/TotalLib/popup/Download.asp?path=fwdownload&lang=eng&fname=SE-506CB_TS01.exe"
FIRMWARE_SUM := firmware.shasum

PATCH_ADDR := 0xC9600
PATCH_BIN := patch.bin
PATCH_SRC := patch.s
BACKDOOR_BIN := firmware-with-backdoor.bin

CHECKSUM_TOOL := python ../flasher/checksum.py
FLASHER_TOOL := ../flasher/mtflash
GCC_ARM := arm-none-eabi-gcc
OBJCOPY_ARM := arm-none-eabi-objcopy
OBJDUMP_ARM := arm-none-eabi-objdump
HEXDUMP_TOOL := xxd


# Compile the backdoored firmware image, from the official TS01 firmware
all: $(BACKDOOR_BIN)

# Disassemble a chunk of code around the patched location in the firmware
disassemble: $(BACKDOOR_BIN) $(PATCH_BIN)
	$(OBJDUMP_ARM) -D -b binary -m arm7tdmi -M force-thumb --start-address=$(PATCH_ADDR) $(BACKDOOR_BIN) | head -n 200
	$(HEXDUMP_TOOL) $(PATCH_BIN)

# Flash the patched firmware! I hope you know what you're doing! The flasher
# tool knows about our backdoor's signature, so it will let you verify that
# the patch is working properly after it restarts.
flash: $(BACKDOOR_BIN)
	$(FLASHER_TOOL) $(BACKDOOR_BIN)

# Download the original firmware from TSST's web site (slow, only happens once)
$(FIRMWARE_EXE): $(FIRMWARE_SUMS)
	echo --- Downloading original firmware image
	curl $(FIRMWARE_URL) > $@

# The EXE is a self-extracting ZIP actually. Grab the .BIN file and check its shasum
$(FIRMWARE_BIN): $(FIRMWARE_EXE)
	unzip -o $(FIRMWARE_EXE) $(FIRMWARE_BIN) || true
	shasum -c $(FIRMWARE_SUM)

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
	rm -f $(FIRMWARE_BIN) $(PATCH_BIN) $(BACKDOOR_BIN)

.PHONY: clean all flash test disassemble
.DELETE_ON_ERROR:
