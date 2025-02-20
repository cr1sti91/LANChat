#include "client_mainwindow.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void CMainWindow::initWelcomeScreen()
{
    m_centralWidget  = new QWidget();
    m_welcomeLabel   = new QLabel("LANChat Client", m_centralWidget);
    m_verticalLayout = new QVBoxLayout(m_centralWidget);

    setCentralWidget(m_centralWidget);
    m_verticalLayout->addWidget(m_welcomeLabel);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);

    this->adjustFontSize(width() / 12);
}

void CMainWindow::adjustFontSize(const int& fontSize)
{
    if(m_welcomeLabel)
    {
        // %1 is replaced with the calculated value for the font size.
        m_welcomeLabel->setStyleSheet(QString("color: #7ed957; font-weight: bold; f"
                                      "ont-style: italic; text-decoration: underline; font-size: %1px;")
                                          .arg(fontSize));
    }
}

void CMainWindow::resetAtributes()
{
    if(m_welcomeLabel)
        m_welcomeLabel = nullptr;

    m_connectionStatusLabel = nullptr;
    m_messagesLabel         = nullptr;

    m_messagesLabelScroll   = nullptr;
    m_messagesLabelLayout   = nullptr;

    m_userInputLEdit        = nullptr;

    m_centralWidget         = nullptr;
    m_verticalLayout        = nullptr;
    m_orizontalLayout       = nullptr;

    m_sendButton            = nullptr;
    m_ipAddressLEdit        = nullptr;
    m_portLEdit             = nullptr;
    m_clientNameLEdit       = nullptr;
    m_connectButton         = nullptr;
}

void CMainWindow::addServerInfo()
{
    this->addLayouts();
    this->addStatusLable("Waiting for IP address and port...");

    m_clientNameLEdit = new QLineEdit(m_centralWidget);
    m_ipAddressLEdit  = new QLineEdit(m_centralWidget);
    m_portLEdit       = new QLineEdit(m_centralWidget);
    m_connectButton   = new QPushButton(m_centralWidget);

    m_connectButton->setPalette(*m_widgetsPalette);
    m_connectButton->setText("Connect");

    m_clientNameLEdit->setFixedHeight(30);
    m_ipAddressLEdit->setFixedHeight(30);
    m_portLEdit->setFixedHeight(30);
    m_connectButton->setFixedHeight(40);

    m_clientNameLEdit->setPlaceholderText("Type your nickname...");
    m_ipAddressLEdit->setPlaceholderText("Type the server IP address...");
    m_portLEdit->setPlaceholderText("Type the server port...");

    m_verticalLayout->addWidget(m_clientNameLEdit);
    m_verticalLayout->addWidget(m_ipAddressLEdit);
    m_verticalLayout->addWidget(m_portLEdit);
    m_verticalLayout->addWidget(m_connectButton);

    m_verticalLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}


void CMainWindow::addPalettes()
{
    m_widgetsPalette = new QPalette;

    // For the buttons
    m_widgetsPalette->setColor(QPalette::Button, Qt::blue);
    m_widgetsPalette->setColor(QPalette::ButtonText, Qt::black);

    // For the scroll bar
    m_widgetsPalette->setColor(QPalette::Base, Qt::blue);

    setWindowTitle(CMainWindow::WINDOWNAME);
}

void CMainWindow::addLayouts()
{
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    m_verticalLayout =  new QVBoxLayout(m_centralWidget);
}

void CMainWindow::addMenu()
{
    m_appMenu        = menuBar()->addMenu("App");
    m_connectionMenu = menuBar()->addMenu("Connection");
    m_optionsMenu    = menuBar()->addMenu("Options");

    m_quitAction          = new QAction("Quit", this);
    m_connectAction       = new QAction("New Connetion", this);
    m_clearMessagesAction = new QAction("Clear", this);

    m_appMenu->addAction(m_quitAction);
    m_connectionMenu->addAction(m_connectAction);
    m_optionsMenu->addAction(m_clearMessagesAction);

    connect(m_quitAction, &QAction::triggered, this, [this](){ QApplication::quit(); });
    connect(m_connectAction, &QAction::triggered, this, &CMainWindow::getServerInfo);
    connect(m_clearMessagesAction, &QAction::triggered, this, &CMainWindow::clearMessages);
}

void CMainWindow::addStatusLable(const char* status)
{
    m_connectionStatusLabel = new QLabel(status, m_centralWidget);

    QPalette labelPalette;

    labelPalette.setColor(QPalette::WindowText, [status](){
        if(!std::strcmp(status, "  Connected!"))
            return Qt::green;
        else if(!std::strcmp(status, "Waiting for IP address and port..."))
            return Qt::cyan;
        else
            return Qt::red;
    }());

    m_connectionStatusLabel->setPalette(labelPalette);

    m_connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_verticalLayout->addWidget(m_connectionStatusLabel, 1, Qt::AlignTop);
    m_connectionStatusLabel->show();
}

void CMainWindow::addUserInput()
{
    m_orizontalLayout = new QHBoxLayout();
    m_verticalLayout->addLayout(m_orizontalLayout);

    m_userInputLEdit = new QLineEdit(m_centralWidget);
    m_userInputLEdit->setPlaceholderText("Type...");

    m_orizontalLayout->addWidget(m_userInputLEdit);
    m_userInputLEdit->show();

    m_sendButton = new QPushButton(m_centralWidget);
    m_sendButton->setText("Send");

    m_sendButton->setPalette(*m_widgetsPalette);

    m_orizontalLayout->addWidget(m_sendButton);
    m_sendButton->show();


    // Messages are sent either when the 'm_sendButton' button or the Enter key is pressed.
    connect(m_sendButton, &QPushButton::clicked, this, &CMainWindow::sendingMessages);
    connect(m_userInputLEdit, &QLineEdit::returnPressed, this, &CMainWindow::sendingMessages);
}

void CMainWindow::addMessagesLabel()
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

void CMainWindow::startConnection()
{
    if(m_clientName.empty() || m_clientName == "SERVER")
    {
        this->setConnectionStatus("Invalid client name!");
        return;
    }

    // If the m_client is connected, to initiate a new connection, it is
    // necessary to close the previous one.
    if(m_client->is_working().has_value())
    {
        if(m_client->is_working())
        {
            if(m_clientThread != nullptr)
            {
                m_clientThread.reset(nullptr);
            }

            m_client->closeConnection();
        }
    }

    m_clientThread =
        std::make_unique<boost::thread>(boost::bind(&Client::connect,
                                                    m_client,
                                                    m_serverIPaddress.c_str(),
                                                    std::atoi(m_serverPort.c_str())
                                                    )
                                        );
}

void CMainWindow::onConnection(const char* status)
{
    delete m_centralWidget; // Upon this destruction all child widgets are destroyed
    this->resetAtributes();

    this->addLayouts();
    this->addStatusLable(status);
    this->addMessagesLabel();
    this->addUserInput();

    m_client->recv();

    connect(m_client, &Client::message_received, this, &CMainWindow::displayMessage);
}

void CMainWindow::displayMessage(const std::string &message)
{
    boost::lock_guard<boost::mutex> lckgrd(m_messageLabelMutex);

    m_messagesLabel->setText(m_messagesLabel->text() + QString::fromStdString(message));

    m_messagesLabel->adjustSize();

    // Set scroll bar in the bottom position
    m_messagesLabelScroll->verticalScrollBar()->setSliderPosition(
        m_messagesLabelScroll->verticalScrollBar()->maximum()
    );
}

void CMainWindow::cleanup()
{
    delete m_widgetsPalette;
}



//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///

void CMainWindow::getServerInfo()
{
    if(m_centralWidget && m_hasEverConnected)
    {
        disconnect(m_client, &Client::message_received, nullptr, nullptr);

        delete m_centralWidget;
        this->resetAtributes();

        this->addServerInfo();

        if(m_client->is_working().has_value())
        {
            if(m_client->is_working())
            {
                if(m_clientThread != nullptr)
                {
                    m_clientThread.reset(nullptr);
                }

                m_client->closeConnection();
            }
        }
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

        this->addServerInfo();
        connect(m_client, &Client::connectionStatus, this, &CMainWindow::setConnectionStatus);
    }

    connect(m_connectButton, &QPushButton::clicked, this, [this](){
        m_clientName      = m_clientNameLEdit->text().toStdString();
        m_serverIPaddress = m_ipAddressLEdit->text().toStdString();
        m_serverPort      = m_portLEdit->text().toStdString();

        this->startConnection();
    });
}


void CMainWindow::setConnectionStatus(const char* status)
{
    if(m_clientThread != nullptr)
    {
        // serverThread is deleted because the attempt to connect has finished.
        m_clientThread->join();
        m_clientThread.reset(nullptr);
        m_clientThread = nullptr;
    }

    if(!std::strcmp(status, "  Connected!"))
    {
        this->onConnection(status);
    }
    else
    {
        m_connectionStatusLabel->setText(status);

        QPalette labelPalette;
        labelPalette.setColor(QPalette::WindowText,
                              (!std::strcmp(status, "Waiting for IP address and port...")? Qt::cyan : Qt::red));
        m_connectionStatusLabel->setPalette(labelPalette);
    }
}

void CMainWindow::sendingMessages()
{
    std::string input = m_userInputLEdit->text().toStdString();
    m_userInputLEdit->clear();

    if(input.empty())
        return;

    input = std::string("<span style='color: green;'>") + m_clientName + ": </span>" + input + "<br>";

    std::copy(input.begin(), input.end(), std::back_inserter(m_send_buffer));

    m_client->send(m_send_buffer);
    m_send_buffer.clear();

    // Adding text to the messageLabel
    this->displayMessage(input);
}

void CMainWindow::clearMessages()
{
    if(m_messagesLabel)
        m_messagesLabel->clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PROTECTED METHODS
///
QSize CMainWindow::sizeHint() const
{
    return QSize(700, 500);
}

void CMainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if(!m_hasEverConnected)
        this->adjustFontSize(event->size().width() / 12);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent),
                                            m_client(new Client(this)),
                                            m_clientThread(nullptr),
                                            m_hasEverConnected(false)
{
    this->initWelcomeScreen();

    this->addPalettes();
    this->addMenu();
}

CMainWindow::~CMainWindow()
{
    // m_client disconnection
    m_client->finish();

    // Widget destruction
    if(m_centralWidget)
    {
        delete m_centralWidget;
        this->resetAtributes();
    }

    this->cleanup();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
