#ifndef RESULT_DIR_H_
#define RESULT_DIR_H_

#include <cassert>
#include <boost/filesystem.hpp>

struct Dir
{
	boost::filesystem::path path;
	
	Dir(int argc, char **argv)
	{
		assert(argc>1);
		path = argv[1];
		boost::filesystem::remove_all(path);
		assert(boost::filesystem::create_directories(path));
	}

	/**
	 * @brief appends file path to the directory path
	 */
	std::string operator()(std::string filepath)
	{
		boost::filesystem::path p(path);
		return (p /= filepath).string();
	}
};

#endif  // RESULT_DIR_H_
