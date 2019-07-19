#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QString>

const quint16 DiscoveryPort = 1999;
const QString DiscoveryWorld = "RECOGNIZE";
const int ConnectionTimeout = 1000;
const int UpdateInterval = 200; // ms

extern QString ClientNAME;
extern QString ClientWALLET;
extern QString DownloadFolder;
#endif // CONSTANTS_H
