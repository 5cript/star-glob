#pragma once

#define ENABLE_BZIP2 1
#define ENABLE_GZIP 1

#include <star-tape/star_tape.hpp>
#include <boost/filesystem/path.hpp>

#include <type_traits>
#include <fstream>

namespace StarGlob
{
    template <auto Compression = StarTape::CompressionType::None>
    class TapeMaker
    {
    public:
        static constexpr auto compression_type = Compression;

    public:
        /**
         *  @param fileName The file path to save the file to.
         */
        TapeMaker(boost::filesystem::path fileName)
            : fileName_{std::move(fileName)}
            , bundle_{
                [this]()
                {
                    if constexpr(Compression == StarTape::CompressionType::Bzip2)
                    {
                        if (fileName_.extension() != ".bz2")
                            fileName_ += ".bz2";
                    }
                    else if constexpr(Compression == StarTape::CompressionType::Gzip)
                    {
                        if (fileName_.extension() != ".gz")
                            fileName_ += ".gz";
                    }

                    return StarTape::createOutputFileArchive <Compression> (fileName_.string());
                }()
            }
        {
        }

        /**
         *  dtor
         */
        ~TapeMaker() = default;

        /**
         *  Creates a tape from a list of files.
         */
        template <typename IteratorT>
        void addFiles(IteratorT const& begin, IteratorT const& end, std::string const& fileRoot, std::string const& prefix = {})
        {
            namespace fs = boost::filesystem;
            using namespace StarTape::TapeOperations;
            auto&& waterfall = StarTape::TapeWaterfall{};
            for (auto i = begin; i != end; ++i)
            {
                if (prefix.empty())
                    waterfall << AddFile((fs::path{fileRoot} / *i).string());
                else
                    waterfall << AddFile((fs::path{fileRoot} / *i).string(), (fs::path{prefix} / *i).lexically_normal().string());
            }
            waterfall.apply(&archive(bundle_));
        }

    private:
        boost::filesystem::path fileName_;
        StarTape::OutputArchiveDataBundle <
            typename StarTape::CompressionTypeToWriter <compression_type>::type,
            std::ofstream
        > bundle_;
    };
}
