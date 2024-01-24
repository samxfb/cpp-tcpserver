# cpp-tcpserver
Wrapping the Asio library to create a simple tcp server.

Mac and Linux is supported

# asio模块加载
```
git submodule update --init --recursive
```

# 编译
```
./build.sh
```

# 运行
```
./tcpserver (端口默认8000)
```

# 测试
tcpserver
```
(base) sam@SamdeMacBook-Pro:~/Desktop/test/cpp-tcpserver$ ./tcpserver
client 127.0.0.1 is connect
Recv client 127.0.0.1 message:12345

Recv client 127.0.0.1 message:hello


client 127.0.0.1 is closed
```
client
```
(base) sam@SamdeMacBook-Pro:~/Desktop/test/cpp-httpclient$  nc 127.0.0.1 8000
12345
12345
hello
hello
^C
```
