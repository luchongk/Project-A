param([string]$platform='64', [bool]$release=$false)

if($platform -eq '32') { $platform = '86' }

tools\faster-vcvars.ps1 $platform

# -nologo                       Don't show useless crap
# -c                            Compile without linking
# -Zi                           Debug format (PDB)
# -Zc:offsetof-                 Conformance to the standard
# -EHsc                         Exception model to use
# -FC                           Show full path of source code files
# -WX                           Treat warnings as errors
# -W4                           Warning level
# -wd4100                       Ignore warning 4100
# -MTd                          CRT linking mode
# -Oi                           Use intrinsics where possible
# -Od                           Optimization level
# -GR-                          Disable runtime type information
# -Iinclude                     Header files folder
# -"Febin\Debug\Gain.exe"       Executable folder
# -Fobin\Debug\                 Object files folder
# -Fdbin\Debug\                 Debug files folder
# -Faasm\                       Assembly files folder
# /link
# /MACHINE:X64
# -LIBPATH:lib                  Add library folder
# -INCREMENTAL:NO               Disable incremental linking
# -OPT:REF                      Linker optimizations

$includes = "include", "include\3rdParty"
$libraries = "user32.lib", "winmm.lib", "gdi32.lib", "glad64_d.lib", "opengl32.lib"
$ignored_warnings = "4100", "4127", "4201", "4458", "4706" #"4189", "4505"

$compileExeOpts = 
	"-nologo", 
	"-Zi", 
	"-Zc:offsetof-", 
	"-EHsc", 
	"-FC",
#	"-WX",
	"-W4" +
	$(foreach($warning in $ignored_warnings) { "-wd$warning" }) +
	$(if($release) { "-MT" } else { "-MTd", "-Od" }) +
	"-Oi",
    "-GR-" +
	$(foreach($inc in $includes) { "-I$inc" }) +
	"-Fobin\Debug\",
	"-Fdbin\Debug\",
#	"-Faasm\",
	"-DUNICODE",
	"-D_UNICODE"

$linkOpts = 
	"-link",
	"-MACHINE:X$platform",
	"-LIBPATH:lib",
	"-INCREMENTAL:NO",
	"-OPT:REF" +
	$libraries
	
rm src\generated.cpp -ErrorAction SilentlyContinue
#cl.exe $compileExeOpts -std:c++17 -Fetools\parser\ tools\parser\parser.cpp -Itools\parser\
tools\reflection_parser.exe src\generated.cpp include\

rm bin\Debug\engine-*.pdb -ErrorAction SilentlyContinue

cl.exe $compileExeOpts -LD -Febin\Debug\ src\engine.cpp $linkOpts -PDB:bin\Debug\engine-$(get-date -Format FileDateTime).pdb
if($LASTEXITCODE -ne 0) { exit 1; }

cl.exe $compileExeOpts "-Febin\Debug\Project_A.exe" src\platform_win32.cpp $linkOpts
if($LASTEXITCODE -ne 0) { exit 1; }
