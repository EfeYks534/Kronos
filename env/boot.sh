./gen_image.sh

qemu-system-x86_64 -M q35 -global hpet.msi=true -smp 4 -serial stdio -no-reboot -no-shutdown -m 4G kronos.img
