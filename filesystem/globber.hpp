#pragma once

#include "../hash_map.hpp"

#include <boost/filesystem.hpp>

#include <string>

namespace StarGlob
{
    class Globber
    {
    public:
        /**
         *  @param root The root directory to glob files (or dirs) in.
         *  @param directories Collect directories instead of files if true.
         */
        Globber(std::string root, bool directories = false);

        void setBlackList(std::vector <std::string> const& blackList);
        void setDirectoryBlackList(std::vector <std::string> const& blackList);

        std::vector <boost::filesystem::path> glob(std::string const& mask, bool prependRoot);
        std::vector <boost::filesystem::path> globRecursive(std::string const& mask, bool prependRoot);

        void glob(std::string const& mask, std::vector <boost::filesystem::path>& files, bool prependRoot);
        void globRecursive(std::string const& mask, std::vector <boost::filesystem::path>& files, bool prependRoot);

        /**
         *  A handle to the hash map.
         */
        HashMap* hashMap();

        void setHashMap(HashMap const& map);

    private:
        template <typename IteratorT>
        void globImpl(IteratorT& i, std::vector <boost::filesystem::path>& result, std::string const& mask, bool prependRoot)
        {
            if (exists(i->status()))    // does p actually exist?
            {
                bool cont = false;
                if (!directories_ && is_regular_file(i->status()))
                    cont = true;
                else if (directories_ && is_directory(i->status()))
                    cont = true;

                if (cont)
                {
                    auto pathTemp = boost::filesystem::relative(i->path(), root_).string();
                    std::replace(pathTemp.begin(), pathTemp.end(), '\\', '/');
                    auto path = boost::filesystem::path{pathTemp};
                    if (checkMask(path, mask) && !isBlacklisted(path))
                    {
                        if (prependRoot)
                        {
                            hashes_.addFile((root_ / path).string());
                            result.push_back(root_ / path);
                        }
                        else
                        {
                            hashes_.addFile((root_ / path).string(), path.string());
                            result.push_back(path);
                        }
                    }
                }
            }
        }

        /**
         *  Check for a *? wildcard match.
         */
        bool checkMask(boost::filesystem::path const& p, std::string const& mask);

        /**
         *  Returns whether the path is within the black list or not.
         *
         *  @return Return true if path is BLACKLISTED.
         */
        bool isBlacklisted(boost::filesystem::path const& p);

    private:
        boost::filesystem::path root_;
        std::vector <std::string> blackList_;
        std::vector <std::string> dirBlackList_;
        bool directories_;
        HashMap hashes_;
    };
}
