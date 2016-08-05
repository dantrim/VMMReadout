#ifndef DETECTORCONFIG_H
#define DETECTORCONFIG_H

#include <vector>
#include <string>

//nsw
#include "coordinates.h"
#include "chamber.h"

class DetectorConfig {

    public :
        DetectorConfig();

        void setMapDir(std::string mapdir) { m_map_dir = mapdir; }

        // load the detector configuration based on the input
        // xml file
        bool loadDetectorSetup(std::string filename);

        // get the number of chambers in this detector setup
        int nChamber() { return n_chambers; }

        Chamber getChamber(int i) { return m_chamberArray.at(i); }

        Coordinates position() { return m_position; }
        Coordinates rotation() { return m_rotation; }

    private :
        std::string m_map_dir;
        std::string m_name;

        int n_chambers;

        std::vector<Chamber> m_chamberArray;

        // coorinates
        Coordinates m_position;
        Coordinates m_rotation;

};

#endif
