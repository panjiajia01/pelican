#include "DataTypes.h"
#include "adapters/AbstractStreamAdapter.h"
#include "adapters/AbstractServiceAdapter.h"
#include "data/DataRequirements.h"


#include "utility/memCheck.h"

namespace pelican {


/**
 *@details DataTypes 
 */
DataTypes::DataTypes()
{
}

/**
 *@details
 */
DataTypes::~DataTypes()
{
}

/// @details
//  adds a data requirement. All data must be added before
//  any adapters are set.
void DataTypes::addData(const DataRequirements& data)
{
    _dataRequirements.append(data);
}

void DataTypes::addData(const QList<DataRequirements>& data)
{
    _dataRequirements += data;
}
/// @details
//  Sets a particular adapter for a particular type of data. All data added 
//  with addData will be made consistent with the adpater
void DataTypes::setAdapter(const QString& type, AbstractAdapter* adapter)
{
    _adapters[type] = adapter;
    // ensure each datarequirement is of the correct type
    for(int i=0; i < _dataRequirements.size(); ++i )
    {
        DataRequirements& req = _dataRequirements[i];
        switch( adapter->type() )
        {
            case AbstractAdapter::Service :
                req.setServiceData( type );
                break;
            case AbstractAdapter::Stream :
                req.setStreamData( type );
                break;
            default:
                throw QString("unknown adapter type");
                break;
        }
    }
}

AbstractStreamAdapter* DataTypes::streamAdapter(const QString& type) const
{
    Q_ASSERT( _adapters.contains(type) );
    Q_ASSERT( _adapters.value(type)->type() == AbstractAdapter::Stream );
    return static_cast<AbstractStreamAdapter*>(_adapters.value(type));
}

AbstractServiceAdapter* DataTypes::serviceAdapter(const QString& type) const
{
    Q_ASSERT( _adapters.contains(type) );
    Q_ASSERT( _adapters.value(type)->type() == AbstractAdapter::Service );
    return static_cast<AbstractServiceAdapter*>(_adapters.value(type));
}

const QList<DataRequirements>& DataTypes::dataRequirements() const
{
    return _dataRequirements;
}

} // namespace pelican