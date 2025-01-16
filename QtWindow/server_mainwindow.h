#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include "server.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    static inline const char* WINDOWNAME = "Server LANChat";

private: // Fields
    QPalette*       windowPalette;
    QPalette*       buttonPalette;

    QAction*        quitAction;
    QAction*        listenAction;

    QMenu*          fileMenu;
    QMenu*          listenMenu;

    QLabel*         connectionStatusLabel;
    QLabel*         messagesLabel;

    QScrollArea*    messagesLabelScroll;
    QHBoxLayout*    messagesLabelLayout;

    QLineEdit*      userInput;

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
    void setPalettes();
    void addLayouts();
    void addMenu();
    void addStatusLable();
    void addUserInput();
    void addMessagesLabel();
    void startListening();
    void sendingMessages();


private slots:
    void setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint>);
    void connectionStatus(const char*);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    inline static size_t number = 0;
    QSize sizeHint() const;
};
#endif // CLIENT_MAINWINDOW_H
