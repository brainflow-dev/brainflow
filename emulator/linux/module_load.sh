echo "Removing previous module, if any"
rmmod emulated_cython
echo "Inserting new module"
insmod emulated_cython.ko 
echo "Done"
