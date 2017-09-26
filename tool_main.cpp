#include "tool_main.hpp"
#include "filesystem/globber.hpp"
#include "tape_maker.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <iostream>

//#####################################################################################################################
int main(int argc, char** argv)
{
    //######################################################################################################
    // PROGRAM OPTIONS
    //######################################################################################################

    std::string configPath;
    std::string output;
    po::options_description desc{"Allowed options"};
    desc.add_options()
        ("help", "produce help message")
        ("config,f", po::value<std::string>(&configPath)->default_value("./config.json"), "config to use")
        ("output,o", po::value<std::string>(&output), "file name to write to")
        ("init,i", "initialize with default config")
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch (std::exception const& exc)
    {
        std::cerr << "error during command line parsing: " << exc.what() << "\n";
        return 1;
    }
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 0;
    }
    if (vm.count("init"))
    {
        std::ofstream writer{configPath, std::ios_base::binary};
        if (!writer.good())
        {
            std::cerr << "cannot create config.\n";
            return 1;
        }
        StarGlob::Config exampleConfig;
        exampleConfig.fileRoot = ".";
        exampleConfig.globExpressions.push_back("*.*");
        StarGlob::saveConfig(writer, exampleConfig);
        return 0;
    }
    if (!vm.count("output"))
    {
        std::cerr << "Please specify target file (-o)\n";
        return 1;
    }

    //######################################################################################################
    // PROJECT
    //######################################################################################################

    using namespace StarGlob;

    // load Config
    std::ifstream configFile{configPath, std::ios_base::binary};
    if (!configFile.good())
    {
        std::cerr << "config file not found, specified by (-f)\n";
        return 1;
    }
    auto&& config = loadConfig(configFile);

    auto files = collectFiles(config);

    //TapeMaker<StarTape::CompressionType::Bzip2> tapeMaker{output};
    TapeMaker tapeMaker{output};
    tapeMaker.makeTape(std::begin(files), std::end(files));

    return 0;
}
//---------------------------------------------------------------------------------------------------------------------
std::vector <boost::filesystem::path> collectFiles(StarGlob::Config const& config)
{
    using namespace StarGlob;

    Globber globber{config.fileRoot};

    // glob filtering
    if (config.directoryFilter)
        globber.setDirectoryBlackList(config.directoryFilter.get());
    if (config.fileFilter)
        globber.setBlackList(config.fileFilter.get());

    // collect files
    std::vector <boost::filesystem::path> fileContainer;
    for (auto const& mask : config.globExpressions)
        globber.globRecursive(mask, fileContainer);

    return fileContainer;
}
//#####################################################################################################################
