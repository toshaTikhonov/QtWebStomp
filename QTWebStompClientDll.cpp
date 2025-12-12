#include "QTWebStompClientDll.h"
#include <QtCore/QDebug>

StompMessage::StompMessage(const char* rawMessage)
{
	QString strMessage(rawMessage);
	QList<QString> messageVector = messageToVector(strMessage, "\n");
	m_message = messageVector.back(); // deep copies
	messageVector.pop_back();
	bool first = true;
	for (const auto& header : messageVector)
	{
		int pos = header.indexOf(':', 0);
		QString key = header.mid(0, pos);
		if (first)
		{
			m_messageType = key;
			first = false;
			continue;
		}
		++pos;
		QString value = header.mid(pos, header.length() - pos);
		m_headers[key] = value;
	}
}

StompMessage::StompMessage(const QString& messageType, const QMap<QString, QString>& headers, const char* messageBody)
{
	m_messageType = messageType;
	m_message = QString(messageBody);
	m_headers = headers;
}

QString StompMessage::toString() const
{
	QString result = m_messageType + "\u000A";
	for (auto it = m_headers.constBegin(); it != m_headers.constEnd(); ++it)
	{
		result += it.key() + ":" + it.value() + "\u000A";
	}

	result += "\u000A" + m_message + "\u0000";

	return result;
}

QList<QString> StompMessage::messageToVector(const QString& str, const QString& delim)
{
	QList<QString> messageParts;
	int prev = 0, pos = 0;
	bool last = false;
	do
	{
		if (last)
		{
			QString message = str.mid(prev, str.length() - prev);
			messageParts.push_back(message);
			break;
		}

		pos = str.indexOf(delim, prev);
		if (pos == -1) pos = str.length();
		QString token = str.mid(prev, pos - prev);
		if (!token.isEmpty())
		{
			messageParts.push_back(token);
		}
		else
		{
			// If the token is empty the headers finished =) Just the body left!
			last = true;
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev <= str.length()); // I know but we have to do it at least once, so...
	return messageParts;
}


////////////////////////////////////////////

QTWebStompClient::QTWebStompClient(const char* url, const char* login, const char* passcode, void(*onConnected)(void), const char* vHost, bool debug, QObject *parent)
{
	QUrl myUrl{QString(url)};
	m_debug = debug;
	m_login = login;
	m_passcode = passcode;
	m_onConnectedCallback = onConnected;
	if (m_debug) {
		qDebug() << "Connecting to WebSocket server:" << url;
	}
	m_vHost = vHost;

	connect(&m_webSocket, &QWebSocket::connected, this, &QTWebStompClient::onConnected);


	connect(&m_webSocket, (&QWebSocket::sslErrors),
		this, &QTWebStompClient::onSslErrors);

	connect(&m_webSocket, &QWebSocket::disconnected, this, &QTWebStompClient::closed);
	m_webSocket.open(QUrl(url));
}


void QTWebStompClient::onConnected()
{
	if (m_debug) {
		qDebug() << "-----------------------" << Qt::endl << "Connected to Websocket!" << Qt::endl << "-----------------------" << Qt::endl;
	}
	connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &QTWebStompClient::onTextMessageReceived);

	m_connectionState = Connecting;
	QString connectFrame = "CONNECT\u000A{vHost}accept-version:1.2\u000Alogin:{Login}\u000Apasscode:{Passcode}\u000A\u000A\u0000";
	connectFrame.replace("{Login}", m_login);
	connectFrame.replace("{Passcode}", m_passcode);
	QString vHost = "";
	if (m_vHost) {
		vHost = "vHost:" + QString(m_vHost) + QString("\u000A");
	}

	connectFrame.replace("{vHost}", vHost);

	QString connectFrameMessageWithNullFix = QString(connectFrame.data(), connectFrame.size() + 1); // solves the null-terminator issue
	m_webSocket.sendTextMessage(connectFrameMessageWithNullFix);

	if (m_debug) {
		qDebug() << "Sent message" << connectFrameMessageWithNullFix;
	}
}

void QTWebStompClient::onTextMessageReceived(QString message)
{
	StompMessage stompMessage(message.toUtf8().constData());

	switch (m_connectionState) {

		case Connecting:
			if (m_debug) {
				qDebug() << "Connection response: " << stompMessage.toString();
			}

			if (stompMessage.m_messageType == "CONNECTED")
			{
				if (m_debug) {
					qDebug() << "--------------------" << Qt::endl << "Connected to STOMP!" << Qt::endl << "--------------------" << Qt::endl;
				}
				m_connectionState = Connected;
				if (this->m_onConnectedCallback == NULL)
				{
					qDebug() << "WARNING: No callback selected for connection";
					qFatal("No onConnect callback set!");
				}
				else
				{
					this->m_onConnectedCallback();
				}
			}
			else
			{
				if (m_debug)
				{
					qDebug() << "Message type CONNECTED expected, got " << stompMessage.m_messageType;
				}

				qFatal("Message type CONNECTED expected, got: %s", qPrintable(stompMessage.toString()));
			}
			break;

		case Subscribed:
			// TODO: Improve check, maybe different messages are allowed when subscribed
			if (stompMessage.m_messageType == "MESSAGE") {
				if (m_debug) {
					qDebug() << "Message received from queue!" << Qt::endl << stompMessage.toString();
				}

				m_onMessageCallback(stompMessage);
			}
			else {
				qFatal("Message type MESSAGE expected, got: %s. Message is: %s",
					qPrintable(stompMessage.m_messageType),
					qPrintable(stompMessage.toString()));
			}
			break;

		default:
			qFatal("Unsupported connection state");
			break;
	}

}

// TODO: Change to have multiple ids (so we can handle more than one subscription)
void QTWebStompClient::Subscribe(const char* queueName, void(*onMessageCallback)(const StompMessage &s), QTWebStompClient::AckMode ackMode)
{
	if (m_connectionState != Connected)
	{
		// For now, if you need to subscribe to 2 queues, you can create two instances of the client. Later an improvement would be to use the id variables of the underlying websocket lib.
		qFatal("Cannot subscribe when connection hasn't finished or when already subscribed. Try using the callback function for onConnect to subscribe");
	}
	QMap<QString, QString> headers;
	headers["id"] = "0";
	headers["destination"] = QString(queueName);
	switch (ackMode) {
		case Client:
			headers["ack"] = "client";
			break;

		case ClientIndividual:
			headers["ack"] = "client-individual";
			break;

		default:
			headers["ack"] = "auto";
			break;
	}

	StompMessage myMessage("SUBSCRIBE", headers, "");

	QString subscribeMessage = myMessage.toString();
	QString subscribeFrame = QString(subscribeMessage.data(), subscribeMessage.size() + 1);

	m_webSocket.sendTextMessage(subscribeFrame);
	m_connectionState = Subscribed;
	m_onMessageCallback = onMessageCallback;
}

void QTWebStompClient::onSslErrors(const QList<QSslError> &errors)
{
	qFatal("SSL error! I'd show the error description if there were an easy way to convert from enum to string in c++. You'll have to debug.");
}

void QTWebStompClient::closed()
{
	qDebug() << "Connection closed =(";
	qFatal("Underlying connection unexpectedly closed =(");
}

void QTWebStompClient::Ack(const StompMessage & s)
{
	QString ack = s.m_headers.value("ack");
	Ack(ack.toUtf8().constData());
}

void QTWebStompClient::Ack(const char* id)
{
	if (m_debug) {
		qDebug() << "Acking message with id: " << id;
	}
	// Yes I know that this is disgusting
	QString ackFrame("ACK\u000Aid:{{TheAckId}}\u000A\u000A\u000A\u0000");
	ackFrame.replace("{{TheAckId}}", id);
	QString ackFrameNullFixed(ackFrame.data(), ackFrame.size() + 1); // solves the null-terminator issue
	m_webSocket.sendTextMessage(ackFrameNullFixed);
}

void QTWebStompClient::Send(const StompMessage & stompMessage)
{
	if (m_debug) {
		qDebug() << "Sending message: " << stompMessage.toString();
	}

	QString sendFrame = stompMessage.m_messageType + "\u000A";
	for (auto it = stompMessage.m_headers.constBegin(); it != stompMessage.m_headers.constEnd(); ++it)
	{
		sendFrame += it.key() + ":" + it.value() + "\u000A";
	}

	sendFrame += "\u000A" + stompMessage.m_message + "\u0000";

	QString sendFrameMessage(sendFrame.data(), sendFrame.size() + 1);
	m_webSocket.sendTextMessage(sendFrameMessage);
}

void QTWebStompClient::Send(const char* destination, const char* message, const QMap<QString, QString> &headers)
{
	QMap<QString, QString> headersWithDestination = headers;
	headersWithDestination[QString("destination")] = QString(destination);
	StompMessage s("SEND", headersWithDestination, message);
	Send(s);
}