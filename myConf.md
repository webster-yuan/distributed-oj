1. 安装ctemplate库时会出现报错信息:
1.1 首先将ctemplate库下载,得到ctemplate-master文件夹,进入之后,执行
`./autogen.sh`
如果出现报错: 
`./autogen.sh: line 17: autoreconf: command not found`
如果出现报错
`error: Libtool library used but ‘LIBTOOL’ is undefined`
解决:
`sudo yum install -y autoconf automake libtoo`
`sudo yum install libtool`

[注意]报错信息只需要关注error就行，不用关注warning

1.2 执行`./configure`
1.3 `make`
1.4 `sudo make install`
注意: ctemplate的头文件都放在了`/usr/local/include`文件夹下,.so都在`/usr/local/lib`文件夹下.发现没有`libctemplate.so.3`,
键入 []$ `export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH`永久生效

2. 无法找到libctemplate.so.3动态库,所以需要在 lib64 文件夹中添加这个库
使用`find / -name libctemplate.so.3`得到路径 `dir`，将这个库链接到lib64目录下
`cd /usr/lib64`
`cp dir/libctemplate.so.3 ./`

3. 为了使用openSSL中的md5算法，需要安装openssl-devel
`sudo yum install openssl-devel`
编译代码时需要加上`-lssl -lcrypto`:
    `g++ -o password_hash password_hash.cpp -lssl -lcrypto`
源文件中包含的头文件是`openssl/md5.h`
4. 时刻保持g++版本问题
`scl enable devtoolset-8 bash`
5. 安装过程详细说明
    更新系统包：
    `sudo yum update`
    安装curl和libcurl开发包：
    `sudo yum install curl curl-devel`
    这将安装所需的所有依赖项，使你能够在C++项目中使用libcurl进行HTTP请求。