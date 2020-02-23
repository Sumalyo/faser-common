# Copied from daqling/cmake/setup.sh to setup recent C++ compiler

echo "Custom compiler, installed by Ansible from OHPC."

export LD_LIBRARY_PATH=/opt/ohpc/pub/compiler/gcc/8.3.0/lib64/:/usr/local/lib64/:$LD_LIBRARY_PATH
export PATH=/opt/ohpc/pub/compiler/gcc/8.3.0/bin:$PATH

# Export package specific environmental variables

export CC='/opt/ohpc/pub/compiler/gcc/8.3.0/bin/gcc'
export CXX='/opt/ohpc/pub/compiler/gcc/8.3.0/bin/g++'
