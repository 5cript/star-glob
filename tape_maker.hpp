#pragma once

#include <star-tape/star_tape.hpp>
#include <boost/filesystem.hpp>

#include <type_traits>
#include <fstream>

#include <iostream>

namespace StarGlob
{
#if defined(ENABLE_BZIP2) && ENABLE_BZIP2 == 1
    constexpr auto BestAvailableCompression = StarTape::CompressionType::Bzip2;
#elif defined(ENABLE_GZIP) && ENABLE_GZIP == 1
    constexpr auto BestAvailableCompression = StarTape::CompressionType::Gzip;
#else
    constexpr auto BestAvailableCompression = StarTape::CompressionType::None;
#endif // ENABLE_GZIP

    template <auto Compression = BestAvailableCompression>
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
#if defined(ENABLE_BZIP2) && ENABLE_BZIP2 == 1
                    if constexpr(Compression == StarTape::CompressionType::Bzip2)
                    {
                        if (fileName_.extension() != ".bz2")
                            fileName_ += ".bz2";
                    }
#endif // ENABLE_BZIP2
#if defined(ENABLE_GZIP) && ENABLE_GZIP == 1
                    if constexpr(Compression == StarTape::CompressionType::Gzip)
                    {
                        if (fileName_.extension() != ".gz")
                            fileName_ += ".gz";
                    }
#endif // ENABLE_GZIP

                    return StarTape::createOutputFileArchive <Compression> (fileName_.string());
                }()
            }
            , operations_{}
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
        void addFiles(
            IteratorT const& begin,
            IteratorT const& end,
            std::string const& fileRoot,
            std::string const& prefix = {},
            bool dereferenceLinks = false
        )
        {
            namespace fs = boost::filesystem;
            using namespace StarTape::TapeOperations;
            for (auto i = begin; i != end; ++i)
            {
                if (!dereferenceLinks && fs::is_symlink(fs::symlink_status(*i)))
                {
                    if (prefix.empty())
                        operations_ << AddLink(fs::read_symlink(*i).string(), (fs::path{fileRoot} / *i).string());
                    else
                        operations_ << AddLink(fs::read_symlink(*i).string(), (fs::path{prefix} / *i).lexically_normal().string());
                }
                else
                {
                    if (prefix.empty())
                        operations_ << AddFile((fs::path{fileRoot} / *i).string());
                    else
                        operations_ << AddFile((fs::path{fileRoot} / *i).string(), (fs::path{prefix} / *i).lexically_normal().string());
                }
            }
        }

        /**
         *  Writes changes.
         */
        void apply()
        {
            operations_.apply(&archive(bundle_), nullptr, false);
        }

        /**
         *  Writes changes.
         */
        void apply(std::function <void(int, int)> const& cb)
        {
            operations_.setProgressCallback(cb);
            operations_.apply(&archive(bundle_), nullptr, false);
        }

    private:
        boost::filesystem::path fileName_;
        StarTape::OutputArchiveDataBundle <
            typename StarTape::CompressionTypeToWriter <compression_type>::type,
            std::ofstream
        > bundle_;
        StarTape::TapeWaterfall operations_;
    };
}
