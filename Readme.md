# 🌐 ft_irc
*This project has been created as part of the 42 curriculum by iarefeva, mmravec.*

## 📖 Description

### Project Overview
This project is a custom **IRC (Internet Relay Chat) server** developed as part of the 42 curriculum.
Project was developed based on **Weechat** client, but other clients working with IRC protocol can be used. Server accepts allowing multiple users to join channels and exchange messages in real-time.

### 🧩 Features
- Multi-client handling
- User authentication (PASS, NICK, USER)
- Channel management:
	- JOIN / PART
	- TOPIC
	- INVITE
	- KICK
- Channel operator
- Private messaging (PRIVMSG)
- Channel modes (oklit)
- Graceful client disconnection
## ⭐ Bonus Features
- DCC File Transfer (Direct Client-to-Client)
Supports sending files between users using DCC
Implemented via direct TCP connection between clients
⚠️ Currently works only within the same local network (LAN)
- IRC Bot


## ⚙️ Instructions

### 🏗️ Project Structure
```
ft_irc/
├── src/        # Source files
├── inc/        # Header files
├── Makefile
└── README.md
```

### 🔧 Compilation

Clone the repository and compile using make:
``` bash
git clone <link to the repo> ft_irc
cd ft_irc
make
```
This will generate the executable:
```bash
./ircserv
```

### ▶️ Usage
Run the server with:
``` bash
./ircserv <port> <password>
```
Example:
``` bash
./ircserv 6667 mypassword
```
### 💻 Connecting to the server
There will be described connection through WeeChat, but you can use any IRC client such as: 
- nc (netcat)
- HexChat
- WeeChat
- irssii

In WeeChat window, first add server:
```
/server add <name> <server IP>/6667 (or other port)
```
Second add password to the server:
```
/set irc.server.<name>.password <password>
```
Third connect to the server:
```
/connect <name>
```
All commands for connection WeeChat will send automaticaly.

### 🧩 Features
- Multi-client handling
- User authentication (PASS, NICK, USER)
- Channel management:
	- JOIN / PART
	- TOPIC
	- INVITE
	- KICK
- Operator commands
- Private messaging (PRIVMSG)
- Channel modes (depending on implementation)
- Graceful client disconnection

### 🧪 Example Usage
```
Client 1: JOIN #42
Client 2: JOIN #42
Client 1: PRIVMSG #42 :Hello from 42!
```
Expected behavior:

Both clients receive the message in real time.

## 📚 Resources

### 📖 Documentation
RFC 1459 – Internet Relay Chat Protocol
RFC 2812 – IRC Client Protocol
Beej’s Guide to Network Programming
Linux poll / select man pages

## 🤖 AI Usage

AI tools (such as ChatGPT) were used in this project for:

Clarifying IRC protocol behavior and edge cases
Understanding socket programming concepts
Debugging specific issues
Improving code structure and readability
Assisting in writing documentation (including this README)

All implementation decisions, architecture, and final code were designed and validated by the authors.

## 🚀 Possible Improvements
Full support of all IRC modes
Better scalability (epoll/kqueue)
Logging system
TLS/SSL support