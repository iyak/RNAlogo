//
//  util.hpp
//  RNAlogo
//
//  Created by Hiroshi Miyake on 2016/11/09.
//  Copyright © 2016 Kiryu Lab. All rights reserved.
//

#ifndef util_h
#define util_h

#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include<sstream>
#include<istream>
#include<ostream>
#include<iomanip>
#include<sstream>
#include<algorithm>
#include<stdexcept>
#include<limits>
#include<numeric>
#include<random>
#include<thread>
#include<mutex>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<ctime>
#include<cfloat>
#include<cstdarg>
#include<cmath>
#include<unordered_map>
#include<sys/types.h>
#include<unistd.h>

namespace iyak {

  /* abbreviations */
  using std::abs;
  using std::lock_guard;
  using std::atof;
  using std::accumulate;
  using std::vector;
  using std::pair;
  using std::cout;
  using std::cerr;
  using std::endl;
  using std::to_string;
  using std::round;
  using std::string;
  using std::istream;
  using std::ifstream;
  using std::ofstream;
  using std::ostream;

  using V = std::vector<double>;
  using VV = std::vector<V>;
  using VVV = std::vector<VV>;
  using VVVV = std::vector<VVV>;

  using VI = std::vector<int>;
  using VVI = std::vector<VI>;

  using VS = std::vector<std::string>;
  using VVS = std::vector<VS>;

  using PII = std::pair<int, int>;
  using VPII = std::vector<PII>;

  using VB = std::vector<bool>;
  using VVB = std::vector<VB>;

  template<class T> using suomap = std::unordered_map<std::string, T>;

  using isstream = std::istringstream;
  using osstream = std::ostringstream;

  using mutex = std::mutex;
  using lock = std::lock_guard<mutex>;
  using ulock = std::unique_lock<mutex>;

  template<class T> using uptr = std::unique_ptr<T>;
  template<class T> using sptr = std::shared_ptr<T>;

  size_t constexpr npos = std::string::npos;
  double constexpr inf = std::numeric_limits<double>::infinity();
  int constexpr large = std::numeric_limits<int>::max();

  template<class T>
  std::ostream& operator <<(std::ostream& s, std::vector<T> const& v) {
    s << "[";
    for (auto e=v.begin(); e != v.end(); ++ e) {
      s << *e << (e == v.end()-1? "": ",");
    }
    return s << "]";
  }
  template<class T> string to_str(T const& t) {
    osstream oss; oss<<t; return oss.str();
  }
  template<class T> int size(T const&  v) {
    return static_cast<int>(v.size());
  }

  /* debug log */
  void _format(std::ostream& c) {c<<std::endl;}
  template<class T, class...A>
  void _format(std::ostream& c, T const& t, A const&...a) {c<<t<<' ';_format(c, a...);}
  template<class...A>void cry(A const&...a){_format(std::cerr, a...);}
  template<class...A>void die(A const&...a){cry(a...);throw std::runtime_error("");}
  template<class...A>void check(bool b, A const&...a){if(!b)die(a...);}
  template<class...A>void expect(bool b, A const&...a){if(!b)cry(a...);}

  /* stream manager */
  struct NullBuf: std::streambuf {int overflow(int c) {return c;}};
  NullBuf null_buf;
  std::ostream null_stream(&null_buf);
  suomap<std::ostream*> _map_strm = {
    {"~NULL~", &null_stream}, {"~COUT~", &std::cout}, {"~CERR~", &std::cerr}
  };
  std::vector<uptr<std::ostream>> _uptr_strm;
  std::vector<std::string> _nam_strm;
  string const get_ostream(int i) {return _nam_strm[i];}

  void init_ostream(int n) {
    check(1 <= n);
    _nam_strm.assign(n, "~NULL~");
    _uptr_strm.resize(n);
  }

  void set_ostream(int i, std::string s) {
    if (_map_strm.find(s) == end(_map_strm)) { /* if new stream (file) */
      _uptr_strm[i] = uptr<std::ostream>(new std::ofstream(s));
      std::ostream* os = _uptr_strm[i].get();
      check(!!*os, "cannot open:", s);
      _map_strm[s] = os;
    }
    _nam_strm[i] = s;
    /* no-op if already opened stream */
  }

  void fclear(int i) {
    std::string s = _nam_strm[i];
    if (s=="~NULL~" or s=="~COUT~" or s=="~CERR~") {return;}
    _uptr_strm[i] = uptr<std::ostream>(new std::ofstream(s));
    std::ostream* os = _uptr_strm[i].get();
    check(!!*os, "cannot open:", s);
    _map_strm[s] = os;
  }

  /* data log */
  template<class...A> void dat(int i, A const&... a) {
    _format(*(_map_strm[_nam_strm[i]]), a...);
  }
  template<class...A>void dat0(A const&... a) {
    _format(std::cout, a...);
  }
  template<class...A> void datp(int i, A const&... a) {
    auto& s = *(_map_strm[_nam_strm[i]]);
    auto const p = s.precision();
    s.precision(17);
    dat(i, a...);
    s.precision(p);
  }

  /* timer */
  clock_t chrn = clock();
  double lap(void) {
    double s = double(clock() - chrn) / CLOCKS_PER_SEC;
    chrn = clock();
    return s;
  }

  /* calculation */
  double logsumexp(double x, double y=-inf) {
    return
    (-inf==y)? x:
    (-inf==x)? y:
    x < y?
    y + log1p(exp(x - y)):
    x + log1p(exp(y - x));
  }

  template<class...T> double logsumexp(double x, T const&...y) {
    return logsumexp(x, logsumexp(y...));
  }

  template<class...T> void logaddexp(double& x, T const&...y) {
    x = logsumexp(x, y...);
  }

  template<class T> double logsumexp(std::vector<T> const& v) {
    double sum = -inf;
    for (auto const& e: v) {logaddexp(sum, logsumexp(e));}
    return sum;
  }

  void lnormal (std::vector<double> &v) {
    double sum = logsumexp(v);
    for (auto &e: v) {e -= sum;}
  }

  int max_index(V const& v) {
    int s = 0;

    double m = -inf;
    for (int i = 0; i < (int) v.size(); ++i )
      if (m <= v[i]) {
        s = i;
        m = v[i];
      }
    return s;
  }

  template<class T> T max(T x, T y) {return x<y? y: x;}
  template<class T, class...B> T max(T x, T y, B const&... z) {
    return max(max(x, y), z...);
  }
  template<class T> T min(T x, T y) {return x<y? x: y;}
  template<class T, class... B> T min(T x, T y, B const&...z) {
    return min(min(x, y), z...);
  }

  double norm2(double x) {return x * x;}
  template<class T> double norm2(vector<T> const& v) {
    double n = 0;
    for (auto vi: v) n += norm2(vi);
    return n;
  }

  /* string manipulation */
  template<class T>
  T iss_cast(std::string const& s) {
    if (0 == s.size()) return T();
    T val;
    isstream iss(s);
    iss >> val;
    return val;
  }

  string strip(string const& s, string const& drop=" \n") {
    if (0==size(s)) return "";
    size_t i = s.find_first_not_of(drop);
    size_t j = s.find_last_not_of(drop);
    if (npos==i or npos==j or j<i) return "";
    return s.substr(i, j-i+1);
  }

  template<class T>
  vector<T> split(string const& s, string const& delim) {
    if (0==size(strip(s))) return vector<T>();
    size_t p0 = 0;
    size_t p1 = 0;

    vector<T> elms {};
    while (npos != (p1=s.find(delim, p0))) {
      elms.push_back(iss_cast<T>(s.substr(p0, p1-p0)));
      p0 = p1 + delim.size();
    }
    elms.push_back(iss_cast<T>(s.substr(p0)));

    return elms;
  }

  template<>
  VS split<string>(string const& s, string const& delim) {
    size_t p0 = 0;
    size_t p1 = 0;

    VS elms {};
    while (npos != (p1=s.find(delim, p0))) {
      elms.push_back(s.substr(p0, p1-p0));
      p0 = p1 + delim.size();
    }
    elms.push_back(s.substr(p0));
    return elms;
  }

  template<class T>
  string paste(vector<T> const& v, string const& d="") {
    if (0==size(v)) return string();
    string a = to_str(v[0]);
    for (int i=1; i<size(v); ++i)
      a += d + to_str(v[i]);
    return a;
  }

  template<class B> string paste1(B b) {return to_str(b);}
  template<class...A,class B> string paste1(B const b, A const... a) {
    return to_str(b) + " " + paste1(a...);
  }
  template<class B> string paste0(B b) {return to_str(b);}
  template<class...A,class B> string paste0(B const b, A const... a) {
    return to_str(b) + paste0(a...);
  }

  /* misc */
  bool double_eq(double const x, double const y, double const eps=1e-10) {
    return
    (x==-inf) != (y==-inf)? false:
    (x==inf) != (y==inf)? false:
    x==-inf? y==-inf:
    x==inf? y==inf:
    0.==y? abs(x) < eps:
    0.==x? abs(y) < eps:
    abs(1.0 - x/y) < eps;
  }

  template<class T>
  bool any(vector<T> const& v, T const& e) {
    for (auto const& w:v) if (w==e) return true;
    return false;
  }
}

#endif /* util_h */