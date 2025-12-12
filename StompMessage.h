#pragma once

#ifndef BUILD_STATIC
# if defined(QTWEBSTOMPCLIENTDLL_LIB)
#  define QTWEBSTOMPCLIENTDLL_EXPORT Q_DECL_EXPORT
# else
#  define QTWEBSTOMPCLIENTDLL_EXPORT Q_DECL_IMPORT
# endif
#else
# define QTWEBSTOMPCLIENTDLL_EXPORT
#endif

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>

class QTWEBSTOMPCLIENTDLL_EXPORT StompMessage {
public:
	/* This constructor is not meant for the user, just for the dll*/
	StompMessage(const char* rawMessage);

	StompMessage(const QString& messageType, const QMap<QString, QString>& headers, const char* messageBody = "");

	/* Creates a string with the message in a readable format =)*/
	QString toString() const;

	enum MessageType { CONNECT, SUBSCRIBE, SEND, MESSAGE };

	QMap<QString, QString> m_headers;
	QString m_message;
	QString m_messageType;

private:

	QList<QString> messageToVector(const QString& str, const QString& delim);
};


