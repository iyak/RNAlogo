# RNAlogo

A biological sequence logo generator in SVG format, for the usage as a header-only library for cpp programs.
The alphabets are given as any kinds of strings coded in Unicode, such as "Ψ" (greek) or "AU" (two characters).

## Download & Run sample code

```bash
git clone htpps://github.com/iyak/RNAlogo.git
cd RNAlogo
./waf configure build
build/bin/RNAlogo-sample > sample.svg
```

The generated sample logo looks like,

![sample1](https://cdn.rawgit.com/iyak/RNAlogo/master/logos/sample1.svg)

## Usage

Include the header file. Then,

```c++
#include "logo.hpp"
using namespace iyak;

int main() {
  RNAlogo logo;
  
  /* font settings */
  //logo.set_font("/Library/Fonts//AppleGothic.ttf"); // this step is done during configuration. see wscript.
  
  /* geometory settings */
  logo.set_title("sample");
  // logo.set_x_axis_height(0); // if you don't want position label.

  /* color map settings */
  logo.map_color("A", "green"); // HTML-like color name ok.
  logo.map_color("G", "gold");
  logo.map_color("C", "royalblue");
  logo.map_color("U", "crimson");
  logo.map_color("Ψ", "#9932CC"); // Unicode ok.
  logo.map_color("Ψo", "#9932CC"); // multiple characters ok.

  /* run */
  logo.pict_table_bit
  (
   {{1,22,4,93},{1,24,29, 3},{12,1,19,39},{5,1,51,2},{23,1,66,3},{56,2,4,6}}, // stack count
   {{"A","C","G","Ψ"},{"A","C","G","U"},{"A","C","G","Ψo"},{"A","C","G","U"}, // each alphabet
    {"A","C","G","U"},{"A","C","G","U"}},
    {} // meta sequence. you can put meta sequence right under the logo. see sample2.
   );

  return 0;
}
```
