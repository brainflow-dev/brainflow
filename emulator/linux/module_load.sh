echo "rmmod will be executed to uninstall previos module if any"
rmmod emulated_cython
insmod emulated_cython.ko
echo "new driver installed"