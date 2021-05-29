./gen_image.sh

qemu-system-x86_64 -M q35 -smp 4 -enable-kvm -serial stdio -no-reboot -no-shutdown -m 4G shitos4.img
