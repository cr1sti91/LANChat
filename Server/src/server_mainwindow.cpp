#include "server_mainwindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void SMainWindow::initWelcomeScreen()
{
    m_centralWidget  = new QWidget();
    m_welcomeLabel   = new QLabel("LANChat Server", m_centralWidget);
    m_verticalLayout = new QVBoxLayout(m_centralWidget);

    setCentralWidget(m_centralWidget);
    m_verticalLayout->addWidget(m_welcomeLabel);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);

    this->adjustFontSize(width() / 12);
}

void SMainWindow::adjustFontSize(const int &fontSize)
{
    if(m_welcomeLabel)
    {
        // %1 is replaced with the calculated value for the font size.
        m_welcomeLabel->setStyleSheet(QString("color: #ff5733; font-weight: bold; font-style: italic;"
                                              " text-decoration: underline; font-size: %1px;")
                                            .arg(fontSize));
    }
}

void SMainWindow::resetAtributes()
{
    if(m_welcomeLabel)
        m_welcomeLabel = nullptr;

    m_connectionStatusLabel = nullptr;
    m_messagesLabel         = nullptr;

    m_messagesLabelScroll   = nullptr;
    m_messagesLabelLayout   = nullptr;

    m_userInputLine         = nullptr;

    m_centralWidget         = nullptr;
    m_verticalLayout        = nullptr;
    m_orizontalLayout       = nullptr;

    m_sendButton            = nullptr;
}

void SMainWindow::setPalettes()
{
    m_widgetsPalette = new QPalette;

    // For the buttons
    m_widgetsPalette->setColor(QPalette::Button, Qt::blue);
    m_widgetsPalette->setColor(QPalette::ButtonText, Qt::black);

    // For the scroll area
    m_widgetsPalette->setColor(QPalette::Base, Qt::blue);

    setWindowTitle(SMainWindow::WINDOWNAME);
}

void SMainWindow::addLayouts()
{
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    m_verticalLayout =  new QVBoxLayout(m_centralWidget);
}

void SMainWindow::addMenu()
{
    m_appMenu     = menuBar()->addMenu("App");
    m_listenMenu  = menuBar()->addMenu("Connection");
    m_optionsMenu = menuBar()->addMenu("Options");

    m_quitAction          = new QAction("Quit", this);
    m_listenAction        = new QAction("Listen", this);
    m_clearMessagesAction = new QAction("Clear", this);

    m_appMenu->addAction(m_quitAction);
    m_listenMenu->addAction(m_listenAction);
    m_optionsMenu->addAction(m_clearMessagesAction);

    connect(m_quitAction, &QAction::triggered, this, [this](){ QApplication::quit(); });
    connect(m_listenAction, &QAction::triggered, this, &SMainWindow::startListening);
    connect(m_clearMessagesAction, &QAction::triggered, this, &SMainWindow::clearMessages);
}

void SMainWindow::addStatusLable()
{
    m_connectionStatusLabel = new QLabel(m_centralWidget);
    m_connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // To be able to copy the address and port, being selected
    m_connectionStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                                     Qt::TextSelectableByKeyboard);

    m_verticalLayout->addWidget(m_connectionStatusLabel, 1, Qt::AlignTop);
    m_connectionStatusLabel->show();


    connect(m_server, &Server::listening_on, this, &SMainWindow::setStatusLabel);
}

void SMainWindow::addUserInput()
{
    m_orizontalLayout = new QHBoxLayout();
    m_verticalLayout->addLayout(m_orizontalLayout);

    m_userInputLine = new QLineEdit(m_centralWidget);
    m_userInputLine->setPlaceholderText("Type...");

    m_orizontalLayout->addWidget(m_userInputLine);
    m_userInputLine->show();

    m_sendButton = new QPushButton(m_centralWidget);
    m_sendButton->setText("Send");

    m_sendButton->setPalette(*m_widgetsPalette);

    m_orizontalLayout->addWidget(m_sendButton);
    m_sendButton->show();

    // Messages are sent either when the 'm_sendButton' button or the Enter key is pressed.
    connect(m_sendButton, &QPushButton::clicked, this, &SMainWindow::sendingMessages);
    connect(m_userInputLine, &QLineEdit::returnPressed, this, &SMainWindow::sendingMessages);
}

void SMainWindow::addMessagesLabel()
{
    m_messagesLabel = new QLabel(m_centralWidget);
    m_messagesLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_messagesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_messagesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    m_messagesLabel->setWordWrap(true);
    m_messagesLabel->setAlignment(Qt::AlignBottom);

    m_messagesLabelScroll = new QScrollArea(m_centralWidget);
    m_messagesLabelScroll->setWidget(m_messagesLabel);
    m_messagesLabelScroll->setWidgetResizable(true);
    m_messagesLabelScroll->setPalette(*m_widgetsPalette);

    m_messagesLabelLayout = new QHBoxLayout();
    m_messagesLabelLayout->addWidget(m_messagesLabelScroll);

    m_verticalLayout->addLayout(m_messagesLabelLayout, Qt::AlignCenter);
}

void SMainWindow::startListening()
{
    // If the m_server is connected, to initiate a new connection, it is
    // necessary to close the previous one.
    if(m_server->is_working().has_value() && m_server->is_working())
    {
        disconnect(m_server, &Server::message_received, nullptr, nullptr);

        if(m_centralWidget)
        {
            delete m_centralWidget;
            resetAtributes();
        }

        if(m_serverThread != nullptr)
        {
            m_serverThread.reset(nullptr);
        }

        m_server->closeConnection();

    }
    else
    {
        m_hasEverConnected = true;

        //Deleting the central widget to create a new one.
        if(m_centralWidget)
        {
            delete m_centralWidget;
            this->resetAtributes();
        }

        connect(m_server, &Server::connectionStatus, this, &SMainWindow::connectionStatus);
    }

    m_serverThread = std::make_unique<boost::thread>(&Server::listen, m_server);
    this->addLayouts();
    this->addStatusLable();
}

void SMainWindow::sendingMessages()
{
    std::string input = m_userInputLine->text().toStdString();
    m_userInputLine->clear();

    if(input.empty())
        return;

    input = "<span style='color: red;'>SERVER: </span>" + input + "<br>";

    std::copy(input.begin(), input.end(), std::back_inserter(m_send_buffer));

    m_server->send(m_send_buffer);
    m_send_buffer.clear();

    // Adding text to the messageLabel
    this->displayMessage(input);
}

void SMainWindow::displayMessage(const std::string &message)
{
    boost::lock_guard<boost::mutex> lckgrd(m_messageLabelMutex);

    m_messagesLabel->setText(m_messagesLabel->text() + QString::fromStdString(message));

    // Adjust the widget size to fit the new content.
    // This ensures that the text is fully visible and that the scrollbar will adjust accordingly.
    m_messagesLabel->adjustSize();

    // Set scroll bar in the bottom position
    m_messagesLabelScroll->verticalScrollBar()->setValue(
        m_messagesLabelScroll->verticalScrollBar()->maximum()
    );
}

void SMainWindow::cleanup()
{
    delete m_widgetsPalette;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void SMainWindow::setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "  Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";

    QPalette statusLabelPalette;
    statusLabelPalette.setColor(QPalette::WindowText, Qt::cyan);
    m_connectionStatusLabel->setPalette(statusLabelPalette);

    m_connectionStatusLabel->setText(ipAndPort);

    // Disconnection from signal
    disconnect(m_server, &Server::listening_on, this, &SMainWindow::setStatusLabel);
}

void SMainWindow::connectionStatus(const char* status)
{
    if(m_serverThread != nullptr)
    {
        // m_serverThread is deleted because the attempt to connect has finished.
        m_serverThread->join();
        m_serverThread.reset(nullptr);
        m_serverThread = nullptr;
    }

    QPalette labelPalette;
    m_connectionStatusLabel->setText(status);

    labelPalette.setColor(QPalette::WindowText, (!std::strcmp(status, "  Connected!"))? Qt::green : Qt::red);
    m_connectionStatusLabel->setPalette(labelPalette);

    if(!std::strcmp(status, "  Connected!"))
    {
        this->addMessagesLabel();
        this->addUserInput();

        m_server->recv();

        connect(m_server,&Server::message_received, this, &SMainWindow::displayMessage);
    }
}

void SMainWindow::clearMessages()
{
    if(m_messagesLabel)
        m_messagesLabel->clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PROTECTED METHODS
///
QSize SMainWindow::sizeHint() const
{
    return QSize(700, 500);
}

void SMainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if(!m_hasEverConnected)
        this->adjustFontSize(event->size().width() / 12);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
SMainWindow::SMainWindow(QWidget *parent) : QMainWindow(parent),
                                            m_server(new Server(this)),
                                            m_serverThread(nullptr),
                                            m_hasEverConnected(false)
{
    this->initWelcomeScreen();

    this->setPalettes();
    this->addMenu();
}

SMainWindow::~SMainWindow()
{
    m_server->finish();

    if(m_centralWidget)
    {
        delete m_centralWidget;
        this->resetAtributes();
    }

    this->cleanup();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
