#pragma once

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#   include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>
#endif

#include <iostream>
#include <string>
#include <unordered_map>

namespace StarGlob
{
    using ReadableHash = std::string;

    struct HashMap : public JSON::Stringifiable <HashMap>
                   , public JSON::Parsable <HashMap>
    {
        std::unordered_map <std::string, ReadableHash> fileHashMappings;
        std::string prefix;

        /**
         *  Calculate hash of file and add it to the map.
         */
        void addFile(std::string const& fileName, std::string const& alias = {});

        /**
         *  Add another map to this one. Does overwrite already existing elements.
         */
        void append(HashMap const& map);

        /**
         *  @return The first invalid hash comparison.
         */
        std::unordered_map <std::string, ReadableHash>::iterator verifyAgainst(std::string const& root);

        void toStream(std::ostream& stream) const;
        void fromStream(std::istream& stream);
    };

    inline std::ostream& operator<<(std::ostream& stream, HashMap const& map)
    {
        map.toStream(stream);
        return stream;
    }
    inline std::istream& operator>>(std::istream& stream, HashMap& map)
    {
        map.fromStream(stream);
        return stream;
    }
}

BOOST_FUSION_ADAPT_STRUCT
(
    StarGlob::HashMap,
    fileHashMappings
)
