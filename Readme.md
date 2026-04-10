# 🌐 ft_irc
*This project has been created as part of the 42 curriculum by iarefeva, mmravec.*

## 📖 Description

### Project Overview
This project is a custom **IRC (Internet Relay Chat) server** developed as part of the 42 curriculum.<br>
Project was developed based on **Weechat** client, but other clients working with IRC protocol can be used. Server accepts allowing multiple users to join channels and exchange messages in real-time.<br>

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
- DCC File Transfer (Direct Client-to-Client)<br>
Supports sending files between users using DCC<br>
Implemented via direct TCP connection between clients<br>
⚠️ Currently works only within the same local network (LAN)<br>
- IRC Bot


## ⚙️ Instructions

### 🏗️ Project Structure
```
ft_irc/
├── bot/        # Bonus files for bot
├── inc/        # Mandatory header files
├── src/        # Mandatory source files
├── unit_tests/ # Unit_tests
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
_Expected behavior:_ Both clients receive the message in real time.

### 🤖 Bot Usage

This project also includes a simple IRC bot called MrBot. It connects to our IRC server and responds to basic commands.

#### 🔧 Compilation

```
make bonus
```

This will generate the executable:
```
./ircbot
```

#### ▶️ Running the Bot

Run the bot with:
```
./ircbot <IP> <port> <password>
```
Example:
```
./ircbot 127.0.0.1 6667 mypassword
```
<IP> – IP address of your IRC server
<port> – Port used by the server
<password> – Server password (PASS command)

#### 🔌 Behavior

After launching, the bot will:
- Connect to the IRC server
- Authenticate using: <br>
		- PASS <br>
		- NICK (MrBot) <br>
		- USER <br>
- Listen for incoming messages
- Respond to commands automatically


#### 💬 Available Commands

You can interact with the bot via private messages:

!joke _- Returns a random joke_

!help _- Displays help information_

#### 🧪 Example Interaction
```
User: PRIVMSG MrBot :!joke
MrBot: Why do programmers prefer dark mode? Because light attracts bugs.

User: PRIVMSG MrBot :!help
MrBot: Hi, I am MrBot! Available commands: !joke, !help
```

#### 🛑 Stopping the Bot

Press Ctrl + C to gracefully terminate the bot.

#### ⚠️ Notes
The bot currently handles:<br>
- PRIVMSG (commands)
- PING (keeps connection alive)<br>

The bot does not automatically join channels (can be extended)<br>
Make sure the server is running

### 📁 DCC File Transfer (WeeChat)

This project supports DCC (Direct Client-to-Client) file transfer, allowing users to send files directly to each other.

#### ⚙️ Requirements
Both users must be connected to the IRC server<br>
Both users must use a client that supports DCC (e.g., WeeChat)<br>
⚠️ Works reliably only within the same local network (LAN)<br>
📤 Sending a File (WeeChat)<br>

In WeeChat, use:
```
/dcc send <nickname> <file_path>
```
Example:
```
/dcc send user2 ./example.txt
```
#### 📥 Receiving a File

When another user sends you a file, WeeChat will display a notification.

To accept the file:
```
/dcc accept <nickname>
```
#### 📂 Default Download Location

Received files are usually saved in:
```
~/weechat/dcc/
```
You can change this path in WeeChat settings.

#### ⚠️ Notes & Limitations
DCC uses a direct TCP connection between clients, not the server<br>
Firewalls or NAT may block transfers<br>
This implementation currently:<br>
- May not work over the internet without port forwarding<br>
- Make sure the receiving client allows incoming DCC connections<br>

#### 🧪 Example
```
User1: /dcc send user2 ./file.txt
User2: (receives notification)
User2: /dcc accept user1
```
Result: file.txt is transferred directly from User1 to User2.


## 📚 Resources

### 📖 Documentation
RFC 1459 – Internet Relay Chat Protocol<br>
RFC 2812 – IRC Client Protocol<br>
Beej’s Guide to Network Programming<br>
Linux poll / select man pages<br>

## 🤖 AI Usage

AI tools (such as ChatGPT, Clode) were used in this project for:

Clarifying IRC protocol behavior and edge cases<br>
Understanding socket programming concepts<br>
Debugging specific issues<br>
Improving code structure and readability<br>
Assisting in writing documentation (including this README)<br>

All implementation decisions, architecture, and final code were designed and validated by the authors.

## 🚀 Possible Improvements
Full support of all IRC modes<br>
Better scalability (epoll/kqueue)<br>
Logging system<br>
TLS/SSL support<br>