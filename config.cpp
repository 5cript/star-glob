#include "config.hpp"

#include <SimpleJSON/stringify/jss_vector.hpp>
#include <SimpleJSON/stringify/jss_fundamental.hpp>
#include <SimpleJSON/stringify/jss_string.hpp>
#include <SimpleJSON/stringify/jss_optional.hpp>
#include <SimpleJSON/stringify/jss_convenience.hpp>

#include <SimpleJSON/parse/jsd_fundamental.hpp>
#include <SimpleJSON/parse/jsd_container.hpp>
#include <SimpleJSON/parse/jsd_string.hpp>
#include <SimpleJSON/parse/jsd_optional.hpp>

namespace StarGlob
{
//#####################################################################################################################
    Config loadConfig(std::istream& json)
    {
        Config cc;
        auto tree = JSON::parse_json(json);
        JSON::parse(cc, "starGlob", tree);
        return cc;
    }
//---------------------------------------------------------------------------------------------------------------------
    void saveConfig(std::ostream& stream, Config const& cfg)
    {
        stream << "{";
        JSON::try_stringify(stream, "starGlob", cfg, JSON::ProduceNamedOutput);
        stream << "}";
    }
//#####################################################################################################################
}
