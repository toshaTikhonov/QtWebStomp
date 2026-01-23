#include <QtCore/QCoreApplication>
#include "QTWebStompClientDll.h"
#include "StompMessage.h"

QTWebStompClient* myClient;

void onMessage(const StompMessage &s)
{
	qDebug() << "The message we got is\r\n" << s.toString();
	//myClient->Ack(s); // you can either specify the entire message as a parameter of ack or just the id you wanna ack.

	//myClient->Send("/topic/payment/90030171", "this is my infinite loop");
}

void onConnect()
{
	myClient->Subscribe("/topic/payment/90030171", onMessage, QTWebStompClient::ClientIndividual); // with type client-individual you'll have to ack every message that's received when you want it removed from the queue.
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	myClient = new QTWebStompClient("ws://192.168.10.128:8094/pay-websocket/websocket", "", "", onConnect, "", true);
	return a.exec();
}


