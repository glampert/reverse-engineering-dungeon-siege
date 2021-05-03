
--
-- NOTE:
--  This premake script was only tested on Mac OSX.
--  It is tunned for the Clang compiler and might
--  not work out-of-the-box with GCC!
--
--  It could also benefit from a serious cleanup.
--  It has too much copy-pasting for my liking, but
--  I haven't had the chance of fixing that yet...
--

-----------------------------------------------------------
-- My "paranoid" set of compiler flags (tuned for Clang):
-----------------------------------------------------------
local COMMON_COMPILER_FLAGS = {
	-- misc:
	"-fstrict-aliasing",
	-- warnings:
	"-Wall",
	"-Weffc++",
	"-Wextra",
	"-Wformat=2",
	"-Wgcc-compat",
	"-Wglobal-constructors",
	"-Waddress-of-array-temporary",
	"-Wheader-guard",
	"-Wheader-hygiene",
	"-Winit-self",
	"-Wmissing-braces",
	"-Woverloaded-virtual",
	"-Wparentheses",
	"-Wpointer-arith",
	"-Wreturn-type",
	"-Wself-assign",
	"-Wsequence-point",
	"-Wshadow",
	"-Wstrict-aliasing",
	"-Wswitch",
	"-Wswitch-default",
	"-Wuninitialized",
	"-Wunknown-pragmas",
	"-Wunused",
	"-Wweak-template-vtables",
	"-Wweak-vtables",
	"-Wwrite-strings",
	"-Wdisabled-optimization",
	"-Wshorten-64-to-32"
};

-----------------------------------------------------------
-- C++11 flags for all C++ builds:
-----------------------------------------------------------
local CPLUSPLUS_FLAGS = {
	"-std=c++11",
	"-stdlib=libc++",
	"-g" -- FIXME: Remove -g (debug symbols) on "release" builds!
};

-----------------------------------------------------------
-- Misc constants:
-----------------------------------------------------------
local LIB_UTILS_NAME = "Utils";
local LIB_SIEGE_NAME = "Siege";

-----------------------------------------------------------
-- Common configurations for all projects:
-----------------------------------------------------------
solution("OpenSiege");
	configurations("Debug", "Release");
	location("build");
	targetdir("build");
	includedirs("source");
	defines({
		-- Global switches:
		"SIEGE_ENABLE_LOGGING=1",
		"SIEGE_LOG_FORCE_STDOUT=0",
		"SIEGE_TANK_DEBUG=1",
		"SIEGE_ASP_DEBUG=1",
		"SIEGE_SNO_DEBUG=1",
		-- Other debug switches:
		-- FIXME: These should not be here for a "release" build!
		"_LIBCPP_DEBUG=0",
		"_LIBCPP_DEBUG2=0",
		"_GLIBCXX_DEBUG",
		"_SECURE_SCL"
	}); -- defines

-----------------------------------------------------------
-- Utils static library:
-----------------------------------------------------------
project(LIB_UTILS_NAME);
	language("C++");
	kind("StaticLib");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/utils/*.hpp", "source/utils/*.cpp" });

-----------------------------------------------------------
-- Siege static library:
-----------------------------------------------------------
project(LIB_SIEGE_NAME);
	language("C++");
	kind("StaticLib");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/siege/*.hpp", "source/siege/*.cpp" });

-----------------------------------------------------------
-- tankdump command line tool:
-----------------------------------------------------------
project("tankdump");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/tools/tankdump/tankdump.cpp" });
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });

-----------------------------------------------------------
-- raw2tga command line tool:
-----------------------------------------------------------
project("raw2tga");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({
		"source/tools/raw2x/raw2x_base.hpp",
		"source/tools/raw2x/raw2x_base.cpp",
		"source/tools/raw2x/raw2tga.cpp"
	});
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });

-----------------------------------------------------------
-- raw2png command line tool:
-----------------------------------------------------------
project("raw2png");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({
		"source/tools/raw2x/raw2x_base.hpp",
		"source/tools/raw2x/raw2x_base.cpp",
		"source/tools/raw2x/raw2png.cpp"
	});
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });

-----------------------------------------------------------
-- asp2obj command line tool:
-----------------------------------------------------------
project("asp2obj");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/tools/asp2obj/asp2obj.cpp" });
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });

-----------------------------------------------------------
-- sno2obj command line tool:
-----------------------------------------------------------
project("sno2obj");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/tools/sno2obj/sno2obj.cpp" });
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });

-----------------------------------------------------------
-- tga2raw command line tool:
-----------------------------------------------------------
project("tga2raw");
	language("C++");
	kind("ConsoleApp");
	configuration("macosx", "linux", "gmake"); -- Debug & Release
	buildoptions({ COMMON_COMPILER_FLAGS, CPLUSPLUS_FLAGS });
	files({ "source/tools/tga2raw/tga2raw.cpp" });
	links({ LIB_UTILS_NAME, LIB_SIEGE_NAME });
