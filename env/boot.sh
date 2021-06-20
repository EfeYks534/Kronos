./gen_image.sh

qemu-system-x86_64 -M q35 -smp 4 -d int -serial stdio -no-reboot -no-shutdown -m 4G kronos.img
