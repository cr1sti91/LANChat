#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

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

private: // Atributes
    QAction*     quitAction;
    QAction*     listenAction;
    QMenu*       fileMenu;
    QMenu*       listenMenu;
    QLabel*      connectionStatusLabel;
    QLineEdit*   userInput;
    QWidget*     centralWidget;

    QVBoxLayout* layout;

    QString                   serverIP;
    boost::shared_ptr<Server> server;
    boost::thread*            serverThread;


private: // Methods
    void addMenu();
    void addStatusLable();
    void addUserInput();


private slots:
    void setStatusLabel(boost::shared_ptr<boost::asio::ip::tcp::endpoint>);
    void connectionStatus(const QString&);


signals:
    void serverListening();


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QSize sizeHint() const;
};
#endif // CLIENT_MAINWINDOW_H
