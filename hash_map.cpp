#include "hash_map.hpp"

#include <SimpleJSON/stringify/jss_fundamental.hpp>
#include <SimpleJSON/stringify/jss_string.hpp>
#include <SimpleJSON/stringify/jss_optional.hpp>
#include <SimpleJSON/stringify/jss_unordered_map.hpp>
#include <SimpleJSON/stringify/jss_convenience.hpp>

#include <SimpleJSON/parse/jsd_fundamental.hpp>
#include <SimpleJSON/parse/jsd_container.hpp>
#include <SimpleJSON/parse/jsd_string.hpp>
#include <SimpleJSON/parse/jsd_unordered_map.hpp>
#include <SimpleJSON/parse/jsd_optional.hpp>

#include <boost/filesystem.hpp>
#include <sstream>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

namespace StarGlob
{
    namespace fs = boost::filesystem;
//#####################################################################################################################
    std::string makeHash(std::string const& fileName)
    {
        using namespace CryptoPP;

        SHA256 hash;
        std::string hashString;
        FileSource(
            fileName.c_str(),
            true,
            new HashFilter(
                hash,
                new HexEncoder(
                    new StringSink(hashString),
                    true
                )
            )
        );

        return hashString;
    }
//#####################################################################################################################
    void HashMap::addFile(std::string const& fileName, std::string const& alias)
    {
        auto hashString = makeHash(fileName);

        if (alias.empty())
            fileHashMappings[fileName] = hashString;
        else
            fileHashMappings[alias] = hashString;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::unordered_map <std::string, ReadableHash>::iterator HashMap::verifyAgainst(std::string const& root)
    {
        for (auto i = std::begin(fileHashMappings); i != std::end(fileHashMappings); ++i)
        {
            auto path = fs::path{root} / i->first;

            if (!fs::exists(path))
                return i;

            if (i->second != makeHash(path.string()))
                return i;
        }

        return std::end(fileHashMappings);
    }
//---------------------------------------------------------------------------------------------------------------------
    void HashMap::toStream(std::ostream& stream) const
    {
        stream << "{";
        JSON::try_stringify(stream, "hashes", *this, JSON::ProduceNamedOutput);
        stream << "}";
    }
//---------------------------------------------------------------------------------------------------------------------
    void HashMap::fromStream(std::istream& stream)
    {
        auto tree = JSON::parse_json(stream);
        JSON::parse(*this, "hashes", tree);
    }
//#####################################################################################################################
}
