#include "globber.hpp"

#include <functional>

namespace StarGlob
{
//#####################################################################################################################
    template <typename IteratorT>
    void globImpl(Globber* globber, IteratorT& i, std::vector <boost::filesystem::path>& result)
    {
    }
//#####################################################################################################################
    Globber::Globber(std::string root, bool directories)
        : root_{std::move(root)}
        , blackList_{}
        , dirBlackList_{}
        , directories_{directories}
        , hashes_{}
    {

    }
//---------------------------------------------------------------------------------------------------------------------
    HashMap* Globber::hashMap()
    {
        return &hashes_;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Globber::setHashMap(HashMap const& map)
    {
        hashes_ = map;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Globber::setBlackList(std::vector <std::string> const& blackList)
    {
        blackList_ = blackList;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Globber::setDirectoryBlackList(std::vector <std::string> const& blackList)
    {
        dirBlackList_ = blackList;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Globber::isBlacklisted(boost::filesystem::path const& p)
    {
        for (auto const& i : blackList_)
            if (checkMask(p, i))
                return true;
        for (auto const& i : dirBlackList_)
        {
            auto root = p;
            while (!root.parent_path().empty())
                root = root.parent_path();
            if (root.string() == i)
                return true;
        }
        return false;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <boost::filesystem::path> Globber::glob(std::string const& mask, bool prependRoot)
    {
        std::vector <boost::filesystem::path> result;
        glob(mask, result, prependRoot);
        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    std::vector <boost::filesystem::path> Globber::globRecursive(std::string const& mask, bool prependRoot)
    {
        std::vector <boost::filesystem::path> result;
        globRecursive(mask, result, prependRoot);
        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    void Globber::glob(std::string const& mask, std::vector <boost::filesystem::path>& files, bool prependRoot)
    {
        using namespace boost::filesystem;

        for (directory_iterator i{root_}, end; i != end; ++i)
            globImpl(i, files, mask, prependRoot);
    }
//---------------------------------------------------------------------------------------------------------------------
    void Globber::globRecursive(std::string const& mask, std::vector <boost::filesystem::path>& files, bool prependRoot)
    {
        using namespace boost::filesystem;

        for (recursive_directory_iterator i{root_}, end; i != end; ++i)
            globImpl(i, files, mask, prependRoot);
    }
//---------------------------------------------------------------------------------------------------------------------
    bool Globber::checkMask(boost::filesystem::path const& p, std::string const& mask)
    {
        std::function <bool(const char*, const char*)> match;

        match = [&match](char const *needle, char const *haystack) -> bool
        {
            for (; *needle != '\0'; ++needle)
            {
                switch (*needle)
                {
                    case '?':
                        if (*haystack == '\0')
                            return false;
                        ++haystack;
                        break;
                    case '*':
                    {
                        if (needle[1] == '\0')
                            return true;
                        size_t max = strlen(haystack);
                        for (size_t i = 0; i < max; i++)
                            if (match(needle + 1, haystack + i))
                                return true;
                        return false;
                    }
                    default:
                        if (*haystack != *needle)
                            return false;
                        ++haystack;
                }
            }
            return *haystack == '\0';
        };

        return match(mask.c_str(), p.string().c_str());
    }
//#####################################################################################################################
}
