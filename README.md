This is a chat application that allows communication between two devices in a LAN network.
To compile the project, it is necessary to install the conan package manager, which is used to install and 
link the boost library required for the networking part. For the graphical interface part, it is necessary 
to install Qt Creator to link the Qt libraries.
To run the application, it is first necessary to run the ServerChat executable and from the "Listen" menu,
choose the "Listen" action so that the server finds a valid IP address of the device in the LAN network 
and then starts listening to it. Then, the ClientChat executable is run with the "Connect" action from the
"Connect" menu. In the typing lines, it is necessary to enter the IP address and the port on which the 
server listens (these are in the server status line). After the "Connect" button is clicked, the connection
is established and communication can be made between ServerChat and ClientChat.
Clicking the "Listen" or "Connect" button in the menu bar again stops the current session and starts a new 
connection session.
