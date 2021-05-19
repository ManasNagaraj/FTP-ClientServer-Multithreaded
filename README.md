# FTP-ClientServer-Multithreaded

shiiittiestt code ever written in complete state of delusion

just skip this one won't help you in any way

## Usage

To run the FTP server:

```
cd server && g++ ./server 5000

```

To run the FTP client:

```
cd client && g++ ./client 127.0.0.1 5000

```

> You can multithread by creating client instances on different terminals

> Use any port above 1024

## Commands

You can use the following commands on the client side:

cd into a directory on the client side

```
ftp>cd <directory>
```

Kill the connection with the following command.

```
ftp>quit
```

dump the contents ont he client side Directory.

```
ftp>ls
```

Get the working directory on te client side.

```
ftp>pwd
```

File upload from client to the server.

```
ftp>put <filename>
```

File download from the server side.

```
ftp>get <filename>
```
