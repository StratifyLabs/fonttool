#ifndef THEMEMANAGER_HPP
#define THEMEMANAGER_HPP

#include <sapi/fs.hpp>
#include <sapi/var.hpp>
#include <sapi/sgfx.hpp>

#include "ApplicationPrinter.hpp"

class ThemeManager : public ApplicationPrinter {
public:
   ThemeManager();

   /*! \details Imports a theme from a JSON file.
    *
    * ```json
    * {
    *   "light": {
    *     "background": "#ffffff",
    *     "border": "#cccccc",
    *     "color": "#ffffff",
    *     "text": "#222222",
    *   },
    *   "dark": {
    *     "background": "#ffffff",
    *     "border": "#000000",
    *     "color": "#222222",
    *     "text": "#ffffff",
    *   },
    * }
    * ```
    *
    * Also add, primary, secondary, info, success, warning, danger
    *
    *
    */
   int import(
         fs::File::SourcePath input,
         fs::File::DestinationPath output,
         u8 bits_per_pixel
         );

private:

   //holds the RGB values for all styles and states
   sgfx::Theme m_theme;
   File m_theme_file;

   typedef struct {
     u8 red;
     u8 green;
     u8 blue;
   } theme_color_t;

   u16 mix(const theme_color_t & first, const theme_color_t & second);
   theme_color_t calculate_highlighted(const theme_color_t & color);
   theme_color_t calculate_disabled(const theme_color_t & color);

   void set_color(enum Theme::style style,
         enum Theme::state state,
         const var::Array<theme_color_t, 4> base_colors
         );

   enum sgfx::Theme::style get_theme_style(const var::String & style_name);
   var::String get_style_name(enum Theme::style value);
   var::String get_state_name(enum Theme::state value);
   var::Vector<var::String> get_styles() const;
   var::Vector<var::String> get_states() const;
   theme_color_t import_hex_code(const String & hex) const;
};

#endif // THEMEMANAGER_HPP
