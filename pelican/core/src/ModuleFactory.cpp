#include <QStringList>
#include <QtGlobal>
#include "core/ModuleFactory.h"
#include "utility/Config.h"

#include "modules/AbstractModule.h"
#include "modules/VisibilityPrinter.h"
#include "modules/BasicFlagger.h"
#include "modules/EmptyModule.h"
#include "modules/ZenithCalibrater.h"
#include "modules/ZenithImagerDft.h"
#include "modules/ZenithModelVisibilities.h"
#include "modules/ImageWriterFits.h"
#include "modules/ImageCombiner.h"

#include "utility/memCheck.h"

namespace pelican {

/**
 * @details
 * ModuleFactory constructor.
 */
ModuleFactory::ModuleFactory(const Config *config)
{
    _config = config;
    _configRoot.append(Config::NodeId("pipeline", ""));
    _configRoot.append(Config::NodeId("modules", ""));
}

/**
 * @details
 * ModuleFactory destructor.
 */
ModuleFactory::~ModuleFactory()
{
    foreach( AbstractModule* module, _modules ) {
        delete module;
    }
}

/**
 * @details
 * Public interface to create a new module.
 *
 * @param[in] type     A string containing the type of the module.
 * @param[in] name     A string containing the name of the module.
 *
 * @return
 * Returns a pointer to the newly-created module.
 * The object is still owned by the factory.
 */
AbstractModule* ModuleFactory::createModule(const QString& type, const QString& name)
{
    /* Create an index into the configuration file and get the configuration */
    Config::TreeAddress address = _configRoot;
    address.append(QPair<QString, QString>(type, name));
    ConfigNode config = _config->get(address);

    /* Create the module */
    return _createModule(type, config);
}

/**
 * @details
 * Creates a new module.
 * Called by the public function createModule.
 *
 * @param[in] name   A string containing the name of the module.
 * @param[in] config Configuration of the module.
 *
 * @return
 * Returns a pointer to the newly-created module.
 * The object is still owned by the factory.
 */
AbstractModule* ModuleFactory::_createModule(const QString& name, const ConfigNode& config)
{
    AbstractModule* module = 0;
    if (name == "BasicFlagger" ) {
        module = new BasicFlagger(config);
        _modules.append( module );
    }
    else if (name == "EmptyModule" ) {
        module = new EmptyModule(config);
        _modules.append( module );
    }
    else if (name == "ZenithCalibrater" ) {
        module = new ZenithCalibrater(config);
        _modules.append( module );
    }
    else if (name == "ZenithImagerDft" ) {
        module = new ZenithImagerDft(config);
        _modules.append( module );
    }
    else if (name == "ZenithModelVisibilities" ) {
        module = new ZenithModelVisibilities(config);
        _modules.append( module );
    }
    else if (name == "VisibilityPrinter") {
        module = new VisibilityPrinter(config);
        _modules.append(module);
    }
    else if (name == "ImageWriterFits") {
        module = new ImageWriterFits(config);
        _modules.append(module);
    }
    else if (name == "ImageCombiner") {
        module = new ImageCombiner(config);
        _modules.append(module);
    }
    else {
        throw QString("Unknown module type '" + name + "'");
    }
    return module;
}

} // namespace pelican