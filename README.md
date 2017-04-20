# A UDP Server that serve tokens using Token Bucket Algorithm

## Introduction

This repository contains a UDP server that implements the Token Bucket Algorithm that can be used for rate limiting and request throttling of individual IPv4 address. 

A web application or script can send UDP queries to the server with messages containing an IPv4 address; to check with the server whether access should be granted to that particular IP address. 

The server will look up the token bucket for this IPv4 address and check whether it has enough tokens. If there are sufficient tokens, the server decrements the tokens and send a positive response back to the Web application or script. The tokens for each IP bucket are refilled at a regular rate. 

Upon receiving the positive response, the web application/script can grant access to the IP address. If the server replies with a negative response (due to the rate limit being exceeded), the web application/script can reject access. 

The server implements a Token Bucket Algorithm for rate limiting accesses to IPv4 addresses. 


Refer to the following for a detailed article on setting up and using this UDP Token Bucket Server. 
[https://www.nighthour.sg/articles/2017/token-bucket-rate-limiter.html](https://www.nighthour.sg/articles/2017/token-bucket-rate-limiter.html)

## Building the Source Code

A Makefile is included to build the source code. It is configured to use cc as the compiler and can build the code on linux and BSD systems. For linux systems like Ubuntu 16, cc is by default set to gcc. On FreeBSD 11, the default cc is the clang compiler. Both can be used to build the source. 

To build the udp server

>make

A binary tbserver will be created

To build the test client

>make testclient

A binary testclient will be created. 

The testclient can be used to run some testing against the tbserver. 


## Source signature
Gpg Signed commits are used for committing the source files. 

> Look at the repository commits tab for the verified label for each commit. 

> A userful link on how to verify gpg signature in [https://github.com/blog/2144-gpg-signature-verification](https://github.com/blog/2144-gpg-signature-verification)


