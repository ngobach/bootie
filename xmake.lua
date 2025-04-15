add_requires("pugixml")
add_requires("argparse")

target("bootie")
  set_kind("binary")
  set_languages("c++20")
  add_rules("utils.bin2c", { extensions = { ".raw" }})
  add_rules("mode.release", "mode.debug")
  add_files("src/*.cpp")
  add_files("resources/gptdisk-64-sectors.raw")
  add_packages("pugixml")
  add_packages("argparse")
  if is_plat("mingw") then
    add_ldflags("-static")
  end
