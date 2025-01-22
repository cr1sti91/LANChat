
# LANChat 🖥️💬

This is a chat application that allows communication between two devices in a **LAN network**.

---

## Requirements 📋
To compile and run this project, the following tools are required:
- **[Conan](https://conan.io/):** Package manager for installing and linking the Boost library (required for networking).
- **[Qt Creator](https://www.qt.io/product/development-tools):** For linking the Qt libraries used in the graphical interface.

---

## How to Compile 🛠️
1. **Install Conan:**
   ```bash
   pip install conan
   ```
2. **Install Qt Creator:** Download and install it from [Qt's official website](https://www.qt.io/download).
3. **Configure Conan and install dependencies:**
   Inside the project directory, run:
   ```bash
   conan install . --build=missing
   ```
4. **Build the project:** Open the project in Qt Creator and compile it.

---

## How to Run 🚀
### Step 1: Start the Server
1. Run the `ServerChat` executable.
2. From the **"Listen" menu**, choose the **"Listen" action**.
   - The server will detect a valid IP address of the device in the LAN network and start listening on it.

### Step 2: Start the Client
1. Run the `ClientChat` executable.
2. From the **"Connect" menu**, choose the **"Connect" action**.
3. In the input fields:
   - Enter the **IP address** and **port** where the server is listening.
   - (This information can be found in the server's status line.)
4. Click the **"Connect" button** to establish the connection.

### Step 3: Chat!
- Once connected, you can start communicating between the `ServerChat` and `ClientChat`.

### Ending a Session
- Click the **"Listen"** or **"Connect"** button in the menu bar to stop the current session and start a new one.

---

## Features ✨
- Communication over a **LAN network**.
- **Server-Client architecture** for managing connections.
- **Graphical interface** powered by Qt for an intuitive user experience.

---

## License 📄

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

## Contact ✉️
If you have any questions or issues, feel free to contact:
- **Email:** cristi.tacu61@gmail.com