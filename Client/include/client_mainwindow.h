#ifndef CLIENT_CMAINWINDOW_H
#define CLIENT_CMAINWINDOW_H

#include <QScrollArea>
#include <QScrollBar>
#include <QMainWindow>
#include <QPalette>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMenuBar>
#include <QLabel>
#include <QString>

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
    QPalette*       m_widgetsPalette        {nullptr};

    QMenu*          m_appMenu               {nullptr};
    QMenu*          m_connectionMenu        {nullptr};
    QMenu*          m_optionsMenu           {nullptr};

    QAction*        m_quitAction            {nullptr};
    QAction*        m_connectAction         {nullptr};
    QAction*        m_clearMessagesAction   {nullptr};

    QLabel*         m_welcomeLabel          {nullptr};
    QLabel*         m_connectionStatusLabel {nullptr};
    QLabel*         m_messagesLabel         {nullptr};

    QScrollArea*    m_messagesLabelScroll   {nullptr};
    QHBoxLayout*    m_messagesLabelLayout   {nullptr};

    QLineEdit*      m_userInputLine         {nullptr};

    QWidget*        m_centralWidget         {nullptr};
    QVBoxLayout*    m_verticalLayout        {nullptr};
    QHBoxLayout*    m_orizontalLayout       {nullptr};

    QPushButton*    m_sendButton            {nullptr};

    QLineEdit*      m_ipAddressInput        {nullptr};
    QLineEdit*      m_portInput             {nullptr};
    QPushButton*    m_connectButton         {nullptr};

    std::string                        m_serverPort;
    std::string                        m_serverIPaddress;

    Client*                            m_client;
    std::unique_ptr<boost::thread>     m_clientThread;

    std::vector<boost::uint8_t>        m_send_buffer;

    boost::mutex                       m_messageLabelMutex;

    bool                               m_hasEverConnected;


private:
    /**
     * @brief initWelcomeScreen Initializing widgets to be displayed immediately upon widget creation
     */
    void initWelcomeScreen();
    /**
     * @brief adjustFontSize Font adjustment when the window is resized
     * @param fontSize Font size for the m_welcomeLabel
     */
    void adjustFontSize(const int& fontSize);
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
    void addPalettes();
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
    /**
     * @brief displayMessage Display messages in messageLabel.
     * @param message The message to display.
     */
    void displayMessage(const std::string& message);
    /**
     * @brief cleanup Freeing memory allocated that was not freed through the parent-child relationship.
     */
    void cleanup();

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

protected:
    /**
     * @brief Suggests a default size for the main window.
     * @return Suggested size.
     */
    QSize sizeHint() const override;
    /**
     * @brief resizeEvent Override this to handle resize events (ev).
     */
    void resizeEvent(QResizeEvent* event) override;

public:
    /**
     * @brief Constructor for the CMainWindow class.
     *        Initializes the client object and calls the addPalettes and addMenu methods.
     */
    CMainWindow(QWidget *parent = nullptr);
    /**
     * @brief Destructor for the CMainWindow class. Calls Client::finish method.
     *        Delete centralWidget (all child widgets are destroyed).
     */
    ~CMainWindow();
};
#endif // CLIENT_CMAINWINDOW_H
