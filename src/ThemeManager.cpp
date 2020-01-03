#include "ThemeManager.hpp"
#include <sapi/var/Json.hpp>

ThemeManager::ThemeManager() : m_theme(m_theme_file)
{

}

var::Vector<var::String> ThemeManager::get_styles() const {
   var::Vector<var::String> result;
   //these must be in the correct order
   result.push_back("dark");
   result.push_back("light");
   result.push_back("brandPrimary");
   result.push_back("brandSecondary");
   result.push_back("info");
   result.push_back("success");
   result.push_back("warning");
   result.push_back("danger");
   return result;
}

var::String ThemeManager::get_style_name(enum Theme::style value){
   switch(value){
      case Theme::style_dark: return "dark";
      case Theme::style_light: return "light";
      case Theme::style_brand_primary: return "brandPrimary";
      case Theme::style_brand_secondary: return "brandSecondary";
      case Theme::style_info: return "info";
      case Theme::style_success: return "success";
      case Theme::style_warning: return "warning";
      case Theme::style_danger: return "danger";
   }
   return "";
}

var::String ThemeManager::get_state_name(enum Theme::state value){
   switch(value){
      case Theme::state_default: return "default";
      case Theme::state_disabled: return "disabled";
      case Theme::state_highlighted: return "highlighted";
   }
   return "";
}

enum sgfx::Theme::style ThemeManager::get_theme_style(const var::String & style_name){
   if( style_name == "dark" ){ return sgfx::Theme::style_dark; }
   if( style_name == "light" ){ return sgfx::Theme::style_light; }
   if( style_name == "brandPrimary" ){ return sgfx::Theme::style_brand_primary; }
   if( style_name == "brandSecondary" ){ return sgfx::Theme::style_brand_secondary; }
   if( style_name == "info" ){ return sgfx::Theme::style_info; }
   if( style_name == "success" ){ return sgfx::Theme::style_success; }
   if( style_name == "warning" ){ return sgfx::Theme::style_warning; }
   if( style_name == "danger" ){ return sgfx::Theme::style_danger; }

   //error
   printer().error("unrecognized style name " + style_name);
   return sgfx::Theme::style_light;
}

ThemeManager::theme_color_t ThemeManager::import_hex_code(const var::String & hex) const {
   theme_color_t result = {0};

   if( hex.length() != 7 ){
      printer().error("invalid hex code format " + hex + " use `#aaddcc`");
      return result;
   }
   result.red = hex.create_sub_string(String::Position(1), String::Length(2)).to_unsigned_long(String::base_16);
   result.green = hex.create_sub_string(String::Position(3), String::Length(2)).to_unsigned_long(String::base_16);
   result.blue = hex.create_sub_string(String::Position(5), String::Length(2)).to_unsigned_long(String::base_16);

   return result;
}

var::Vector<var::String> ThemeManager::get_states() const {
   var::Vector<var::String> result;

   result.push_back("default");
   result.push_back("highlighted");
   result.push_back("disabled");
   return result;
}



int ThemeManager::import(
      File::SourcePath input,
      File::DestinationPath output,
      u8 bits_per_pixel
      ){
   var::JsonObject configuration = var::JsonDocument().load(
            fs::File::Path(input.argument())
            ).to_object();

   if( configuration.is_valid() == false ){
      printer().error("failed to load path " + input.argument());
      return -1;
   }

   String output_path;

   if( output.argument().is_empty() ){
      output_path = FileInfo::no_suffix(input.argument()) + ".sth";
   } else {
      output_path = output.argument();
   }

   if( m_theme.create(
          output_path,
          fs::File::IsOverwrite(true),
          Theme::BitsPerPixel(bits_per_pixel),
          Theme::PixelFormat(0)
          ) < 0 ){
      printer().error("failed to save theme to " + output_path);
      return -1;
   }

   for(const auto & style: get_styles() ){
      var::JsonObject style_object =
            configuration.at(style).to_object();

      printer().message("processing " + style);
      if( style_object.is_valid() == false ){
         printer().error("input missing style " + style);
         return -1;
      }
      var::Array<theme_color_t, 4> colors;
      colors.at(0) = import_hex_code(
               style_object.at("background").to_string()
               );
      colors.at(1) = import_hex_code(
               style_object.at("border").to_string()
               );
      colors.at(2) = import_hex_code(
               style_object.at("color").to_string()
               );
      colors.at(3) = import_hex_code(
               style_object.at("text").to_string()
               );

      set_color(
               get_theme_style(style),
               Theme::state_default,
               colors
               );

      colors.at(0) =
            calculate_highlighted(import_hex_code(
                                     style_object.at("background").to_string()
                                     ));
      colors.at(1) =
            calculate_highlighted(import_hex_code(
                                     style_object.at("border").to_string()
                                     ));
      colors.at(2) =
            calculate_highlighted(import_hex_code(
                                     style_object.at("color").to_string()
                                     ));
      colors.at(3) =
            calculate_highlighted(import_hex_code(
                                     style_object.at("text").to_string()
                                     ));

      set_color(
               get_theme_style(style),
               Theme::state_highlighted,
               colors
               );

      colors.at(0) =
            calculate_disabled(import_hex_code(
                                  style_object.at("background").to_string()
                                  ));
      colors.at(1) =
            calculate_disabled(import_hex_code(
                                  style_object.at("border").to_string()
                                  ));
      colors.at(2) =
            calculate_disabled(import_hex_code(
                                  style_object.at("color").to_string()
                                  ));
      colors.at(3) =
            calculate_disabled(import_hex_code(
                                  style_object.at("text").to_string()
                                  ));

      set_color(
               get_theme_style(style),
               Theme::state_disabled,
               colors
               );

   }

   m_theme_file.close();

   printer().info("saved theme to " + output_path);

   return 0;
}

void ThemeManager::set_color(
      enum Theme::style style,
      enum Theme::state state,
      const var::Array<theme_color_t, 4> base_colors
      ){

   var::Vector<u16> colors( m_theme.color_count() );
   printer().debug(String().format(
                      "theme palette has %d colors",
                      colors.count()
                      ));
   for(size_t first = 0; first < base_colors.count(); first++){
      for(size_t second = 0; second < base_colors.count(); second++){
         size_t offset = first*4 + second;
         u16 rgb = mix(
                  base_colors.at(first),
                  base_colors.at(second)
                  );

         printer().debug(String().format(
                            "setting %s, %s to color %d:0x%04X",
                            get_style_name(style).cstring(),
                            get_state_name(state).cstring(),
                            offset,
                            rgb
                            ));

         colors.at(offset) = rgb;
      }
   }
   m_theme_file.write(colors);

}

u16 ThemeManager::mix(const theme_color_t & first, const theme_color_t & second){
   //first has 66% weigth, second has 33% weight
   u32 red = (first.red * 666 + second.red * 333 + 500)/1000;
   u32 green = (first.green * 666 + second.green * 333 + 500)/1000;
   u32 blue = (first.blue * 666 + second.blue * 333 + 500)/1000;
   u16 rgb = (red & 0xf8) << 8;
   rgb |= (green & 0xfc) << 3;
   rgb |= (blue & 0xf8) >> 3;
   printer().debug(
            String().format("mix %02X %02X %02X + %02X %02X %02X = %02X %02X %02X = 0x%04X",
                            first.red, first.green, first.blue,
                            second.red, second.green, second.blue,
                            red, green, blue,
                            rgb
                   ));
   return rgb;
}

ThemeManager::theme_color_t ThemeManager::calculate_highlighted(const theme_color_t & color){
   return color;
}

ThemeManager::theme_color_t ThemeManager::calculate_disabled(const theme_color_t & color){
   return color;
}
