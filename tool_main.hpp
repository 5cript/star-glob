#pragma once

#include "hash_map.hpp"
#include "config.hpp"
#include <boost/filesystem.hpp>

std::vector <boost::filesystem::path> collectFiles(StarGlob::Glob const& glob, StarGlob::HashMap& hashes);
