
PORT          ?= /dev/ttyUSB1 # HiFive Unmatched USB-UART
OPENSBI        = $(shell pwd)/lib/opensbi/build/platform/generic/firmware/fw_dynamic.bin

$(OUTPUT)/cache/OVMF.fd:
	mkdir -p cache
	cd cache && curl -o OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASERISCV64_VIRT_CODE.fd && dd if=/dev/zero of=OVMF.fd bs=1 count=0 seek=33554432

.PHONY: image
image: build
	# Create boot filesystem
	rm -rf $(OUTPUT)/image.dir
	mkdir -p $(OUTPUT)/image.dir/EFI/BOOT/
	mkdir -p $(OUTPUT)/image.dir/boot/
	make -C lib/limine
	cp lib/limine/BOOTRISCV64.EFI $(OUTPUT)/image.dir/EFI/BOOT/
	cp lib/limine.cfg $(OUTPUT)/image.dir/boot/
	cp $(OUTPUT)/badger-os.elf $(OUTPUT)/image.dir/boot/
	
	# Build OpenSBI and U-boot
	make -C lib/opensbi PLATFORM=generic CROSS_COMPILE=$(CROSS_COMPILE)
	make -C lib/u-boot sifive_unmatched_defconfig CROSS_COMPILE=$(CROSS_COMPILE)
	make -C lib/u-boot OPENSBI=$(OPENSBI) CROSS_COMPILE=$(CROSS_COMPILE)
	
	# Format FAT filesystem
	rm -f $(OUTPUT)/image_bootfs.bin
	dd if=/dev/zero bs=1M count=4  of=$(OUTPUT)/image_bootfs.bin
	mformat -i $(OUTPUT)/image_bootfs.bin
	mcopy -s -i $(OUTPUT)/image_bootfs.bin $(OUTPUT)/image.dir/* ::/
	
	# Create image
	rm -f $(OUTPUT)/image.hdd
	dd if=/dev/zero bs=1M count=64 of=$(OUTPUT)/image.hdd
	# 1M (SPL), 1007K (U-boot), 4M /boot, remainder /root
	sgdisk -a 1 \
		--new=1:34:2081    --change-name=1:spl   --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
		--new=2:2082:4095  --change-name=2:uboot --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
		--new=3:4096:12287 --change-name=3:boot  --typecode=3:0x0700 \
		--new=4:12288:-0   --change-name=4:root  --typecode=4:0x8300 \
		$(OUTPUT)/image.hdd
	
	# Copy data onto partitions
	dd if=lib/u-boot/spl/u-boot-spl.bin bs=512 seek=34   of=$(OUTPUT)/image.hdd conv=notrunc
	dd if=lib/u-boot/u-boot.itb         bs=512 seek=2082 of=$(OUTPUT)/image.hdd conv=notrunc
	dd if=$(OUTPUT)/image_bootfs.bin             bs=512 seek=4096 of=$(OUTPUT)/image.hdd conv=notrunc

.PHONY: qemu
qemu: $(OUTPUT)/cache/OVMF.fd image
	qemu-system-riscv64 \
		-M virt -cpu rv64 -m 2G \
		-device ramfb -device qemu-xhci -device usb-kbd \
		-drive if=pflash,unit=0,format=raw,file=$(OUTPUT)/cache/OVMF.fd \
		-device virtio-scsi-pci,id=scsi -device scsi-hd,drive=hd0 \
		-drive id=hd0,format=raw,file=$(OUTPUT)/image.hdd
