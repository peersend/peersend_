#include "TcpSocket.h"
#include <QApplication>
#include "constants.h"

TcpSocket::TcpSocket(QTcpSocket * socket)
  : QObject()
  , m_transmittingFile(NULL)
  , m_receivedFile(NULL)
  , m_currentReceivedFileSize(0)
  , m_reconnectToServer(false)
  , m_cancelState(eUndefined)
  , m_soket(socket)
  , is_first_message(true)
  , m_needToWrite(0)
{
    m_soket->socketOption(QAbstractSocket::LowDelayOption);
    socket->setParent(this);
    connect(m_soket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    connect(m_soket, SIGNAL(readyRead()),    this, SLOT(onReceivedData()));
    connect(this, SIGNAL(fileTransmitted()), this, SLOT(onFileTransmitted()));
    connect(this, SIGNAL(packageSended()), this, SLOT(onPckegeSended()));
    connect(this, SIGNAL(packageListUpdated()), this, SLOT(onPckegeSended()));
    connect(m_soket, SIGNAL(bytesWritten(qint64)), this, SLOT(onBytesWriten(qint64)));

    m_totalToTransmittedCount=0;

    m_currentTransmittedFileSize=0;
    m_currentTrunsmittedFileTransmitted=0;
    m_currentTrunsmittedFileTransmitted_1=0;

    m_totalReceivedCount=0;

    m_currentReceivedFileSize=0;
    m_currentReceivedFileTransmitted=0;
    m_currentReceivedFileTransmitted_1=0;

    m_recalcTimer.setSingleShot(false);
    m_recalcTimer.setInterval(UpdateInterval);
    connect(&m_recalcTimer, SIGNAL(timeout()), this, SLOT(onRecalc()));
    m_recalcTimer.start();
}

TcpSocket::~TcpSocket() {
    m_soket->disconnect(this, 0, 0, 0);
    m_soket->close();
    //m_soket->deleteLater();
}

void TcpSocket::onSocketDisconnected()
{
    m_info.clear();
    m_buffer.clear();
    delete m_receivedFile;
    m_currentReceivedFileSize = 0;

    if (m_reconnectToServer) {
    }
    else {
        emit socketDiskonnected();
    }
}

void TcpSocket::onFileTransmitted()
{
    delete m_transmittingFile;

    if(m_filesToTransmitt.isEmpty())
        return;

    QFileInfo filePath = m_filesToTransmitt.first();
    m_filesToTransmitt.pop_front();

    m_transmittingFile = new QFile(filePath.absoluteFilePath());
    if (m_transmittingFile->open(QIODevice::ReadOnly)) {
        QString fileName = filePath.fileName();
        QPair<QByteArray, int> nextName = qMakePair<QByteArray, int> (fileName.toUtf8(), eFileName);
        m_packsBuffer.append(nextName);
        QPair<QByteArray, int> nextSize = qMakePair<QByteArray, int> (QByteArray::number(filePath.size()), eFileSize);
        m_packsBuffer.append(nextSize);
        emit packageListUpdated();
    }
    else {
        delete m_transmittingFile;
        m_transmittingFile = NULL;
    }
}

void TcpSocket::onPckegeSended()
{
    if(m_packsBuffer.isEmpty()) {
        if(!m_transmittingFile) {
            emit fileTransmitted();
            return;
        }
        QByteArray data = m_transmittingFile->read(8192);
        if (data.isEmpty()) {
            emit log(QString(tr("transmitted %1")).arg(m_transmittingFile->fileName()));
            m_transmittingFile->close();
            delete m_transmittingFile;
            m_transmittingFile = NULL;
            emit fileTransmitted();
        }
        else {
            m_packsBuffer.append(qMakePair<QByteArray, int> (data, eFile));
            emit packageListUpdated();
        }
    }
    else {
        QPair <QByteArray, int> package = m_packsBuffer.first();
        m_packsBuffer.pop_front();
        writeData(package.first, package.second);
    }
}

void TcpSocket::onBytesWriten(qint64 writen)
{
    m_needToWrite -= writen;
    if(m_needToWrite == 0)
        emit packageSended();
}

void TcpSocket::parseBuffer()
{
    DataHeader * header = (DataHeader*)(m_buffer.constData());
    while(header->size + sizeof(DataHeader) <= m_buffer.size()) {
        memcpy(&m_info, header, sizeof(DataHeader));
        m_buffer.remove(0, sizeof(DataHeader));

        switch (m_info.m_type)
        {
        case eMessage:
            onMessage();
            break;

        case eFile:
            onFile();
            break;

        case eFileName:
            onFileName();
            break;

        case eFileSize:
            onFileSize();
            break;

        case eCancelTransfer:
            onCancelTransfer();
            return;

        case eFileNameInQueue:
            onFileNameInQueue();
            return;
        default:
            //emit log("Incorrect header");
            break;
        }

        header = (DataHeader*)(m_buffer.constData());
    }
}

void TcpSocket::onRecalc()
{
    emit receivingProgress(m_currentReceivedFileTransmitted, m_currentReceivedFileSize);
    emit downloadSpeed((m_currentReceivedFileTransmitted - m_currentReceivedFileTransmitted_1)*1000/UpdateInterval);
    m_currentReceivedFileTransmitted_1 = m_currentReceivedFileTransmitted;

    emit transmittingProgress(m_currentTrunsmittedFileTransmitted, m_currentTransmittedFileSize);
    emit uploadSpeed((m_currentTrunsmittedFileTransmitted - m_currentTrunsmittedFileTransmitted_1)*1000/UpdateInterval);
    m_currentTrunsmittedFileTransmitted_1 = m_currentTrunsmittedFileTransmitted;


    if(m_totalToTransmittedCount != 0 &&  m_filesToTransmitt.size() ==  0) {
        m_totalToTransmittedCount = 0;
        m_filesToTransmitt.clear();
    }
    emit totalTransmittingProgress(m_totalToTransmittedCount - m_filesToTransmitt.size(), m_totalToTransmittedCount);


    int ReceivedSize = m_filesToReceive.size();

    if(ReceivedSize != 0 && m_totalReceivedCount == ReceivedSize ) {
        m_totalReceivedCount = 0;
        m_filesToReceive.clear();
    }
    emit totalReceivingProgress(m_totalReceivedCount,ReceivedSize);
}

void TcpSocket::closeSocket()
{
    m_soket->close();
    qDebug() << "socket Closed";
}

bool TcpSocket::isConnected()
{
    return m_soket->state() == QAbstractSocket::ConnectedState;
}

void TcpSocket::onReceivedData()
{
    QApplication::processEvents();

    m_buffer.append(m_soket->readAll());

    if (m_buffer.isEmpty()) {
        return;
    }

    if (m_info.m_type == eUndefined) {
        if ((size_t)m_buffer.size() < sizeof(DataHeader)) {
            return;
        }

        DataHeader * header = (DataHeader*)(m_buffer.constData());
        if(header->size + sizeof(DataHeader) <= m_buffer.size())
            parseBuffer();
    }
}

void TcpSocket::onCancelTransfer()
{
    writeChunkToFile(QByteArray());
}

void TcpSocket::writeChunkToFile(const QByteArray data)
{
    if (!m_receivedFile) {
        return;
    }

    if (!data.isEmpty()) {
        qint64 writeSize = m_receivedFile->write(data);
        if (writeSize != data.size()) {
            // report error and stop receiving file
            emit log("Can't write to file");
            return;
        }

        m_currentReceivedFileTransmitted += data.size();
    }

    if (m_currentReceivedFileSize <= m_currentReceivedFileTransmitted) { // а это как? что за бонус?

        m_currentReceivedFileSize = 0;
        m_currentReceivedFileTransmitted = 0;

        quint64 m_totalToReceiveCount;

        m_receivedFile->flush();

        emit log(QString(tr("received %1")).arg(m_filesToReceive.at(m_totalReceivedCount++)));
        QFileInfo fileinfo(*m_receivedFile);
        QString path = fileinfo.absoluteFilePath();

        if (!data.isEmpty()) {
            emit receivedFile(path);
        }
        else {

            //m_receivedFile->remove();
        }

        delete m_receivedFile;
        m_receivedFile = NULL;
    }
}

void TcpSocket::onFile()
{
    if (!m_receivedFile) {
        m_buffer.remove(0, m_info.size);
        m_info.clear();
        return;
    }

    if (!m_receivedFile->isOpen()) {
        if (!m_receivedFile->open(QIODevice::Append)) {
            qWarning() << "Can't open file for write";
            m_buffer.remove(0, m_info.size);
            m_info.clear();
            return;
        }
    }

    int buffSize = m_info.size;
    writeChunkToFile(QByteArray(m_buffer.constData(), buffSize));

    m_info.clear();
    m_buffer.remove(0, buffSize);
}

void TcpSocket::onFileName()
{
    if (m_downloadingFolder.isEmpty()) {
        qWarning() << "Download folder not set";
        return;
    }

    if (m_receivedFile) {
        delete m_receivedFile;
        m_receivedFile = NULL;
    }

    if (m_buffer.size() < m_info.size) {
        // wait for data
        // need restore header
        m_buffer.prepend((const char *)&m_info, sizeof(DataHeader));
        m_info.clear();
        return;
    }

    const QString fileName = QString::fromUtf8(m_buffer.constData(), m_info.size);
    m_buffer.remove(0, m_info.size);
    m_info.clear();

    QString absPath = m_downloadingFolder + QDir::separator() + fileName;
    if (QFile::exists(absPath)) {
        // todo report error and stop receiving file
//        qWarning() << "File already exists";
        //return;
        absPath = findNextFileName(absPath);
    }

    m_receivedFile = new QFile(absPath);
    if (!m_receivedFile->open(QIODevice::Append)) {
        qWarning() << "Can't open file for write";
        return;
    }
}

void TcpSocket::onFileSize()
{
    QByteArray aSize = m_buffer.left(m_info.size);
    m_currentReceivedFileSize = aSize.toULongLong();
    m_buffer.remove(0, m_info.size);
    m_info.clear();
}

void TcpSocket::onFileNameInQueue()
{
    const QString fileName = QString::fromUtf8(m_buffer.constData(), m_info.size);
    m_filesToReceive.append(fileName);
    m_buffer.remove(0, m_info.size);
    m_info.clear();
}

void TcpSocket::onMessage()
{
    const QString msg = QString::fromUtf8(m_buffer.constData(), m_info.size);
    if (m_info.size <= 0) {
        // todo
    }

    m_buffer.remove(0, m_info.size);
    m_info.clear();
    // первым сообщением приходит имя и кошелек, для отображения в табе
    if(is_first_message) {
        is_first_message = false;
        emit clientName(msg);
    }
    else {
        emit receivedMessage(msg);
    }
}

void TcpSocket::setDownloadingFolder(const QString & pathToFolder)
{
    m_downloadingFolder = pathToFolder;
}

void TcpSocket::sendFiles(const QStringList & filesPath)
{
    for (int i = 0; i < filesPath.size(); ++i) {
        sendFile(filesPath.at(i));
    }
}

void TcpSocket::sendFile(const QString & filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fInfo(filePath);
    if (!fInfo.exists()) {
        qWarning() << "File not exist: " + fInfo.absoluteFilePath();
        return;
    }

    m_totalToTransmittedCount++;
    m_filesToTransmitt.append(fInfo);
    m_packsBuffer.append(qMakePair<QByteArray, int>(fInfo.fileName().toUtf8(), eFileNameInQueue));
    //emit packageListUpdated();

    if(m_filesToTransmitt.size() == 1)
        emit packageListUpdated();
}

void TcpSocket::sendMessage(const QString & message)
{
    return writeData(message.toUtf8(), eMessage);
}

void TcpSocket::setReconnectToServer(bool reconnectToServer)
{

}

void TcpSocket::setReconnectTimerInterval(int interval)
{

}

void TcpSocket::cancelTransfer()
{
    m_cancelState = eCancelTransfer;
    this->deleteLater();
}

void TcpSocket::writeData(const QByteArray & data, int type)
{
    QApplication::processEvents();
    if (m_soket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Socket not connected";
        return;
    }

    if (m_cancelState == eCancelTransfer) {
        type = eCancelTransfer;
    }

    if (data.isEmpty()) {
        qWarning() << "Sending data is empty";
        return;
    }
    if(type == eFileSize) {
        m_currentTransmittedFileSize = data.toULongLong();
        m_currentTrunsmittedFileTransmitted = 0;
        m_currentTrunsmittedFileTransmitted_1 = 0;
    }

    DataHeader header(data.size(), type);
    QByteArray byteHeader((const char *)&header, sizeof(DataHeader));
    m_needToWrite += byteHeader.size();
    m_soket->write(byteHeader);
    m_needToWrite += data.size();
    m_soket->write(data);
// switch для отладки
    switch (type) {
        case eFile:
            m_currentTrunsmittedFileTransmitted += data.size();
            if(m_currentTrunsmittedFileTransmitted == m_currentTransmittedFileSize) {
                m_currentTrunsmittedFileTransmitted =0;
                m_currentTransmittedFileSize = 0;
            }
            break;
        case eFileNameInQueue:
            m_totalReceivedCount++;
        case eMessage:
        case eFileName:
        case eFileSize:
        case eCancelTransfer:
            return;
        default:
            break;
        }
}
//--------------------------------------------------------------------------------------------------
QString TcpSocket::findNextFileName(QString aFile)
{
    QFileInfo fi(aFile);
    QString aDir = fi.absolutePath();
    QString aName = fi.baseName();
    QString aExt = fi.fileName();
    aExt = aExt.mid(aName.size());

    int index = 0;
    QString newPath;
    do {
        newPath = QString("%1/%2(%3)%4")
            .arg(aDir)
            .arg(aName)
            .arg(++index)
            .arg(aExt);
    } while(QFile::exists(newPath));

    return newPath;
}
