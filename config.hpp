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
    struct Glob : public JSON::Stringifiable <Glob>
                  , public JSON::Parsable <Glob>
    {
        /// source file root
        std::string fileRoot;

        /// path prefix to prepend into tar.
        boost::optional <std::string> pathPrefix;

        /// Wildcard globber expression
        std::vector <std::string> globExpressions;

        /// directory blacklist (no wildcards)
        boost::optional <std::vector <std::string>> directoryFilter;

        /// file blacklist to exclude from globber. (say *.cpp is globbed and main.cpp excluded).
        boost::optional <std::vector <std::string>> fileFilter;
    };

    struct Config : public JSON::Stringifiable <Config>
                  , public JSON::Parsable <Config>
    {
        std::vector <Glob> globbers;
    };

    Config loadConfig(std::istream& json);
    void saveConfig(std::ostream& stream, Config const& cfg);
}

BOOST_FUSION_ADAPT_STRUCT
(
    StarGlob::Glob,
    fileRoot, globExpressions, directoryFilter, fileFilter, pathPrefix
)

BOOST_FUSION_ADAPT_STRUCT
(
    StarGlob::Config,
    globbers
)
