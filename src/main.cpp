/*! \file */ //Copyright 2011-2018 Tyler Gilbert; All Rights Reserved

#include <sapi/fmt.hpp>
#include <sapi/hal.hpp>
#include <sapi/sys.hpp>
#include <sapi/var.hpp>
#include <sapi/sgfx.hpp>

#include "Util.hpp"
#include "BmpFontManager.hpp"
#include "SvgFontManager.hpp"
#include "ApplicationPrinter.hpp"

void show_usage(const Cli & cli);

int main(int argc, char * argv[]){
	String bits_per_pixel;
	String action;


	Cli cli(argc, argv);
	cli.set_publisher("Stratify Labs, Inc");
	cli.handle_version();

	Ap::printer().set_verbose_level( cli.get_option("verbose") );

	action = cli.get_option(
				"action",
				Cli::Description("specify the action to perform --action=show|convert|clean")
				);

	bool is_details = cli.get_option(
				"details",
				Cli::Description("show details")
				) == "true";


	String input = cli.get_option(
				"input",
				Cli::Description("specify the path to the input --input=<path> (file suffix should be sbf|svg|bmp|txt)")
				);

	String output = cli.get_option(
				"output",
				Cli::Description("specify the path to the output folder --output=<path>")
				);

	String canvas_size = cli.get_option(
				"canvas",
				Cli::Description("specify the canvas size to use in pixels --canvas=128")
				);

	if( canvas_size.to_integer() == 0 ){
		Ap::printer().message("using 128 pixel canvas");
		canvas_size = "128";
	}

	String pour_size = cli.get_option(
				"pour",
				Cli::Description("specify the size of the grid pour search --pour=4")
				);

	if( pour_size.to_integer() == 0 ){
		Ap::printer().message("using 4 pixel grid pour size");
		pour_size = "3";
	}

	String downsample_size = cli.get_option(
				"downsample",
				Cli::Description("specify the downsampling factor --downsampling=4")
				);

	if( downsample_size.to_integer() == 0 ){
		Ap::printer().message("using 4 pixel grid pour size");
		downsample_size = "4";
	}

	bool is_icon = cli.get_option(
				"icon",
				Cli::Description("input should be an svg or a folder with svg icons --icon=true")
				) == "true";

	bool is_overwrite = cli.get_option(
				"overwrite",
				Cli::Description("overwrite existing files --overwrite=true")
				) == "true";

	bool is_map = cli.get_option(
				"map",
				Cli::Description("generate an map file as well as the font files (or specify a path to a map to convert)")
				) == "true";

	bool is_json = cli.get_option(
				"json",
				Cli::Description("generate a json file of the SVG input")
				) == "true";


	String characters = cli.get_option(
				"characters",
				Cli::Description("specify the characters to process (default is ascii)")
				);

	if( characters.is_empty() ){
		characters = Font::ascii_character_set();
		characters.erase(String::Position(0), String::Length(1));
	}

	bits_per_pixel = cli.get_option(
				"bpp",
				Cli::Description("specify the number of bits to use for each pixel --bpp=<1|2|4|8>")
				);

	if( bits_per_pixel.is_empty() ){
		bits_per_pixel = "1";
	} else if( bits_per_pixel == "true" ){
		Ap::printer().error("use --bpp=<1|2|4|8>");
		exit(0);
	}

	switch(bits_per_pixel.to_integer()){
		case 1:
		case 2:
		case 4:
		case 8:
			break;
		default:
			Ap::printer().error("use --bpp=<1|2|4|8>");
			exit(0);
	}

	Ap::printer().open_object("options");
	{
		Ap::printer().key("action", action);
		Ap::printer().key("details", is_details ? "true" : "false");
		Ap::printer().key("input", input);
		Ap::printer().key("output", output.is_empty() ? "<auto>" : output.cstring() );
		Ap::printer().key("canvas", canvas_size);
		Ap::printer().key("downsample", downsample_size);
		Ap::printer().key("pour", pour_size);
		Ap::printer().key("overwrite", is_overwrite ? "true" : "false");
		Ap::printer().key("characters", characters.is_empty() ? "<ascii>" : characters.cstring() );
		Ap::printer().key("bitsPerPixel", bits_per_pixel);
		Ap::printer().close_object();
	}

	String input_suffix = FileInfo::suffix(input);

	if( action == "show" ){

		Ap::printer().message("action=show");



		if( input_suffix == "svic" ){
			Ap::printer().message(
						"Show icon file %s with canvas size %d",
						input.capacity(),
						canvas_size.to_integer()
						);

			if( output.is_empty() == false ){
				if( FileInfo::suffix(output) != "bmp" ){
					Ap::printer().error(
								"output must be a .bmp file"
								);
				}
			}

			Util::show_icon_file(
						File::SourcePath(input),
						File::DestinationPath(output),
						canvas_size.to_integer(),
						downsample_size.to_integer(),
						bits_per_pixel.to_integer()
						);
		} else {
			Ap::printer().message(
						"Show font file %s with details %d",
						input.cstring(),
						is_details
						);
			Util::show_file_font(
						File::SourcePath(input),
						File::DestinationPath(output),
						Util::IsDetails(is_details)
						);
		}
		exit(0);
	}

	if( action == "clean" ){
		Ap::printer().message("Cleaning directory %s from sbf files", input.cstring());
		Util::clean_path(input, "sbf");
		exit(0);
	}

	if( action == "convert" ){

		//input is a bmp, svg or map file
		if( input.is_empty() || input == "true" ){
			Ap::printer().error("input file or directory must be specified with --input=<path>");
			return 1;
		}

		if( input_suffix == "svg" || is_icon ){

			SvgFontManager svg_font;

			svg_font.set_output_json(is_json);
			svg_font.set_pour_grid_size( pour_size.to_integer() );
			svg_font.set_canvas_size( canvas_size.to_integer() );
			svg_font.set_generate_map(is_map);
			svg_font.set_downsample_factor(
						Area(
							downsample_size.to_integer(),
							downsample_size.to_integer()
							)
						);

			if( is_icon ){
				svg_font.set_flip_y(false);
				Ap::printer().message(
							"convert folder %s/*.svg to icons",
							input.cstring()
							);

				svg_font.process_icons(
							File::SourcePath(input),
							File::DestinationPath(output)
							);
			} else {
				//svg file with a converted TTF
				Ap::printer().message("convert font file");
				svg_font.set_character_set(characters);
				svg_font.set_flip_y(true);
				svg_font.process_font(
							File::SourcePath(input),
							File::DestinationPath(output)
							);
			}
		} else if( input_suffix == "json" ){
			//map file input
			Ap::printer().message("generating sbf font from map file");
			BmpFontGenerator bmp_font_generator;
			if( bmp_font_generator.import_map(input) == 0 ){

				if( File::get_info(output).is_directory() ){
					output << "/" << FileInfo::base_name(input);
				}

				if( bmp_font_generator.generate_font_file(output) < 0 ){
					return 1;
				}
			}
		} else if( input_suffix == "bmp" ){

			Ap::printer().message(
						"generating sbf font from bmp/definition file"
						);

			BmpFontManager bmp_font_manager;
			bmp_font_manager.set_bits_per_pixel(bits_per_pixel.to_integer());
			if( is_map ){
				bmp_font_manager.set_generate_map(true);
			}
			//pass the base name which will use both bmp (for image) and txt for kernel defs
			bmp_font_manager.convert_font(
						FileInfo::no_suffix(input)
						);
			exit(0);
		} else if( input_suffix == "sbf" ){
			//generate a map file from the sbf file
			Ap::printer().message(
						"generate map file from sbf font"
						);

			BmpFontManager bmp_font_manager;
			bmp_font_manager.set_bits_per_pixel(bits_per_pixel.to_integer());
			bmp_font_manager.generate_map(input);
			exit(0);
		}
	}

	if( cli.get_option("help") == "true" ){
		show_usage(cli);
		exit(0);
	}

	return 0;
}

void show_usage(const Cli & cli){
	cli.show_options();
}

