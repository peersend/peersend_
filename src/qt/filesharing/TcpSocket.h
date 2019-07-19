#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QtNetwork>
#include <QAtomicInt>
#include <QFile>
#include <QPair>

class TcpSocket: public QObject
{
    Q_OBJECT

public:
    TcpSocket(QTcpSocket * socket);
    virtual ~TcpSocket();

public slots:
    bool isConnected();

    void setDownloadingFolder(const QString & pathToFolder);
    void sendFile(const QString & filePath);
    void sendFiles(const QStringList & filesPath);
    void sendMessage(const QString & message);

    void setReconnectToServer(bool reconnectToServer);
    void setReconnectTimerInterval(int interval);

    void cancelTransfer();
    void closeSocket();

private slots:
    void onReceivedData();
    void onSocketDisconnected();

    void onFileTransmitted();
    void onPckegeSended();
    void onBytesWriten(qint64);

    void parseBuffer();
    void onRecalc();

    QString findNextFileName(QString);

signals:
    void transferCanceled();

    void transmittingProgress(qint64 current, qint64 total);
    void receivingProgress(qint64 current, qint64 total);
    void totalTransmittingProgress(qint64 current, qint64 total);
    void totalReceivingProgress(qint64 current, qint64 total);


    void receivedFile(const QString path);

    void receivedMessage(const QString message);
    void downloadSpeed(const int speed);
    void uploadSpeed(const int speed);
    void socketDiskonnected();
    void clientName(const QString&);

    void log(QString);

signals: // for internal use
    void packageListUpdated();
    void transmittFilesListUpdated();
    void fileTransmitted();
    void packageSended();

private:
    enum {
        eUndefined       = 0,
        eMessage,        //1
        eFile,           //2
        eFileSize,       //3
        eFileName,       //4
        eFileNameInQueue,//5
        eCancelTransfer  //6
    };

    typedef struct _DataHeader
    {
        int size;
        int m_type;

        _DataHeader() : size(0), m_type(eUndefined) {}
        _DataHeader(int size, int type) : size(size), m_type(type) {}

        void clear() { size = 0, m_type = eUndefined; }
    } DataHeader;

    void onCancelTransfer();
    void onMessage();
    void onFile();
    void onFileName();
    void onFileSize();
    void onFileNameInQueue();
    bool sendFile(const QFileInfo & filePath);
    void writeData(const QByteArray & data, int type);
    void writeChunkToFile(const QByteArray data);

    QByteArray m_buffer;

    QFile *m_transmittingFile;
    QFile * m_receivedFile;
    DataHeader m_info;

    quint64 m_totalToTransmittedCount;

    quint64 m_currentTransmittedFileSize;
    quint64 m_currentTrunsmittedFileTransmitted;
    quint64 m_currentTrunsmittedFileTransmitted_1;

    quint64 m_totalReceivedCount;

    quint64 m_currentReceivedFileSize;
    quint64 m_currentReceivedFileTransmitted;
    quint64 m_currentReceivedFileTransmitted_1;

    QString m_downloadingFolder;
    bool m_reconnectToServer;

    QAtomicInt m_cancelState;

    QList<QPair <QByteArray, int> > m_packsBuffer;
    QList<QFileInfo> m_filesToTransmitt;
    QList<QString> m_filesToReceive;
    QTcpSocket *m_soket;
    bool is_first_message;

    qint64 m_needToWrite;

    QTimer m_recalcTimer;
};

#endif // TCPSOCKET_H
