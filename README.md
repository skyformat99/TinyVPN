# TinyVPN

一个用来学习的简易vpn，基于tun隧道实现

# 安装

TinyVPN使用了gflags和libtins第三方库，使用cmake编译

所以在编译之前需要安装libpcap、libcrypto和cmake

可以使用

```
apt-get install libpcap-dev libssl-dev cmake
```

或

```
yum install libpcap-devel openssl-devel cmake
```
来安装依赖的软件包

最后使用cmake外部编译即可

```
cd TinyVPN
mkdir build
cd build
cmake ..
make
```

二进制文件在build/bin目录下

# 使用

## 客户端

1. 启动client
```
$ sudo ./client --srv_port <服务端端口> --srv_addr <服务端IP>
```
2. 添加路由
```
$ sudo ip route add default dev <tun设备名>
$ sudo ip route add <服务端IP> via <网关>
```
tun设备名一般为tun0，服务端IP和启动client时的服务端IP是一样的。
网关查询可以使用`route -n`

## 服务端

1. 启动server
```
$ sudo ./server --port <端口> --tun_addr <tun设备IP>
```
2. 添加SNAT
```
$ iptables -t nat -A POSTROUTING -s <tun设备子网>-o eth0 -j SNAT --to-source <出口ip(例如eth0的ip)>
```
