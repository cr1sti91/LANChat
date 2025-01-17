#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include "client.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    static inline const char* WINDOWNAME = "Client LANChat";

private:
    // Fields
    QPalette*       windowPalette         {nullptr};
    QPalette*       buttonPalette         {nullptr};

    QAction*        quitAction            {nullptr};
    QAction*        connectAction         {nullptr};

    QMenu*          fileMenu              {nullptr};
    QMenu*          connectionMenu        {nullptr};

    QLabel*         connectionStatusLabel {nullptr};
    QLabel*         messagesLabel         {nullptr};

    QScrollArea*    messagesLabelScroll   {nullptr};
    QHBoxLayout*    messagesLabelLayout   {nullptr};

    QLineEdit*      userInput             {nullptr};

    QWidget*        centralWidget         {nullptr};
    QVBoxLayout*    verticalLayout        {nullptr};
    QHBoxLayout*    orizontalLayout       {nullptr};

    QPushButton*    sendButton            {nullptr};

    // Server - client atributes
    QLineEdit*      ipAddressInput        {nullptr};
    QLineEdit*      portInput             {nullptr};
    QPushButton*    connectButton         {nullptr};

    std::string                        serverPort;
    std::string                        serverIPaddress;

    Client*                            client{nullptr};
    std::unique_ptr<boost::thread>     clientThread;

    std::vector<boost::uint8_t>        send_buffer;

    boost::mutex                       messageLabelMutex;


private: // Methods
    void addServerInfo();

    void setPalettes();
    void addLayouts();
    void addMenu();
    void addStatusLable(const char*);
    void addUserInput();
    void addMessagesLabel();
    void startConnection();
    void onConnection(const char*);

private slots:
    void getServerInfo();
    void connectionStatus(const char*);
    void sendingMessages();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QSize sizeHint() const;
};
#endif // CLIENT_MAINWINDOW_H
