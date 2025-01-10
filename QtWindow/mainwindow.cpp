#include "mainwindow.h"
#include <QDebug>

void MainWindow::addMenu()
{
    quitAction = new QAction("Quit", this);

    connect(quitAction, &QAction::triggered, [this](){
        QApplication::quit();
    });

    fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(quitAction);
}

void MainWindow::addLabel()
{
    ipLabel = new QLabel(this);
    ipLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    ipLabel->setText("Remote host name or IP: ");
    ipLabel->setFixedWidth(ipLabel->fontMetrics().horizontalAdvance(ipLabel->text()) + 10);
    ipLabel->move(0, fileMenu->size().height());
}

void MainWindow::addLineEdit()
{
    ipLineEdit = new QLineEdit(this);
    ipLineEdit->setFixedWidth(ipLabel->width());
    ipLineEdit->setFixedHeight(ipLabel->height());
    ipLineEdit->move(ipLabel->width(), fileMenu->size().height());
}

void MainWindow::addButtons()
{
    connectButton = new QPushButton("Connect", this);
    connectButton->move(2 * ipLabel->width(), fileMenu->size().height());
}

void MainWindow::connect_connectButton()
{
    connect(connectButton, &QPushButton::clicked, this, [=](){
        remoteIP = ipLineEdit->text();

        // Making connection
    });
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->addMenu();
    this->addLabel();
    this->addLineEdit();
    this->addButtons();

    this->connect_connectButton();
}


QSize MainWindow::sizeHint() const
{
    return QSize(700, 500);
}
