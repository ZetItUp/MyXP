GCCPARMS = -m32 -Iinclude -g -fno-use-cxa-atexit -nostdlib -fno-stack-protector -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-return-type -fpermissive
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = 	obj/loader.o \
			obj/gdt.o \
			obj/lib/string.o \
			obj/lib/orderedarray.o \
			obj/lib/mem.o \
			obj/console.o \
			obj/drivers/driver.o \
			obj/hardware/port.o \
			obj/hardware/interruptstubs.o \
			obj/hardware/interrupts.o \
			obj/memorym.o \
			obj/multitasking.o \
			obj/hardware/pci.o \
			obj/drivers/keyboard.o \
			obj/drivers/mouse.o \
			obj/kernel.o 


run: mykernel.iso
	qemu-system-x86_64 -d int -cdrom mykernel.iso -m 512M -no-reboot -no-shutdown

d: mykernel.iso
	objcopy --only-keep-debug mykernel.bin kernel.sym
	qemu-system-x86_64 -s -S -d int -cdrom mykernel.iso -m 512M -no-reboot -no-shutdown

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	# ~/dev/cross/bin/i686-elf-gcc $(GCCPARMS) -c -o $@ $<
	gcc $(GCCPARMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

mykernel.iso: mykernel.bin
	mkdir -p iso
	mkdir -p iso/boot
	mkdir -p iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	echo 'set timeout=0'					>  iso/boot/grub/grub.cfg
	echo 'set default=0'					>> iso/boot/grub/grub.cfg
	echo ''									>> iso/boot/grub/grub.cfg
	echo 'menuentry "My Experience" {'		>> iso/boot/grub/grub.cfg
	echo '	multiboot /boot/mykernel.bin'	>> iso/boot/grub/grub.cfg
	echo '	boot'							>> iso/boot/grub/grub.cfg
	echo '}'								>> iso/boot/grub/grub.cfg
	grub-mkrescue --output=mykernel.iso iso >> iso/boot/grub/grub.cfg
	rm -rf iso

.PHONY: clean
clean:
	rm -rf obj mykernel.bin mykernel.iso