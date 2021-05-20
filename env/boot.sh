./gen_image.sh

qemu-system-x86_64 -M q35 -serial stdio -no-reboot -no-shutdown -m 4G shitos4.img
