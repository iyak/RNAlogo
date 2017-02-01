from subprocess import Popen,PIPE
import sys

out="./build"

def exe(cmd):
    return Popen(cmd.split(), stdout=PIPE).stdout.read().decode("utf8").strip()

def options(opt):
    opt.load("compiler_cxx")

def configure(cnf):
    cnf.load("compiler_cxx")
    cnf.find_program("freetype-config", var="FTCNF")

    cnf.check_cfg(
            path="freetype-config",
            args="--cflags --libs",
            package="",
            uselib_store="freetype"
            )

    if (sys.platform.startswith("win") or sys.platform.startswith("cygwin")):
        print("for windows, I don't get path for fonts.")
        pass
    elif (sys.platform.startswith("darwin")):
        ttfs = exe("find /Library/Fonts -name *.ttf")
        for ttf in ttfs.split("\n"):
            if ("gothic" in ttf or "Gothic" in ttf):
                cnf.env.append_unique("DEFINES", ["_DFONT=\"%s\""%ttf])
                print("set font:", ttf)
                break
        else:
            print("could not find font file.")
            print("please set manually")
    else:
        ttfs = exe("find /usr/share/fonts -name *.ttf")
        for ttf in ttfs.split("\n"):
            if ("gothic" in ttf or "Gothic" in ttf):
                cnf.env.append_unique("DEFINES", ["_DFONT=\"%s\""%ttf])
                print("set font:", ttf)
                break
        else:
            print("could not find font file.")
            print("please set manually")

def build(bld):

  bld(
      features="cxx cxxprogram",
      cxxflags="-Wall -std=c++11",
      source="RNAlogo/main.cpp",
      include="RNAlogo",
      target="bin/RNAlogo",
      uselib="freetype",
      )
