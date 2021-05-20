rm -f shitos4.img

dd if=/dev/zero bs=1M count=128 of=./shitos4.img

parted -s ./shitos4.img mklabel msdos
parted -s ./shitos4.img mkpart primary 1 100%

LOOPDEV=$(sudo losetup --show -Pf ./shitos4.img)

LOOPPART=$LOOPDEV"p1"

sudo mkfs.ext2 $LOOPPART

mkdir -p ./mount

sudo mount $LOOPPART --target ./mount

sudo mkdir ./mount/boot/

sudo cp shitos4.bin ./mount/shitos4.bin
sudo cp limine.cfg  ./mount/boot/limine.cfg

sudo cp ./limine/limine.sys ./mount/boot/limine.sys

sudo umount $LOOPPART

sudo losetup --detach $LOOPDEV

rm -rf ./mount

./limine/limine-install shitos4.img
