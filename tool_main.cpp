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
    std::string compressionString;
    po::options_description desc{"Allowed options"};
    desc.add_options()
        ("help", "produce help message")
        ("config,f", po::value<std::string>(&configPath)->default_value("./config.json"), "config to use")
        ("output,o", po::value<std::string>(&output), "file name to write to")
        ("init,i", "initialize with default config")
        ("make-hashes,h", "make __meta dir with hashes of each file")
        (
                "compression,c",
                po::value <std::string>(&compressionString),
                "Compression to use, not all are available on every system. Can be gz,bz2,none. "
                "Defaults to strongest compression method."
        )
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
        StarGlob::Glob glob;
        StarGlob::Config exampleConfig;
        glob.fileRoot = ".";
        glob.globExpressions.push_back("*.*");
        exampleConfig.globbers.push_back(glob);
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

    std::unique_ptr <TapeMakerBase> tapeMaker;
    auto compression = BestAvailableCompression;
    if (vm.count("compression"))
    {
        if (compressionString == "bz2")
        {
#if defined(ENABLE_BZIP2)
            compression = StarTape::CompressionType::Bzip2;
            tapeMaker.reset(new TapeMaker <StarTape::CompressionType::Bzip2>(output));
#else
            std::cerr << "Bzip2 not supported with this build\n";
            return 1;
#endif
        }
        else if (compressionString == "gz")
        {
#if defined(ENABLE_GZIP)
            compression = StarTape::CompressionType::Gzip;
            tapeMaker.reset(new TapeMaker <StarTape::CompressionType::Gzip>(output));
#else
            std::cerr << "Gzip not supported with this build\n";
            return 1;
#endif
        }
        else if (compressionString == "none")
        {
            compression = StarTape::CompressionType::None;
            tapeMaker.reset(new TapeMaker <StarTape::CompressionType::None>(output));
        }
        else
        {
            std::cerr << "Unknown compression parameter: " << compressionString << "\n";
            std::cerr << "It must be one of: gz, bz2, none";
            return 1;
        }
    }
    else {
        tapeMaker.reset(new TapeMaker(output));
    }

    HashMap hashes;
    for (auto const& glob : config.globbers)
    {
        std::string prefix;
        if (glob.pathPrefix)
            prefix = glob.pathPrefix.get();
        auto files = collectFiles(glob, hashes, prefix);

        //TapeMaker<StarTape::CompressionType::Bzip2> tapeMaker{output};
        addFilesToTapeMaker(tapeMaker.get(), compression, std::begin(files), std::end(files), glob.fileRoot, prefix, false);
    }
    if (vm.count("make-hashes"))
    {
        std::ofstream hashWriter{"hashes.json", std::ios_base::binary};
        hashWriter << hashes;
        tapeMaker->addFile("hashes.json", ".", "__meta/");
    }
    tapeMaker->apply([](int progress, int max)
    {
        if (max != 0)
            std::cout << static_cast <int> (100. * progress / static_cast <double> (max)) << "%\n";
    });

    return 0;
}
//---------------------------------------------------------------------------------------------------------------------
std::vector <boost::filesystem::path> collectFiles(StarGlob::Glob const& glob, StarGlob::HashMap& hashes, std::string const& prefix)
{
    using namespace StarGlob;

    Globber globber{glob.fileRoot};

    StarGlob::HashMap freshHashMap;
    freshHashMap.prefix = prefix;
    globber.setHashMap(freshHashMap);

    // glob filtering
    if (glob.directoryFilter)
        globber.setDirectoryBlackList(glob.directoryFilter.get());
    if (glob.fileFilter)
        globber.setBlackList(glob.fileFilter.get());

    // collect files
    std::vector <boost::filesystem::path> fileContainer;
    for (auto const& mask : glob.globExpressions)
        globber.globRecursive(mask, fileContainer, false);

    hashes.append(*globber.hashMap());

    return fileContainer;
}
//#####################################################################################################################
