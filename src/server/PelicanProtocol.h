#ifndef PELICANPROTOCOL_H
#define PELICANPROTOCOL_H


#include "AbstractProtocol.h"

/**
 * @file PelicanProtocol.h
 */

namespace pelican {

/**
 * @class PelicanProtocol
 *  
 * @brief
 *   The primary protocol for communication between pipelines and
 *   the server
 * @details
 * 
 */

class PelicanProtocol : public AbstractProtocol
{
    public:
        PelicanProtocol(  );
        ~PelicanProtocol();
        virtual ServerRequest request(QTcpSocket& socket);
        virtual void send(QDataStream& stream, const AbstractProtocol::StreamData_t& );
        virtual void send(QDataStream& stream, const AbstractProtocol::ServiceData_t& );
        virtual void send(QDataStream& stream, const QString& );
        virtual void sendError(QDataStream& stream, const QString&);

    private:
};

} // namespace pelican
#endif // PELICANPROTOCOL_H 
