#ifndef CLIENT_CMAINWINDOW_H
#define CLIENT_CMAINWINDOW_H

#include "client.h"


/**
 * @class CMainWindow
 * @brief Is the class that coordinates the UI and client server backend.
 *
 * This class manages the server GUI, including menus, user input, and
 * connection status display. It communicates with the Server class to handle
 * network operations.
 */
class CMainWindow : public QMainWindow
{
    Q_OBJECT
public:

    static inline const char* WINDOWNAME = "Client LANChat";

private:
    // Fields
    QPalette*       windowPalette         {nullptr};
    QPalette*       buttonPalette         {nullptr}; 

    QMenu*          appMenu               {nullptr};
    QMenu*          connectionMenu        {nullptr};
    QMenu*          optionsMenu           {nullptr};

    QAction*        quitAction            {nullptr};
    QAction*        connectAction         {nullptr};
    QAction*        clearMessagesAction   {nullptr};

    QLabel*         connectionStatusLabel {nullptr};
    QLabel*         messagesLabel         {nullptr};

    QScrollArea*    messagesLabelScroll   {nullptr};
    QHBoxLayout*    messagesLabelLayout   {nullptr};

    QLineEdit*      userInputLine         {nullptr};

    QWidget*        centralWidget         {nullptr};
    QVBoxLayout*    verticalLayout        {nullptr};
    QHBoxLayout*    orizontalLayout       {nullptr};

    QPushButton*    sendButton            {nullptr};

    QLineEdit*      ipAddressInput        {nullptr};
    QLineEdit*      portInput             {nullptr};
    QPushButton*    connectButton         {nullptr};

    std::string                        serverPort;
    std::string                        serverIPaddress;

    Client*                            client;
    std::unique_ptr<boost::thread>     clientThread;

    std::vector<boost::uint8_t>        send_buffer;

    boost::mutex                       messageLabelMutex;


private:
    /**
     * @brief resetAtributes Reset pointers to widgets with nullptr
     */
    void resetAtributes();
    /**
     * @brief Initializes the widgets needed to receive server data from the user.
     *        It is called in the getServerInfo method when the Connect button is
     *        clicked in the Connect menu.
     */
    void addServerInfo();
    /**
     * @brief Sets the palette for the application window and buttons.
     *        It is called in the CMainWindow constructor.
     */
    void setPalettes();
    /**
     * @brief Initializes the central widget and layouts.
     *        It is called in the addServerInfo and onConnection methods.
     */
    void addLayouts();
    /**
     * @brief Initializes the menus.
     *        It is called in the CMainWindow constructor.
     */
    void addMenu();
    /**
     * @brief Initializes the status label.
     *        It is called in the addServerInfo and onConnection methods.
     * @param status Connection status message.
     */
    void addStatusLable(const char* status);
    /**
     * @brief Initializes the widgets to be able to read data from the user.
     *        It is called in the onConnection method.
     */
    void addUserInput();
    /**
     * @brief Adds a label for messages.
     *        It is called in the onConnection method.
     */
    void addMessagesLabel();
    /**
     * @brief Starts the connection to the server by calling the Client::connect method
     *        in a temporary thread.
     *        It is called when the connectButton button is clicked.
     */
    void startConnection();
    /**
     * @brief Starts the connection to the server by calling the Client::connect method
     *        in a temporary thread.
     *        It is called when the connectButton button is pressed.
     * @param status Connection status message.
     */
    void onConnection(const char* status);

private slots:
    /**
     * @brief Begins initialization to switch to displaying widgets for reading server
     *        data from the user. If the central widget is initialized before the given
     *        method is called, it is reset (all child widgets are destroyed).
     *        It is called when the Connect button in the Connect menu is clicked.
     */
    void getServerInfo();
    /**
     * @brief Sets the status of connections to the server.
     *        It is connected to the Client::connectionStatus signal.
     * @param status Connection status given by the client object.
     */
    void connectionStatus(const char* status);
    /**
     * @brief Calls the Client::send method with the argument being the text from userInputLine.
     *        It is called when the sendButton is clicked.
     */
    void sendingMessages();
    /**
     * @brief Deleting messages from messageLabel
     */
    void clearMessages();

public:
    /**
     * @brief Constructor for the CMainWindow class.
     *        Initializes the client object and calls the setPalettes and addMenu methods.
     */
    CMainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor for the CMainWindow class. Calls Client::finish method.
     *        Delete centralWidget (all child widgets are destroyed).
     */
    ~CMainWindow();
    /**
     * @brief Suggests a default size for the main window.
     * @return Suggested size.
     */
    QSize sizeHint() const;
};
#endif // CLIENT_CMAINWINDOW_H
