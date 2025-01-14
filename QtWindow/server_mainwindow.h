#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <cstring>
#include <vector>

#include <QMainWindow>
#include <QPalette>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QLabel>

#include "server.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    static inline const char* windowName = "Server LANChat";

private: // Fields
    QPalette*       windowPalette;
    QPalette*       buttonPalette;

    QAction*        quitAction;
    QAction*        listenAction;

    QMenu*          fileMenu;
    QMenu*          listenMenu;

    QLabel*         connectionStatusLabel;
    QLabel*         messagesLabel;

    QLineEdit*      userInput;

    QWidget*        centralWidget;
    QVBoxLayout*    verticalLayout;
    QHBoxLayout*    orizontalLayout;

    QPushButton*    sendButton;

    QString                   serverIP;
    boost::shared_ptr<Server> server;
    boost::thread*            serverThread;

    std::vector<boost::uint8_t> send_buffer;


private: // Methods
    void setPalettes();
    void addLayouts();
    void addMenu();
    void addStatusLable();
    void addUserInput();
    void addMessagesLabel();


private slots:
    void setStatusLabel(boost::shared_ptr<boost::asio::ip::tcp::endpoint>);
    void connectionStatus(const char *);


signals:
    void serverListening();


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QSize sizeHint() const;
};
#endif // CLIENT_MAINWINDOW_H
