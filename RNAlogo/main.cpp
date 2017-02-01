//
//  main.cpp
//  RNAlogo
//
//  Created by Hiroshi Miyake on 2017/01/31.
//  Copyright © 2017 Kiryu Lab. All rights reserved.
//

#include<string>
#include<iostream>
#include"logo.hpp"

using namespace iyak;

int main(int argc, char* argv[]) {
  RNAlogo logo;

  logo.set_title("sample");

  logo.map_color("A", "green");
  logo.map_color("G", "gold");
  logo.map_color("C", "royalblue");
  logo.map_color("U", "crimson");
  logo.map_color("Ψ", "#9932CC");
  logo.map_color("Ψo", "#9932CC");

  logo.pict_table_bit
  (
   {{1,22,4,93},{1,24,29, 3},{12,1,19,39},{5,1,51,2},{23,1,66,3},{56,2,4,6}},
   {{"A","C","G","Ψ"},{"A","C","G","U"},{"A","C","G","Ψo"},{"A","C","G","U"},
    {"A","C","G","U"},{"A","C","G","U"}}, {}
   );

  return 0;
}
