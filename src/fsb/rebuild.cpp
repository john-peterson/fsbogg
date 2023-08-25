#include "fsb/fsb.hpp"
#include "fsb/container.hpp"
#include "fsb/io/buffer_view.hpp"
#include "fsb/io/utility.hpp"
#include "fsb/vorbis/rebuilder.hpp"

#include <boost/filesystem.hpp>
#include <glog/logging.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace {

struct rebuild_options {
	std::filesystem::path ogg;
};

void usage(const char *name) {
	std::cout <<
	          "Usage: " << name << " ogg\n";
}

rebuild_options parse_options(int argc, char **argv) {
	rebuild_options options;
	options.ogg = std::filesystem::current_path();

	if (argc<2) {
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	options.ogg=argv[1];

	return options;
}

void print_sample(std::ostream & os, const fsb::sample & sample) {
	os
	        << "Name:          " << sample.name << '\n'
	        << "Frequency:     " << sample.frequency << '\n'
	        << "Channels:      " << int(sample.channels) << '\n'
	        << "Offset:        " << sample.offset << '\n'
	        << "Size:          " << sample.size << '\n'
	        << "Vorbis CRC-32: " << std::hex <<  sample.vorbis_crc32 << '\n'
	        << "Loop start:    " << sample.loop_start << '\n'
	        << "Loop end:      " << sample.loop_end << '\n'
	        << "Unknown:       " << sample.unknown << '\n';
}

} // namespace

int main(int argc, char **argv) {
	google::InitGoogleLogging(argv[0]);

	const rebuild_options options = parse_options(argc, argv);
	const auto ipath = options.ogg;

	fsb::sample  sample;
	sample.name="ogg";
	sample.frequency=44100;
	sample.channels=1;
	sample.size=std::filesystem::file_size(ipath);
	sample.offset=0;
	sample.loop_start=0;
	sample.loop_end=0;
	sample.unknown=0;
	sample.vorbis_crc32=0x2d2931;
	print_sample(std::cout, sample);
	std::cout << std::endl;

	std::ifstream istream(ipath.native(),  std::ios_base::in | std::ios_base::binary);
	CHECK(istream) << "Failed to open path: " << ipath.native();

//	fsb::container container(istream, "");
	//auto & header = container.file_header();
	// std::cout << path.native() << std::endl;

	std::vector<char> data_buffer_ = fsb::io::read(istream, sample.size);
	istream.close();
	const char * const sample_begin = data_buffer_.data() + sample.offset;
	const char * const sample_end = sample_begin + sample.size;
	fsb::io::buffer_view sample_view(sample_begin, sample_end);

	const std::filesystem::path opath = "out.ogg";
	std::ofstream ostream(opath.native());
	CHECK(ostream) << "Failed to open output file: " << opath;

	fsb::vorbis::rebuilder rebuilder;
	rebuilder.rebuild(sample, sample_view, ostream);

	return 0;
}
