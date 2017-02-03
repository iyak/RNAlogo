//
//  main_stack.cpp
//  RNAlogo
//
//  Created by Hiroshi Miyake on 2017/02/04.
//  Copyright © 2017 Kiryu Lab. All rights reserved.
//

#include<string>
#include<iostream>
#include"util.hpp"
#include"logo.hpp"

using namespace iyak;

int main(int argc, char* argv[]) {
  umap<char,int> c2i {{'A',0},{'C',1},{'G',2},{'U',3}};
  VV stack {};
  for (string line; std::cin >> line;) {
    line.erase(line.end());
    stack.resize(line.length(),{0,0,0,0});
    for (int i=0; i<line.length(); ++i)
      stack[i][c2i[line[i]]] += 1;
  }
  RNAlogo logo;
  std::cout<<logo.pict_table_bit(stack,VVS(stack.size(),{"A","C","G","U"}));
  return 0;
}
