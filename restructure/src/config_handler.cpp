// vmm
#include "config_handler.h"
//#include "string_utils.h"

// std/stl
#include <exception>

//////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------ //
//  ConfigHandler -- Constructor
// ------------------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////////////////
ConfigHandler::ConfigHandler(QObject *parent)
    : QObject(parent)
{


}
//// ------------------------------------------------------------------------ //
void ConfigHandler::LoadConfig(const QString &filename)
{
    using boost::property_tree::ptree;
    using namespace boost::property_tree::xml_parser;
    ptree pt;
    read_xml(filename.toStdString(), pt, trim_whitespace | no_comments);

    try {
        for(const auto& conf : pt.get_child("configuration")) {
            std::cout << conf.first << std::endl;

        } // configuration
    }
    catch(std::exception &e)
    {
        std::cout << "!! --------------------------------- !!" << std::endl;
        std::cout << "ERROR : " << e.what() << std::endl;
        std::cout << "!! --------------------------------- !!" << std::endl;
        exit(1);
    }

}

