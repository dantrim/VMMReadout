#ifndef DAQCONFIG_H
#define DAQCONFIG_H

#include <vector>
#include <string>

//nsw
#include "febconfig.h"
#include "detectorconfig.h"

class DaqConfiguration {

    public :
        DaqConfiguration();

        std::string febConfigFile() { return m_febConfigFile; }
        std::string detectorConfigFile() { return m_detectorConfigFile; }

        FEBConfig& febConfig() { return *m_febConfig; }
        DetectorConfig& detConfig() { return *m_detConfig; }

        bool loadDaqXml(std::string filename);
        bool readDaqXml(std::string fullfilename);
        bool loadFEB();
        bool loadDetectorSetup();

    private :
        std::string m_map_dir;
        std::string m_febConfigFile;
        std::string m_detectorConfigFile;

        FEBConfig* m_febConfig;
        DetectorConfig* m_detConfig;

}; // class


#endif
