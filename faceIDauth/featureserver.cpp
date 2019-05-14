//
// Created by test2 on 19-5-14.
//

#include "featureserver.h"
FeatureServer();
void Run();
void FeatureServer::OnConnection(const TcpConnectionPtr& conn);
void onMessage(const TcpConnectionPtr& conn, Buffer* msg, Timestamp timestamp);


void writeFile(const char* filename, const std::string & filecontent);