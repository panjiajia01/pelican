#include "PelicanClientProtocol.h"
#include "Data.h"
#include "StreamData.h"
#include "ServerRequest.h"
#include "ServerResponse.h"
#include "ServiceDataResponse.h"
#include "StreamDataResponse.h"
#include "ServiceDataRequest.h"
#include "StreamDataRequest.h"
#include "data/DataRequirements.h"
#include <QByteArray>
#include <QAbstractSocket>
#include <QDataStream>
#include <iostream>

#include "utility/memCheck.h"

namespace pelican {


// class PelicanClientProtocol 
PelicanClientProtocol::PelicanClientProtocol()
    : AbstractClientProtocol()
{
}

PelicanClientProtocol::~PelicanClientProtocol()
{
}

QByteArray PelicanClientProtocol::serialise(const ServerRequest& req)
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_4_0);
    ds << (quint16)req.type();
    switch(req.type()) {
        case ServerRequest::Acknowledge:
            break;
        case ServerRequest::DataSupport:
            break;
        case ServerRequest::StreamData:
            {
                const StreamDataRequest& r = static_cast<const StreamDataRequest& >(req);
                ds << (quint16)r.size();
                DataRequirementsIterator it=r.begin();
                while( it != r.end() )
                {
                    _serializeDataRequirements(ds, *it);
                    ++it;
                }
            }
            break;
        case ServerRequest::ServiceData:
            {
                const ServiceDataRequest& r = static_cast<const ServiceDataRequest& >(req);
                ds << (quint16)r.types().size();
                foreach( QString type, r.types() ) {
                    ds << type << r.version(type);
                }
            }
            break;
        default:
            break;
    }
    return data;
}

boost::shared_ptr<ServerResponse> PelicanClientProtocol::receive(QAbstractSocket& socket)
{
//     std::cout << "PelicanClientProtocol::receive()" << std::endl;
    int timeout = 2000;
    ServerResponse::Response type = ServerResponse::Error;
    while (socket.bytesAvailable() < (int)sizeof(quint16)) {
        if ( !socket.waitForReadyRead(timeout) ) {
            std::cout << "PelicanClientProtocol::receive error!!!!!!!!!!" << std::endl;
            return boost::shared_ptr<ServerResponse>(new ServerResponse(type,  socket.errorString() ));
        }
    }

    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_0);
    in >> (quint16&)type;

//     std::cout << "PelicanClientProtocol::receive(): Type: " << type << std::endl;


    switch(type) {
        case ServerResponse::Acknowledge:
            break;
        case ServerResponse::StreamData:
            {
                boost::shared_ptr<StreamDataResponse> s(new StreamDataResponse);
                quint16 streams;
                in >> streams;
                for(int i=0; i < streams; ++i ) {

                    QString name;
                    in >> name;
                    QString id;
                    in >> id;
                    quint64 size;
                    in >> size;

                    StreamData* sd = new StreamData(name, 0, (unsigned long)size);
                    s->setStreamData(sd);
                    sd->setId(id);

                    // read in associate meta-data
                    quint16 associates;
                    in >> associates;
                    for( unsigned int j=0; j < associates; ++j ) {
                        in >> name >> id >> size;
                        sd->addAssociatedData( boost::shared_ptr<Data>(new Data(name, id, size)) );
                    }
                }
                return s;
            }
            break;
        case ServerResponse::ServiceData:
            {
                boost::shared_ptr<ServiceDataResponse> s(new ServiceDataResponse);
                quint16 sets;
                in >> sets;
                QString name;
                QString version;
                quint64 size;
                for(int i=0; i < sets; ++i ) {
                    in >> name;
                    in >> version;
                    in >> size;
                    s->addData(new Data(name,version,size));
                }
                return s;
            }
            break;
        default:
            break; 
    }
    return boost::shared_ptr<ServerResponse>(new ServerResponse(ServerResponse::Error, "PelicanClientProtocol: Unknown type passed"));
}

void PelicanClientProtocol::_serializeDataRequirements(QDataStream& stream, const DataRequirements& req) const
{
    QSet<QString> serviceData = req.serviceData();
    QSet<QString> streamData = req.streamData();
    stream << serviceData << streamData;

}

} // namespace pelican