#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <ctime>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
using namespace boost;

const size_t MIN_BLOCK_SIZE = 25;

std::string str_dir_path;
boost::mutex output_mutex;

long long parse_file(const std::string& fname)
{
	long long	  value;
	std::string   in_value;
	std::ifstream in_file(str_dir_path+fname);
	in_file >> in_value;
	
	for (std::string::const_iterator pos = in_value.cbegin(); pos != in_value.cend(); ++pos)
		if (!isdigit(*pos))
			return 0;
	value = std::stoll(in_value);

	output_mutex.lock();
	std::cout << fname << ": " << value << std::endl;
	output_mutex.unlock();

	in_file.close();
	return value;
}


void parallel_block_sum(std::vector<std::string>::const_iterator &begin,
						std::vector<std::string>::const_iterator &end,
						long long &curr_sum) 
{
	std::vector<std::string>::const_iterator iter_str;
	for (iter_str = begin; iter_str != end; ++iter_str)
		curr_sum += parse_file(*iter_str);
	boost::this_thread::sleep(boost::posix_time::seconds(1));
}


long long parallel_sum(std::vector<std::string>::const_iterator &begin,
					   std::vector<std::string>::const_iterator &end)
{
	if (begin > end) return 0;

	const size_t distance			= std::distance(begin, end);
	const size_t max_num_threads	= (distance+MIN_BLOCK_SIZE-1) / MIN_BLOCK_SIZE;
	const size_t hdwr_threads		= boost::thread::hardware_concurrency();
	const size_t num_threads		= std::min(max_num_threads, hdwr_threads != 0 ? hdwr_threads:2);

	size_t					block_size = distance / num_threads;
	std::vector<thread>		threads(num_threads-1);
	std::vector<long long>	result(num_threads);

	std::vector<std::string>::const_iterator iter_block_begin = begin;
	std::vector<std::string>::const_iterator iter_block_end;
	for (size_t i = 0; i < (num_threads-1); ++i ) 
	{
		iter_block_end = iter_block_begin;
		std::advance(iter_block_end, block_size);
		threads[i] = boost::thread( parallel_block_sum, 
									iter_block_begin,
									iter_block_end,
									boost::ref(result[i]));
		iter_block_begin = iter_block_end;
	}
	parallel_block_sum(iter_block_begin, end, result[num_threads-1]);
	for_each(threads.begin(), threads.end(), std::mem_fn(&boost::thread::join));

	return std::accumulate(result.begin(), result.end(), 0);
}


std::vector<std::string> parse_dir(const std::string& str_dir_path)
{
	std::vector<std::string>		file_list;
	filesystem::path				path_dir(str_dir_path);
	filesystem::directory_iterator	iter_dir(path_dir), iter_end;

	for ( ; iter_dir != iter_end; ++iter_dir ) 
		if (filesystem::is_regular_file(*iter_dir))
			file_list.push_back(iter_dir->path().filename().string());
	return file_list;
}


int main()
{
	std::cin >> str_dir_path;

	std::vector<std::string> file_list = parse_dir(str_dir_path);
	long long sum = parallel_sum(file_list.cbegin(), file_list.cend());
	std::cout << "sum: " << sum << std::endl;

	std::system("pause");
	return 0;
}