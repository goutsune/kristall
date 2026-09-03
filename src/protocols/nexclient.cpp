#include "nexclient.hpp"
#include "ioutil.hpp"
#include "kristall.hpp"

NexClient::NexClient() : ProtocolHandler(nullptr)
{
    connect(&socket, &QTcpSocket::connected, this, &NexClient::on_connected);
    connect(&socket, &QTcpSocket::readyRead, this, &NexClient::on_readRead);
    connect(&socket, &QTcpSocket::disconnected, this, &NexClient::on_finished);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    connect(&socket, &QTcpSocket::errorOccurred, this, &NexClient::on_socketError);
#else
    connect(&socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &NexClient::on_socketError);
#endif

    connect(&socket, &QAbstractSocket::hostFound, this, [this]() {
        emit this->requestStateChange(RequestState::HostFound);
    });
    emit this->requestStateChange(RequestState::None);
}

NexClient::~NexClient()
{

}

bool NexClient::supportsScheme(const QString &scheme) const
{
    return (scheme == "nex");
}

bool NexClient::startRequest(const QUrl &url, RequestOptions options)
{
    Q_UNUSED(options)

    if(isInProgress())
        return false;

    if(url.scheme() != "nex")
        return false;

    target_url = url;

    // Bah, NEX has no mime type support, so we are expected to guess one.
    // *.txt and */ become text/nex (to parse links), everything else handled by Qt
    QString suffix = target_url.path().section('.', -1).toLower();

    if (suffix == "txt" or  target_url.path().endsWith('/') or target_url.path().isEmpty())
        guessed_type = "text/x-nex";
    else {
        QMimeType type = db.mimeTypeForName(target_url.path());
        guessed_type = type.isValid() ? type.name() : "application/octet-stream";
    }

    this->was_cancelled = false;
    socket.connectToHost(url.host(), url.port(1900));

    return true;
}

bool NexClient::isInProgress() const
{
    return socket.isOpen();
}

bool NexClient::cancelRequest()
{
    was_cancelled = true;
    if (socket.state() != QTcpSocket::UnconnectedState)
    {
        socket.disconnectFromHost();
        this->socket.waitForDisconnected(500);
    }
    socket.close();
    body.clear();
    return true;
}

void NexClient::on_connected()
{
    QString request = target_url.path() + "\n";
    QByteArray request_bytes = request.toUtf8();

    IoUtil::writeAll(socket, request_bytes);

    emit this->requestStateChange(RequestState::Connected);
}

void NexClient::on_readRead()
{
    body.append(socket.readAll());
    emit this->requestProgress(body.size());
}

void NexClient::on_finished()
{
    if(not was_cancelled)
    {
        emit this->requestComplete(this->body, guessed_type);
        was_cancelled = true;
    }
    body.clear();

    emit this->requestStateChange(RequestState::None);
}

void NexClient::on_socketError(QAbstractSocket::SocketError error_code)
{
    // Same as GopherClient::on_SocketError. See there for explanation
    if (error_code == QAbstractSocket::RemoteHostClosedError) {
        socket.close();
        return;
    }
    this->emitNetworkError(error_code, socket.errorString());
}
