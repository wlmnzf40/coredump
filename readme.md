# 编译参数
mkdir build && cd build
cmake ..
make

# 启动参数
echo 'export MALLOC_CONF="prof:true,prof_prefix:jeprof.out,junk:true" ' >> /etc/profile && source /etc/profile
