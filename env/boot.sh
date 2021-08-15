./gen_image.sh

qemu-system-x86_64 -M q35 -global hpet.msi=true -smp 4 -serial stdio -no-reboot -no-shutdown -m 4G -drive file=kronos.vmdk,format=vmdk,if=none,id=mynvme -device nvme,drive=mynvme,serial=deadbeef
