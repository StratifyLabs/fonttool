/*! \file */ //Copyright 2011-2018 Tyler Gilbert; All Rights Reserved


#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <sapi/sys.hpp>
#include <sapi/sgfx.hpp>
#include <sapi/var.hpp>
#include <sapi/fs.hpp>
#include "ApplicationPrinter.hpp"

class Util : public ApplicationPrinter {
public:

	using IsDetails = arg::Argument<bool, struct UtilIsDetailsTag>;

	static void show_icon_file(
			File::SourcePath input_file,
			File::DestinationPath output_file,
			sg_size_t canvas_size,
			u16 downsample_size,
			u8 bits_per_pixel
			);


	static void show_file_font(
			File::SourcePath input_file,
			File::DestinationPath output_file,
			IsDetails is_details = IsDetails(false)
			);
	static void show_font(Font & f);
	static void show_system_font(int idx);
	static void clean_path(const String & path, const String & suffix);


private:
	static void filter(Bitmap & bitmap);


};

#endif /* UTIL_HPP_ */
