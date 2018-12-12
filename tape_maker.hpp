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

    class TapeMakerBase;

    template <auto Compression = BestAvailableCompression>
    class TapeMaker;

    class TapeMakerBase
    {
    public:
        virtual ~TapeMakerBase() = default;

        /**
         *  Writes changes.
         */
        virtual void apply() = 0;

        /**
         *  Writes changes.
         */
        virtual void apply(std::function <void(int, int)> const& cb) = 0;

        virtual void addFile(
            boost::filesystem::path const& path,
            std::string const& fileRoot,
            std::string const& prefix = {},
            bool dereferenceLinks = false
        ) = 0;

        template <typename IteratorT>
        friend void addFilesToTapeMaker(
            TapeMakerBase* tapeMaker,
            StarTape::CompressionType compressionType,
            IteratorT const& begin,
            IteratorT const& end,
            std::string const& fileRoot,
            std::string const& prefix,
            bool dereferenceLinks
        );
    };

    template <auto Compression>
    class TapeMaker : public TapeMakerBase
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
            for (auto i = begin; i != end; ++i)
            {
                addFile(*i, fileRoot, prefix, dereferenceLinks);
            }
        }

        /**
         *  Adds a single file to the tape
         */
        void addFile(
            boost::filesystem::path const& path,
            std::string const& fileRoot,
            std::string const& prefix = {},
            bool dereferenceLinks = false
        ) override
        {
            namespace fs = boost::filesystem;
            using namespace StarTape::TapeOperations;

            auto correctPath = fs::path{fileRoot} / path;
            bool isSymlink = fs::is_symlink(fs::symlink_status(correctPath));
            if (!dereferenceLinks && isSymlink)
            {
                if (prefix.empty())
                    operations_ << AddLink(fs::read_symlink(correctPath).string(), (fs::path{fileRoot} / path).string());
                else
                    operations_ << AddLink(fs::read_symlink(correctPath).string(), (fs::path{prefix} / path).lexically_normal().string());
            }
            else
            {
                if (prefix.empty())
                    operations_ << AddFile(correctPath.string());
                else
                    operations_ << AddFile(correctPath.string(), (fs::path{prefix} / path).lexically_normal().string());
            }
        }

        /**
         *  Writes changes.
         */
        void apply() override
        {
            operations_.apply(&archive(bundle_), nullptr, false);
        }

        /**
         *  Writes changes.
         */
        void apply(std::function <void(int, int)> const& cb) override
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

    template <typename IteratorT>
    void addFilesToTapeMaker(
        TapeMakerBase* tapeMaker,
        StarTape::CompressionType compressionType,
        IteratorT const& begin,
        IteratorT const& end,
        std::string const& fileRoot,
        std::string const& prefix,
        bool dereferenceLinks
    )
    {
        switch (compressionType)
        {
#if defined(ENABLE_BZIP2) && ENABLE_BZIP2 == 1
        case StarTape::CompressionType::Bzip2:
            static_cast <TapeMaker <StarTape::CompressionType::Bzip2>*> (tapeMaker)->
                addFiles <IteratorT>(begin, end, fileRoot, prefix, dereferenceLinks);
            break;
#endif // ENABLE_BZIP2
#if defined(ENABLE_GZIP) && ENABLE_GZIP == 1
        case StarTape::CompressionType::Gzip:
            static_cast <TapeMaker <StarTape::CompressionType::Gzip>*> (tapeMaker)->
                addFiles <IteratorT>(begin, end, fileRoot, prefix, dereferenceLinks);
            break;
#endif // ENABLE_GZIP
        case StarTape::CompressionType::None:
            static_cast <TapeMaker <StarTape::CompressionType::None>*> (tapeMaker)->
                addFiles <IteratorT>(begin, end, fileRoot, prefix, dereferenceLinks);
            break;
        default:
            static_cast <TapeMaker <BestAvailableCompression>*> (tapeMaker)->
                addFiles <IteratorT>(begin, end, fileRoot, prefix, dereferenceLinks);
            break;
        }
    }
}
