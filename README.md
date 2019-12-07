# fonttool

Font tool is a desktop tool used to convert vector fonts and icons to bitmap or vectors fonts and icons that can be used with Stratify OS.

# Input Files

Input files can be 

- the output of the GlyphDesigner (bmp and txt files)
- json files describing svg data

TTF fonts can be converted to svg using an online converter. The resulting svg's can be converted to json using the svgson node program.

# Viewing Fonts and Icons

Use `--action=show` to view fonts and icons.

```
fonttool --action=show --input=assets/OpenSansCondensed-Light-15.sbf
fonttool --action=show --input=assets/icons.svic
```

# Creating fonts and Icons

## Icons

Convert the svg files in the specified folder to a Stratify OS vector icon format:

```
fonttool --action=convert --icon --input=icons --output=assets/icons.svic
```

Or convert a single icon:

```
fonttool --action=convert --icon --input=icons/adjust-solid.svg --output=assets/adjust-solid.svic
```

## Fonts

Font input files must be in svg format. TTF files can be converted to SVG using online font file converters such as https://convertio.co/ttf-svg/.

Once you have a svg font file use:

```
fonttool --action=convert --input=fonts/OpenSansCondensed-Light.svg --output=assets
```

### Font Maps

When generating a font, you can generate a map file that is human editable to fine tune the font.

```
fonttool --action=convert --input=fonts/OpenSansCondensed-Light.svg --output=assets --map
```

Once the editing is done, you can generate a font from the map file.

```
fonttool --action=convert --input=fonts/OpenSansCondensed-Light-map.txt --output=assets
```
