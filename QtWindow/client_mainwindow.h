#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QLabel>

#include "client.h"

#include <boost/shared_ptr.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private: // Atributes
    QAction*     quitAction;
    QMenu*       fileMenu;
    QLabel*      ipLabel;
    QLineEdit*   ipLineEdit;
    QPushButton* connectButton;

    QString remoteIP;

    boost::shared_ptr<Client> client;

private: // Methods
    void addMenu();
    void addLabel();
    void addLineEdit();
    void addButtons();

    void connect_connectButton();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

    QSize sizeHint() const;
};
#endif // CLIENT_MAINWINDOW_H
