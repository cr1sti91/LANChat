#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QLabel>

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
#endif // MAINWINDOW_H
