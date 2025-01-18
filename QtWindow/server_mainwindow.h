#ifndef CLIENT_SMAINWINDOW_H
#define CLIENT_SMAINWINDOW_H

#include "server.h"

/**
 * @class SMainWindow
 * @brief Main window for the server-side application of the LANChat.
 *
 * This class manages the server GUI, including menus, user input, and
 * connection status display. It communicates with the Server class to handle
 * network operations.
 */
class SMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    static inline const char* WINDOWNAME = "Server LANChat";

private: // Fields
    QPalette*       windowPalette;
    QPalette*       buttonPalette;

    QAction*        quitAction;
    QAction*        listenAction;

    QMenu*          appMenu;
    QMenu*          listenMenu;

    QLabel*         connectionStatusLabel;
    QLabel*         messagesLabel;

    QScrollArea*    messagesLabelScroll;
    QHBoxLayout*    messagesLabelLayout;

    QLineEdit*      userInputLine;

    QWidget*        centralWidget;
    QVBoxLayout*    verticalLayout;
    QHBoxLayout*    orizontalLayout;

    QPushButton*    sendButton;

    QString                        serverIP;
    Server*                        server;
    std::unique_ptr<boost::thread> serverThread;

    std::vector<boost::uint8_t>    send_buffer;

    boost::mutex messageLabelMutex;


private: // Methods
    /**
     * @brief Sets the palette for the application window and buttons.
     *        It is called in the SMainWindow constructor.
     */
    void setPalettes();
    /**
     * @brief Initializes the central widget and layouts.
     *        It is called in the startListening method.
     */
    void addLayouts();
    /**
     * @brief Initializes the menus.
     *        It is called in the SMainWindow construtor.
     */
    void addMenu();
    /**
     * @brief Initializes the status label.
     *        It is called in the startListening method.
     */
    void addStatusLable();
    /**
     * @brief Initializes the widgets to be able to read data from the user.
     *        It is called in the connectionStatus method.
     */
    void addUserInput();
    /**
     * @brief Adds a label for messages.
     *        It is called in the connectionStatus method.
     */
    void addMessagesLabel();
    /**
     * @brief Calls the Sever::listen method for the server object in a temporary thread.
     *        If the central widget is initialized before the given
     *        method is called, it is reset (all child widgets are destroyed).
     *        It is called when Listen button in Listen menu is clicked.
     */
    void startListening();
    /**
     * @brief Calls the Server::send method with the argument being the text from userInputLine.
     *        It is called when the sendButton is clicked.
     */
    void sendingMessages();


private slots:
    /**
     * @brief Updates the status label with the server's listening endpoint.
     * @param endpoint Shared pointer to the server endpoint.
     */
    void setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint);
    /**
     * @brief Sets the status of connections to the server.
     *        It is connected to the Client::connectionStatus signal.
     * @param status Connection status given by the server object.
     */
    void connectionStatus(const char* status);

public:
    /**
     * @brief Constructor for the SMainWindow class. Initializes the client object and
     *        calls the setPalettes and addMenu methods.
     * @param parent Pointer to the parent widget (default is nullptr).
     */
    SMainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor for the SMainWindow class. Calls Server::finish method.
     *        Delete centralWidget (all child widgets are destroyed).
     */
    ~SMainWindow();
    /**
     * @brief Suggests a default size for the main window.
     * @return Suggested size.
     */
    QSize sizeHint() const;
};
#endif // CLIENT_SMAINWINDOW_H
