#include "multilayer.h"

#include <iostream>

//nsw

//boost
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>


MultiLayer::MultiLayer() :
    m_map_dir(""),
    m_name(""),
    m_id(""),
    n_layer(0)
{
}

bool MultiLayer::loadMultiLayer(const boost::property_tree::ptree::value_type pt)
{
    bool ok = true;
    using boost::property_tree::ptree;
    using namespace boost;

    int layer_count = 0;

    try
    {
        BOOST_FOREACH(const ptree::value_type &v, pt.second) {
            if(v.first == "layer") layer_count++;
        } // foreach
    } // try
    catch(std::exception &e)
    {
        std::cout << "MultiLayer::loadMultiLayer    ERROR: " << e.what() << std::endl;
        ok = false;
    } // catch

    if(ok) {
        n_layer = layer_count;
        try
        {
            BOOST_FOREACH(const ptree::value_type &v, pt.second) {
                //////////////////////////////////
                // multilayer name
                //////////////////////////////////
                if(v.first == "name") {
                    m_name = v.second.data();
                    boost::trim(m_name);
                }
                //////////////////////////////////
                // multilayer id
                //////////////////////////////////
                else if(v.first == "id") {
                    m_id = v.second.data();
                    boost::trim(m_id);
                }
                //////////////////////////////////
                // coordinates
                //////////////////////////////////
                else if(v.first == "position") {
                    m_position.set(v.second.get<double>("x"),
                                   v.second.get<double>("y"),
                                   v.second.get<double>("z"));
                }
                else if(v.first == "rotation") {
                    m_rotation.set(v.second.get<double>("x"),
                                   v.second.get<double>("y"),
                                   v.second.get<double>("z"));
                }
                //////////////////////////////////
                // load a layer
                //////////////////////////////////
                else if(v.first == "layer") {
                    Layer tmpLayer;
                    tmpLayer.setMapDir(m_map_dir);
                    if(!tmpLayer.loadLayer(v)) ok = false;
                    if(ok) m_layerArray.push_back(tmpLayer);
                }

                else {
                    std::cout << "MultiLayer::loadMultiLayer    WARNING Unknown key: " << v.first << std::endl;
                    ok = false;
                }

            } // foreach
        } // try
        catch(std::exception &e)
        {
            std::cout << "MultiLayer::loadMultiLayer    ERROR: " << e.what() << std::endl;
            ok = false;
        }
    } // ok

    return ok;
}
