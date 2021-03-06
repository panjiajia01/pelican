/*
 * Copyright (c) 2013, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "output/test/TCPConnectionManagerTest.h"
#include "output/TCPConnectionManager.h"
#include "comms/DataSupportRequest.h"
#include "comms/DataSupportResponse.h"
#include "comms/ServerRequest.h"
#include "comms/ServerResponse.h"
#include "comms/DataBlobResponse.h"
#include "comms/StreamDataRequest.h"
#include "comms/PelicanClientProtocol.h"
#include "data/test/TestDataBlob.h"

#include <QtNetwork/QTcpSocket>
#include <QtCore/QCoreApplication>

#include <unistd.h>
#include <iostream>

using namespace std;

namespace pelican {

using test::TestDataBlob;

CPPUNIT_TEST_SUITE_REGISTRATION( TCPConnectionManagerTest );

TCPConnectionManagerTest::TCPConnectionManagerTest()
: CppUnit::TestFixture(), _server(0), _clientProtocol(0)
{
}

TCPConnectionManagerTest::~TCPConnectionManagerTest()
{
}

void TCPConnectionManagerTest::setUp()
{
    _clientProtocol = new PelicanClientProtocol;
    _clientProtocol->setTimeout(200); // 200 ms
    // 0 = let QTcpServer choose the port
    _server = new TCPConnectionManager(0);
}

void TCPConnectionManagerTest::tearDown()
{
    delete _server;
    delete _clientProtocol;
}


void TCPConnectionManagerTest::test_send()
{
    QString streamName = "testData";

    // Use Case:
    //   Client requests a connection.
    // Expect:
    //   Client to be registered for any data.
    StreamDataRequest request;
    DataSpec dataSpec;

    dataSpec.addStreamData(streamName);
    request.addDataOption(dataSpec);

    QTcpSocket* client = _createClient();
    _sendRequest(client, request);
    CPPUNIT_ASSERT_EQUAL(1, _server->clientsForStream(streamName));

    TestDataBlob blob;
    blob.setData("sometestData");
    _server->send(streamName, &blob);

    CPPUNIT_ASSERT( client->state() == QAbstractSocket::ConnectedState );
    boost::shared_ptr<ServerResponse> r = _clientProtocol->receive(*client);
    CPPUNIT_ASSERT( r->type() == ServerResponse::Blob );
    DataBlobResponse* res = static_cast<DataBlobResponse*>(r.get());
    CPPUNIT_ASSERT( res->dataName() == streamName );
    CPPUNIT_ASSERT( res->blobClass() == "TestDataBlob" );
    TestDataBlob recvBlob;
    recvBlob.deserialise(*client, res->byteOrder());

    CPPUNIT_ASSERT(recvBlob == blob);
}

void TCPConnectionManagerTest::test_brokenConnection()
{
    QTcpSocket* client = 0;
    QString streamName = "testData";

    // Use Case:
    //   Client requests a connection.
    // Expect:
    //   Client to be registered for any data.
    {
        DataSpec dataSpec;
        dataSpec.addStreamData(streamName);
        StreamDataRequest request;
        request.addDataOption(dataSpec);
        client = _createClient();
        _sendRequest(client, request);
        CPPUNIT_ASSERT(client->state() == QAbstractSocket::ConnectedState);
        QCoreApplication::processEvents();
        CPPUNIT_ASSERT_EQUAL(1, _server->clientsForStream(streamName));
    }

    // Use Case:
    //   Client dies after connection.
    // Expect:
    //   To be removed from the system.
    {
        delete client;
        // Need to process events to connect the SocketError signal to the
        // TCPConnectionManager::ConnectionError() SLOT
        usleep(200);
        QCoreApplication::processEvents();
        CPPUNIT_ASSERT_EQUAL(0, _server->clientsForStream(streamName));
    }
}


void TCPConnectionManagerTest::test_dataSupportedRequest()
{
    QTcpSocket* client = _createClient();

    boost::shared_ptr<ServerResponse> r;
    QString stream1("stream1");
    TestDataBlob blob;
    blob.setData("stream1Data");

    // Use Case:
    //   Send (write to the client socket) a dataSupport request with no data.
    // Expect:
    //   Stream (when reading the socket) to return a DataSupport response
    //   and add the client to the DataSupport stream.
    {
        QString streamInfo("__streamInfo__");
        DataSupportRequest req;
        CPPUNIT_ASSERT_EQUAL(0, _server->clientsForStream(streamInfo));
        // Write the data support request to the client (socket)
        _sendRequest(client, req);
        CPPUNIT_ASSERT_EQUAL(1, _server->clientsForStream(streamInfo));
        CPPUNIT_ASSERT(client->state() == QAbstractSocket::ConnectedState);
        // Read the data support request from the client.
        r = _clientProtocol->receive(*client);
        CPPUNIT_ASSERT(r->type() == ServerResponse::DataSupport);
    }


    // Use case:
    //   New stream type arrives. (server sends a data blob of stream
    //   type stream1
    // Expect:
    //   To receive a new DataSupport response with the the new data.
    {
        // Sends data to connected clients.
        _server->send(stream1, &blob);
        // Receive new data (added to the socket by _server->send() )
        r = _clientProtocol->receive(*client);
        CPPUNIT_ASSERT(r->type() == ServerResponse::DataSupport);
        DataSupportResponse* res = static_cast<DataSupportResponse*>(r.get());
        CPPUNIT_ASSERT_EQUAL(1, res->streamData().size());
        CPPUNIT_ASSERT(res->streamData().contains(stream1));
    }


    // Use case:
    //   Existing stream type arrives.
    // Expect:
    //   Not to receive a new DataRequest. In the case _server->send()
    //   in fact sends no data so clientProtocol->receive() has nothing to
    //   receive and times out.
    {
        // Sends data to connected clients - same data as in previous use case.
        _server->send(stream1, &blob);
        CPPUNIT_ASSERT(client->state() == QAbstractSocket::ConnectedState);
        // Receive data.
        r = _clientProtocol->receive(*client);
        CPPUNIT_ASSERT(client->state() == QAbstractSocket::ConnectedState);
        CPPUNIT_ASSERT(r->type() == ServerResponse::Error);
        CPPUNIT_ASSERT(r->type() != ServerResponse::DataSupport);
    }
}

QTcpSocket* TCPConnectionManagerTest::_createClient() const
{
    // Create a client and connect it to the server
    QTcpSocket* tcpSocket = new QTcpSocket;
    tcpSocket->connectToHost(QHostAddress::LocalHost, _server->serverPort());
    if (!tcpSocket->waitForConnected(5000)
            || tcpSocket->state() == QAbstractSocket::UnconnectedState)
    {
        delete tcpSocket;
        tcpSocket = 0;
        CPPUNIT_FAIL("Client could not connect to server");
    }
    return tcpSocket;
}

void TCPConnectionManagerTest::_sendRequest(QTcpSocket* tcpSocket, const ServerRequest& req)
{
    QByteArray data = _clientProtocol->serialise(req);
    tcpSocket->write(data);
    while (tcpSocket->bytesToWrite() > 0)
        tcpSocket->waitForBytesWritten(-1);
    tcpSocket->flush();
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
}

} // namespace pelican
