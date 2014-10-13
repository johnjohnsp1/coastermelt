FIRMWARE_EXE := SE-506CB_TS01.exe
FIRMWARE_BIN := SE-506CB_TS01.bin
FIRMWARE_URL := "http://www.tsstodd.com/TotalLib/popup/Download.asp?path=fwdownload&lang=eng&fname=SE-506CB_TS01.exe"
FIRMWARE_SUM := sums.txt

PATCH_ADDR := 0xCABB8
PATCH_BIN := patch.bin
PATCH_SRC := patch.s
BACKDOOR_BIN := backdoor.bin

CHECKSUM_TOOL := python ../flasher/checksum.py
FLASHER_TOOL := ../flasher/mtflash
GCC_ARM := arm-none-eabi-gcc
OBJCOPY_ARM := arm-none-eabi-objcopy
OBJDUMP_ARM := arm-none-eabi-objdump

all: $(BACKDOOR_BIN)
	$(OBJDUMP_ARM) -D -b binary -m arm7tdmi -M force-thumb --start-address=$(PATCH_ADDR) $< | head -n 100

flash: $(BACKDOOR_BIN)
	$(FLASHER_TOOL) $(BACKDOOR_BIN)

$(BACKDOOR_BIN): $(PATCH_BIN) $(FIRMWARE_BIN)
	cp $(FIRMWARE_BIN) $(BACKDOOR_BIN)
	(dd if=$(FIRMWARE_BIN) bs=1 count=$(PATCH_ADDR); cat $(PATCH_BIN)) | dd of=$(BACKDOOR_BIN) conv=notrunc
	$(CHECKSUM_TOOL) --fix $@

$(FIRMWARE_BIN): $(FIRMWARE_EXE)
	unzip -o $(FIRMWARE_EXE) $(FIRMWARE_BIN) || true
	shasum -c $(FIRMWARE_SUM)

$(FIRMWARE_EXE): $(FIRMWARE_SUMS)
	echo --- Downloading original firmware image
	curl $(FIRMWARE_URL) > $@

$(PATCH_BIN): $(PATCH_SRC)
	$(GCC_ARM) -nostdlib -nostdinc -o $@ $<
	$(OBJCOPY_ARM) $@ -O binary

clean:
	rm -f $(FIRMWARE_BIN) patch.bin backdoor.bin

.PHONY: clean all flash
.DELETE_ON_ERROR: