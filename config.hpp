#pragma once

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#   include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>
#endif

#include <vector>
#include <string>
#include <iostream>

#include <boost/optional.hpp>

namespace StarGlob
{
    struct Config : public JSON::Stringifiable <Config>
                  , public JSON::Parsable <Config>
    {
        std::string fileRoot;
        std::vector <std::string> globExpressions;
        boost::optional <std::vector <std::string>> directoryFilter;
        boost::optional <std::vector <std::string>> fileFilter;
    };

    Config loadConfig(std::istream& json);
    void saveConfig(std::ostream& stream, Config const& cfg);
}

BOOST_FUSION_ADAPT_STRUCT
(
    StarGlob::Config,
    fileRoot, globExpressions, directoryFilter, fileFilter
)
