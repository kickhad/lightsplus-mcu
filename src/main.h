#ifndef MAIN_H
#define MAIN_H
#include "WiFi.h"
void setupVGA();
void setupRoutes();
void startServer();
void printWifiStatus();
void PrintIPtoVGA(IPAddress);
void webserverstuff();
#endif