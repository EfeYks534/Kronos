set -e
gbuild clean 
gbuild 

cd ../env/

./boot.sh

cd ../src/
