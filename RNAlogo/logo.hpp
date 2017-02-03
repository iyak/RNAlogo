//
//  logo.hpp
//  RNAlogo
//
//  Created by Hiroshi Miyake on 2017/01/31.
//  Copyright © 2017 Kiryu Lab. All rights reserved.
//

#ifndef logo_h
#define logo_h

#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<cmath>
#include<unordered_map>
#include<ft2build.h>
#include"util.hpp"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_BBOX_H

#ifndef _DFONT
#define _DFONT "You must manually pass font file (.ttf) path"
#endif

namespace iyak {

  class utfs {
    vector<char32_t> _codes;
    string _s;

    int utf8_size(unsigned char byte) {
      return
      byte<0xc0? 1:
      0xc0==(byte&0xe0)? 2:
      0xe0==(byte&0xf0)? 3:
      0xf0==(byte&0xf8)? 4:
      0xf8==(byte&0xfc)? 5:
      0xfc==(byte&0xfe)? 6:
      -1;
    }

    char32_t utf_unit(string s) {
      check(size(s)==utf8_size(s[0]));
      if (1==size(s)) return s[0];
      char32_t c = (s[0]&(0xff>>(size(s)+1))) << (6*(size(s)-1));
      for (int i=1; i<size(s); ++i) {
        c |= (s[i]&0x3f) << (6*(size(s)-1-i));
      }
      return c;
    }

  public:
    int len() {return size(_codes);}
    string str() {return _s;}
    char32_t code(int i) {return _codes.at(i);}
    vector<char32_t> codes() {return _codes;}
    utfs(string s) {
      for (int i=0; i<size(s);) {
        int n = utf8_size(s[i]);
        _codes.push_back(utf_unit(s.substr(i, n)));
        i += n;
      }
    }
  };

  class RNAlogoAlph {

    using FTV = FT_Vector;
    using FTVc = FTV const;

    string _font = _DFONT;
    FT_Library _lib;

    string _alph;
    string _path;

    vector<FT_Face> _faces;
    FT_Outline& o(FT_Face& f) {return f->glyph->outline;}
    FT_Glyph_Metrics& m(FT_Face& f) {return f->glyph->metrics;}

    int _h = 0;
    int _w = 0;

    void load_glyph(FT_Face& f, FT_ULong u) {

      auto err = FT_New_Face(_lib, _font.c_str(), 0, &f);
      check(!err, "fail open:", _font);
      check(f->face_flags&FT_FACE_FLAG_SCALABLE, "unscalable face");

      auto i = FT_Get_Char_Index(f, u);
      err = FT_Load_Glyph(f, i, FT_LOAD_NO_SCALE|FT_LOAD_NO_BITMAP);
      check(!err, "fail load glyph", _alph);

      err = FT_Outline_Embolden(&o(f), 50);
      check(!err, "fail embolden:", _alph);

      mapply(f, {{1,0},{0,-1}}); /* flip */

      double rx = double(m(f).horiBearingX) / m(f).width;
      double ry = double(m(f).horiBearingY) / m(f).height;
      mapply(f, {{rx,0},{0,ry}}); /* shrink */
    }

    void mapply (FT_Face& f, VV m) {
      FT_Fixed scaler = 1<<16;
      FT_Matrix mm {
        .xx=static_cast<FT_Fixed>(round(m[0][0]*scaler)),
        .xy=static_cast<FT_Fixed>(round(m[0][1]*scaler)),
        .yx=static_cast<FT_Fixed>(round(m[1][0]*scaler)),
        .yy=static_cast<FT_Fixed>(round(m[1][1]*scaler)),
      };
      FT_Outline_Transform(&o(f), &mm);
    }

    struct bbox {
      int x,y,w,h;
      bbox(FT_Pos a,FT_Pos b,FT_Pos c,FT_Pos d):
      x((int)a),y((int)b),w((int)c),h((int)d){}
    };

    bbox calc_bb (FT_Face& f) {
      FT_BBox bb;
      auto err = FT_Outline_Get_BBox(&o(f), &bb);
      check(!err, "fail get bb:", _alph);
      return bbox(bb.xMin,bb.yMin,bb.xMax-bb.xMin,bb.yMax-bb.yMin);
    }

    void shift_bb(FT_Face& f, int x, int y) {
      FT_Outline_Translate(&o(f), x, y);
    }

    void set_pos(FT_Face& f, int x, int y) {
      auto bb = calc_bb(f);
      shift_bb(f, x-bb.x, y-bb.y);
      bb = calc_bb(f);
    }

    void load_outline(string& alph) {

      auto utf = utfs(alph).codes();
      _faces.assign(size(utf), FT_Face{});

      _w = _h = 0;
      for (int i=0; i<size(utf); ++i) {
        auto& f = _faces[i];

        load_glyph(f, utf[i]);
        auto bb = calc_bb(f);
        _w += bb.w;
        _h = max(_h, bb.h);

        set_pos(f, 0, 0);
      }

      int w = 0;
      for (auto& f: _faces) {
        auto bb = calc_bb(f);
        shift_bb(f, w, _h-bb.h);
        w += bb.w;
      }
    }

    void set_font(string font) {
      _font = font;
    }

    void set_alph(string alph) {
      _alph = alph;
      auto err = FT_Init_FreeType(&_lib);
      check(!err, "fail init");
      load_outline(alph);
    }

  public:

    RNAlogoAlph(string font, string alph) {
      set_font(font);
      set_alph(alph);
    }

    string put_svg_path(umap<char32_t,string>& map) {
      string s = "";

      for (int i=0; i<size(_faces); ++i) {
        auto& f = _faces[i];
        s += "<path d=\"\n";

        FT_Outline_Funcs callback {
          .move_to = [](FTVc* to, void* ptr) {
            auto s = (string*)ptr;
            *s += paste1("M", to->x, to->y, "\n");
            return 0;
          },
          .line_to = [](FTVc* to, void* ptr) {
            auto s = (string*)ptr;
            *s += paste1("L", to->x, to->y, "\n");
            return 0;
          },
          .conic_to = [](FTVc* c, FTVc* to, void* ptr) {
            auto s = (string*)ptr;
            *s += paste1("Q", c->x, c->y, ",", to->x, to->y, "\n");
            return 0;
          },
          .cubic_to = [](FTVc* c0, FTVc* c1, FTVc* to, void* ptr) {
            auto s = (string*)ptr;
            *s += paste1("C", c0->x, c0->y, ",", c1->x, c1->y,
                         ",", to->x, to->y, "\n");
            return 0;
          }
        };
        FT_Outline_Decompose(&o(f), &callback, &s);

        s += "\" fill=\"" + map[utfs(_alph).code(i)] +
        "\" stroke=\"black\" stroke-width=\"10\"/>\n";
      }
      return s;
    }

    void place(int x, int y, double w, double h) {
      for (auto& f: _faces) {
        mapply(f, {{w/_w,0.},{0.,h/_h}});
        shift_bb(f, x, y);
      }
    }
  };

  struct logo_pair {
    double val;
    RNAlogoAlph alph;
    logo_pair(double v, string f, string a): val(v), alph(f, a) {}
    bool static cmp(logo_pair& a, logo_pair& b) {return a.val < b.val;}
  };

  class RNAlogo {

    using table_t = vector<vector<logo_pair>>;
    table_t _table;
    umap<char32_t, string> _color_map {
      {0x00000041, "#339541"}, // A
      {0x00000047, "#F5C000"}, // G
      {0x00000043, "#545FFF"}, // C
      {0x00000055, "#D21010"}, // U
      {0x00000054, "#D21010"}, // T
    };
    string _font = _DFONT;

    string _title = "";
    VS _meta {};

    int _colw = 1000;
    int _space = 50;
    int _rowh = 5000;
    int _titleh = 500;
    int _yaxisw = 500;
    int _yrulerl = 100;
    int _xaxish = 500;
    int _metah = 500;

    double _scale;
    double _max_v;

    string _svg_header_footer =
    "<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
    "viewBox=\"$VIEWBOX\">\n$PATHTAG$AXESTAG$TITLETAG$METATAG</svg>";

    int spaced(int a) {return a;}
    template<class...T>
    int spaced(int a,T...b) {return a + spaced(b...) + (0<a? _space:0);}

    string put_svg_viewbox() {
      return paste1
      (0,0,
       spaced(_yaxisw, _yrulerl, (_colw+_space)*size(_table)),
       spaced(_titleh, _rowh, _metah, _xaxish));
    }

    string put_svg_path() {
      string s = "";
      for (auto& ti: _table)
        for (auto& tij: ti)
          s += tij.alph.put_svg_path(_color_map);
      return s;
    }

    string put_svg_axes() {
      string s = "";
      if (0<_yaxisw) {
        /* y axis */
        s = paste1
        (
         "<path d=\"M",
         spaced(_yaxisw, _yrulerl),
         spaced(_titleh, _rowh - _scale*(int(_max_v))),
         "L",
         spaced(_yaxisw, _yrulerl),
         spaced(_titleh, _rowh),
         "\" stroke=\"black\" stroke-width=\"15\"/>\n"
         );

        for (int i=0; i<=_max_v+1e-10; ++i) {
          /* y ruler */
          s += paste1
          (
           "<path d=\"M",
           _yaxisw + _space,
           spaced(_titleh, _rowh-round(_scale*i)),
           "L",
           spaced(_yaxisw, _yrulerl),
           spaced(_titleh, _rowh-round(_scale*i)),
           "\" stroke=\"black\" stroke-width=\"40\"/>\n"
           );
          /* y ruler text */
          s += paste0
          (
           "<text text-anchor=\"end\" x=\"", _yaxisw,
           "\" y=\"", spaced(_titleh, _rowh-round(_scale*i)),
           "\" font-size=\"", _yaxisw,
           "\" alignment-baseline=\"middle",
           "\">", to_str(i), "</text>\n"
           );
        }
      }
      if (0<_xaxish) {
        /* x axis text */
        for (int i=0; i<size(_table); ++i) {
          s += paste0
          (
           "<text text-anchor=\"middle\" x=\"",
           spaced(_yaxisw, _yrulerl, (_colw+_space)*i + _colw/2),
           "\" y=\"", spaced(_titleh, _rowh, _metah, _xaxish),
           "\" font-size=\"", _xaxish,
           "\" alignment-baseline=\"baseline"
           "\">", to_str(i+1), "</text>\n"
           );
        }
      }
      return s;
    }

    string put_svg_title() {
      string s = "";
      if (0<_titleh) {
        s += paste0
        (
         "<text text-anchor=\"middle\" x=\"",
         spaced(_yaxisw, _yrulerl, ((_colw+_space)*size(_table)-_space)/2),
         "\" y=\"", _titleh,
         "\" font-size=\"", _titleh,
         "\" alignment-baseline=\"baseline",
         "\">", _title, "</text>\n"
         );
      }
      return s;
    }

    string put_svg_meta() {
      string s = "";
      if (0<_metah) {
        for (int i=0; i<size(_table); ++i) {
          s += paste0
          (
           "<text text-anchor=\"middle\" x=\"",
           spaced(_yaxisw, _yrulerl, (_colw+_space)*i + _colw/2),
           "\" y=\"", spaced(_titleh, _rowh, _metah),
           "\" font-size=\"", _metah,
           "\" alignment-baseline=\"baseline",
           "\">", _meta[i], "</text>\n"
           );
        }
      }
      return s;
    }

    string put_svg() {
      string s = _svg_header_footer;
      size_t i;
      while (npos != (i=s.find("$VIEWBOX"))) s.replace(i,8,put_svg_viewbox());
      while (npos != (i=s.find("$PATHTAG"))) s.replace(i,8,put_svg_path());
      while (npos != (i=s.find("$AXESTAG"))) s.replace(i,8,put_svg_axes());
      while (npos != (i=s.find("$METATAG"))) s.replace(i,8,put_svg_meta());
      while (npos != (i=s.find("$TITLETAG"))) s.replace(i,9,put_svg_title());
      return s;
    }

    V v2bit (V const w) {
      V v(w);
      double sum = 0;
      for (auto vv: v) sum += vv;
      if (0!=sum) for (auto& vv: v) vv /= sum;

      double signal = log2(size(v));
      for (auto vv: v) if (0!=vv) signal += vv * log2(vv);

      for (auto& vv: v) vv = vv * signal;
      return v;
    }

    string pict_logo(VV const& vv,
                     VVS const& aa,
                     VS const& meta,
                     double const max_v) {

      _table.clear();
      check(size(vv)==size(aa), "not same dim:", vv, aa);
      for (int i=0; i<size(vv); ++i) {

        check(size(vv[i])==size(aa[i]), "not same dim:", vv, aa);
        _table.push_back(vector<logo_pair>{});
        for (int j=0; j<size(vv[i]); ++j)
          _table.back().emplace_back(vv[i][j], _font, aa[i][j]);

        std::sort(_table.back().begin(), _table.back().end(), logo_pair::cmp);
      }

      _max_v = max_v;
      _scale = _rowh / _max_v;
      _titleh = max(_titleh, _yaxisw);
      auto x = spaced(_yaxisw, _yrulerl) + _space;

      for (auto t: _table) {
        auto y = spaced(_titleh, _rowh); /* baseline of table */
        for (auto tt: t) {
          tt.alph.place(x, y-tt.val*_scale, _colw, tt.val*_scale);
          y -= tt.val*_scale;
        }
        x += _colw + _space ;
      }

      if (size(vv)==size(meta)) {_meta = meta;}
      else {_metah = 0;}

      return put_svg();
    }

  public:

    /* setters */
    void set_font(string const& font) {_font = font;}
    void set_title(string t) {_title = t;}
    void set_column_width(int column_width) {_colw = column_width;}
    void set_row_height(int row_height) {_rowh = row_height;}
    void set_title_height(int title_height) {_titleh = title_height;}
    void set_x_axis_height(int x_axis_height) {_xaxish = x_axis_height;}
    void set_y_axis_width(int y_axis_width) {_yaxisw = y_axis_width;}
    void set_meta_height(int meta_height) {_metah = meta_height;}
    void set_space(int space) {_space = space;}

    void map_color(std::string s, string c) {
      auto utf = utfs(s).codes();
      for (auto u: utf) _color_map[u] = c;
    }

    string pict_table_bit(VV const& vv,
                          VVS const& aa,
                          VS const& meta={}) {

      int maxd=0;
      VV v(vv);
      for (auto& vi: v) {
        maxd = max(maxd, size(vi));
        vi = v2bit(vi);
      }
      return pict_logo(v, aa, meta, log2(maxd));
    }

    string pict_table_freq(VV const& vv,
                           VVS const& aa,
                           VS const& meta={}) {

      VV v(vv);
      for (auto& vi: v) {
        double sum = 0;
        for (auto vij: vi) sum += vij;
        for (auto& vij: vi) vij /= sum;
      }

      return pict_logo(v, aa, meta, 1.);
    }
  };
}

#endif /* logo_h */
