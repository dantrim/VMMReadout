#ifndef MULTILAYER_H
#define MULTILAYER_H

//boost
#include <boost/property_tree/ptree.hpp>

//nsw
#include "coordinates.h"
#include "layer.h"


class MultiLayer {
    public :
        MultiLayer();
        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }
        bool loadMultiLayer(const boost::property_tree::ptree::value_type pt);

        std::string name() { return m_name; }
        std::string id() { return m_id; }

        int nLayer() { return n_layer; }
        Layer getLayer(int i) { return m_layerArray.at(i); }

    private :
        std::string m_map_dir;
        std::string m_name;
        std::string m_id;

        int n_layer;

        Coordinates m_position;
        Coordinates m_rotation;

        std::vector<Layer> m_layerArray;
};

#endif
