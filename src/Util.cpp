/*! \file */ //Copyright 2011-2018 Tyler Gilbert; All Rights Reserved

#include <cmath>
#include <sapi/fmt.hpp>
#include <sapi/fs.hpp>
#include "Util.hpp"

void Util::show_icon_file(
		File::SourcePath input_file,
		File::DestinationPath output_file,
		sg_size_t canvas_size,
		u16 downsample_size
		){

	File icon_file;
	Printer p;
	bool is_write_bmp = false;
	Bitmap bmp_output;

	if( !output_file.argument().is_empty() ){
		is_write_bmp = true;
	}

	if( icon_file.open(
			 input_file.argument(),
			 OpenFlags::read_only()
			 ) < 0 ){
		printf("failed to open vector icon file\n");
		return;
	}

	Svic icon_collection(input_file.argument());
	Bitmap canvas(Area(canvas_size, canvas_size));

	Area downsampled_area(
				downsample_size, downsample_size
				);

	Area canvas_downsampled_area;
	canvas_downsampled_area.set_width( (canvas.width() + downsample_size/2) / downsample_size );
	canvas_downsampled_area.set_height( (canvas.height() + downsample_size/2) / downsample_size );
	Bitmap canvas_downsampled(canvas_downsampled_area);
	VectorMap map(canvas);

	p.message("%d icons in collection", icon_collection.count());

	u32 count = (sqrtf(icon_collection.count()) + 0.5f);

	if( is_write_bmp ){
		bmp_output.allocate(
					Area(
						canvas_downsampled_area.width() * count,
						canvas_downsampled_area.height() * count
						)
					);
		bmp_output.clear();
	}

	for(u32 i=0; i < icon_collection.count(); i++){

		p.key("name", icon_collection.name_at(i));
		VectorPath vector_path = icon_collection.at(i);
		vector_path << canvas.get_viewable_region();
		canvas.clear();
		sgfx::Vector::draw(
					canvas,
					vector_path,
					map
					);

		if( downsample_size > 1 ){
			canvas_downsampled.downsample_bitmap(
						canvas,
						downsampled_area
						);
			p.open_object("icon") << canvas_downsampled << p.close();
		} else {
			p.open_object("icon") << canvas << p.close();
		}

		if( is_write_bmp ){
			bmp_output.draw_sub_bitmap(
						Point(
							(i % count) * canvas_downsampled_area.width(),
							i / count * canvas_downsampled_area.height()
						),
						canvas_downsampled,
						Region(Point(0,0), canvas_downsampled_area)
						);
		}

	}

	if( is_write_bmp ){

		Palette bmp_pallete;

		bmp_pallete
				.set_bits_per_pixel(1)
				.fill_gradient_gray();


		fmt::Bmp::save(
					output_file.argument(),
					bmp_output,
					bmp_pallete
					);
	}

}


void Util::clean_path(const String & path, const String & suffix){
	Dir d;
	const char * entry;
	String str;

	if( d.open(path) < 0 ){
		printf("Failed to open path: '%s'\n", path.cstring());
	}

	while( (entry = d.read()) != 0 ){
		str = File::suffix(entry);
		if( str == "sbf" ){
			str.clear();
			str << path << "/" << entry;
			printf("Removing file: %s\n", str.cstring());
			File::remove(str);
		}
	}
}

void Util::show_file_font(const String & path, bool is_details){

	FileFont ff;
	Ap::printer().info("Show font %s", path.cstring());

	if( ff.set_file(path) < 0 ){
		printf("Failed to open font %s", path.cstring());
		perror("Open failed");
		return;
	}

	show_font(ff);

	if( is_details ){
		File f;

		if( f.open(
				 path,
				 OpenFlags::read_only()
				 ) < 0 ){
			Ap::printer().error("Failed to open file '%s'", path.cstring());
			return;
		}

		sg_font_header_t header;
		if( f.read(header) != sizeof(sg_font_header_t) ){
			Ap::printer().error("failed to read header");
			return;
		}

		printer().open_object("header");
		{
			printer().key("version", "0x%04X", header.version);
			printer().key("size", "%d", header.size);
			printer().key("characterCount", "%d", header.character_count);
			printer().key("kerningPairCount", "%d", header.kerning_pair_count);
			printer().key("maxWordWidth", "%d", header.max_word_width);
			printer().key("maxHeight", "%d", header.max_height);
			printer().key("bitsPerPixel", "%d", header.bits_per_pixel);
			printer().key("canvasWidth", "%d", header.canvas_width);
			printer().key("canvasHeight", "%d", header.canvas_height);
			printer().close_object();
		}


		Vector<sg_font_kerning_pair_t> kerning_pairs;
		for(u32 i=0; i < header.kerning_pair_count; i++){
			sg_font_kerning_pair_t pair;
			if( f.read(pair) != sizeof(sg_font_kerning_pair_t) ){
				Ap::printer().error("Failed to read kerning pair");
				return;
			}
			kerning_pairs.push_back(pair);
		}

		if( header.kerning_pair_count > 0 ){
			Ap::printer().open_array("kerning pairs");
			for(u32 i=0; i < kerning_pairs.count(); i++){
				Ap::printer().key("kerning", "%d %d %d",
										kerning_pairs.at(i).unicode_first,
										kerning_pairs.at(i).unicode_second,
										kerning_pairs.at(i).horizontal_kerning);

			}
			Ap::printer().close_array();
		} else {
			Ap::printer().info("no kerning pairs present");
		}

		Vector<sg_font_char_t> characters;
		for(u32 i=0; i < header.character_count; i++){
			sg_font_char_t character;
			printer().debug("read character from %d", f.seek(0, File::CURRENT));
			if( f.read(character) != sizeof(sg_font_char_t) ){
				Ap::printer().error(
							"Failed to read character at %d (%d, %d)",
							i,
							f.return_value(),
							f.error_number()
							);
				continue;
			}
			printer().debug("push character %d", character.id);
			characters.push_back(character);
		}

		Ap::printer().open_object("characters");
		for(u32 i=0; i < characters.count(); i++){
			Ap::printer().key(String().format("%d", characters.at(i).id), "canvas %d->%d,%d %dx%d advancex->%d offset->%d,%d",
									characters.at(i).canvas_idx,
									characters.at(i).canvas_x,
									characters.at(i).canvas_y,
									characters.at(i).width,
									characters.at(i).height,
									characters.at(i).canvas_x,
									characters.at(i).advance_x,
									characters.at(i).offset_x,
									characters.at(i).offset_y);

		}
		Ap::printer().close_array();
	}
}

void Util::show_font(Font & f){
	Bitmap b;
	u32 i;


	Ap::printer().info("Alloc bitmap %d x %d with %d bpp",
							 f.get_width(),
							 f.get_height(),
							 f.bits_per_pixel());
	b.set_bits_per_pixel(f.bits_per_pixel());
	b.allocate(Area(f.get_width()*8/4, f.get_height()*5/4));

	for(i=0; i < Font::ascii_character_set().length(); i++){
		b.clear();
		String string;
		string << Font::ascii_character_set().at(i);

		if( i < Font::ascii_character_set().length()-1 ){
			string << Font::ascii_character_set().at(i+1);
			Ap::printer().info("Character: %c", Font::ascii_character_set().at(i+1));
		} else {
			Ap::printer().info("Character: %c", Font::ascii_character_set().at(i));
		}
		f.draw(string, b, Point(2, 0));
		Ap::printer().info("\twidth:%d height:%d xadvance:%d offsetx:%d offsety:%d",
								 f.character().width, f.character().height,
								 f.character().advance_x,
								 f.character().offset_x,
								 f.character().offset_y);

		Ap::printer() << b;
	}
}

void Util::show_system_font(int idx){

#if defined __link
	printf("System fonts not available\n");

#else
	printf("Not implemented\n");
#endif
}
