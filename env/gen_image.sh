rm -f kronos.img

dd if=/dev/zero bs=1M count=64 of=./kronos.img

parted -s ./kronos.img mklabel msdos
parted -s ./kronos.img mkpart primary 1 100%

LOOPDEV=$(sudo losetup --show -Pf ./kronos.img)

LOOPPART=$LOOPDEV"p1"

sudo mkfs.ext2 $LOOPPART

mkdir -p ./mount

sudo mount $LOOPPART --target ./mount

sudo mkdir ./mount/boot/

sudo cp kronos.bin ./mount/kronos.bin
sudo cp limine.cfg  ./mount/boot/limine.cfg

sudo cp ./limine/limine.sys ./mount/boot/limine.sys

sudo umount ./mount

sudo losetup --detach $LOOPDEV

rm -rf ./mount

./limine/limine-install kronos.img

qemu-img convert -f raw -O vmdk kronos.img kronos.vmdk
