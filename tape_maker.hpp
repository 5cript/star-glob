#pragma once

#define ENABLE_BZIP2 1
#define ENABLE_GZIP 1

#include <star-tape/star_tape.hpp>
#include <boost/filesystem/path.hpp>

#include <type_traits>

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
        }

        /**
         *  dtor
         */
        ~TapeMaker() = default;

        /**
         *  Creates a tape from a list of files.
         */
        template <typename IteratorT>
        void makeTape(IteratorT const& begin, IteratorT const& end)
        {
            auto bundle = StarTape::createOutputFileArchive <Compression> (fileName_.string());

            using namespace StarTape::TapeOperations;
            auto&& waterfall = StarTape::TapeWaterfall{};
            for (auto i = begin; i != end; ++i)
            {
                if constexpr(std::is_same <typename IteratorT::value_type, boost::filesystem::path>::value)
                    waterfall << AddFile(i->string());
                else
                    waterfall << AddFile(*i);
            }
            waterfall.apply(&archive(bundle));
        }

    private:
        boost::filesystem::path fileName_;
    };
}
