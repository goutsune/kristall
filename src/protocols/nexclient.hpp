#ifndef NEXCLIENT_HPP
#define NEXCLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QUrl>
#include <QMimeDatabase>
#include <QMimeType>

#include "protocolhandler.hpp"

class NexClient : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit NexClient();

    ~NexClient() override;

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url, RequestOptions options) override;

    bool isInProgress() const override;

    bool cancelRequest() override;

private slots:
    void on_connected();
    void on_readRead();
    void on_finished();
    void on_socketError(QTcpSocket::SocketError error_code);

private:
    QUrl target_url;
    QTcpSocket socket;
    QByteArray body;
    bool was_cancelled;
    QMimeDatabase db;
    QString guessed_type;
};

#endif // NEXCLIENT_HPP
