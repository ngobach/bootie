add_requires("pugixml")
add_requires("argparse")
add_rules("mode.release", "mode.debug")

target("bootie")
  set_kind("binary")
  set_languages("c17", "c++20")
  add_files("src/*.cpp")
  add_packages("pugixml")
  add_packages("argparse")
  if is_plat("mingw") then
      add_ldflags("-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lwinpthread -Wl,-Bdynamic", {force = true})
  end
