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
    QPalette*       widgetsPalette        {nullptr};

    QMenu*          appMenu               {nullptr};
    QMenu*          listenMenu            {nullptr};
    QMenu*          optionsMenu           {nullptr};

    QAction*        quitAction            {nullptr};
    QAction*        listenAction          {nullptr};
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

    QString                        serverIP;
    Server*                        server;
    std::unique_ptr<boost::thread> serverThread;

    std::vector<boost::uint8_t>    send_buffer;

    boost::mutex messageLabelMutex;


private: // Methods
    /**
     * @brief resetAtributes Reset pointers to widgets with nullptr
     */
    void resetAtributes();
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
    /**
     * @brief displayMessage Display messages in messageLabel.
     * @param message The message to display.
     */
    void displayMessage(const std::string& message);


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
    /**
     * @brief Deleting messages from messageLabel
     */
    void clearMessages();

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
    QSize sizeHint() const override;
};
#endif // CLIENT_SMAINWINDOW_H
