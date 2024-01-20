#!/bin/bash

g++ -I. -I./thirdparty/asio/asio/include -std=c++11 -pthread -o tcpserver main.cpp tcpserver.cpp