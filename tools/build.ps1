param([string]$platform='64', [bool]$release=$false)
$debug_build_command = $false

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
# -P							Preprocessor output only
# /link
# /MACHINE:X64
# -LIBPATH:lib                  Add library folder
# -INCREMENTAL:NO               Disable incremental linking
# -OPT:REF                      Linker optimizations

$project = "Project_A"
$includes = "include", "include\3rdParty"
$libraries = "user32.lib", "winmm.lib", "gdi32.lib", "d3d11.lib", "d3dcompiler.lib", "dxguid.lib"
$ignored_warnings = "4100", "4127", "4201", "4458", "4706", "4505", "4459" , "4702", "4326"#"4189"
$exec_folder = if($release) { "bin\Release" } else { "bin\Debug" }

$compile_exe_opts = 
	"-nologo", 
	"-std:c++20",
	"-Zi", 
	"-Zc:offsetof-", 
	"-EHsc", 
	"-FC",
#	"-WX",
	"-W4" +
	($ignored_warnings | % { "-wd$_" }) +
	$(if($release) { "-MT", "-O2" } else { "-MTd", "-Od" }) +
    "-GR-" +
	($includes | % { "-I$_" }) +
	"-Fo$exec_folder\",
	"-Fd$exec_folder\",
#	"-Fa$exec_folder\",
#	"-P",
	"-D_CRT_SECURE_NO_WARNINGS",
	"-DUNICODE",
	"-D_UNICODE" +
	$(if($release) { } else { "-DDEBUG" })

$link_opts = 
	"-link",
	"-SUBSYSTEM:WINDOWS",
	"-MACHINE:X$platform",
	"-LIBPATH:lib",
	"-INCREMENTAL:NO",
	"-OPT:REF" +
	$libraries

if($debug_build_command) {
	echo "cl.exe $compile_exe_opts -Fe$exec_folder\$project.exe src\main.cpp $link_opts`n"
}

cl.exe $compile_exe_opts "-Fe$exec_folder\$project.exe" src\main.cpp $link_opts

# Only return the error if we are not running the game,
# because compilation of the .exe always fails when we are running
if(($LASTEXITCODE -ne 0) -and !(Get-Process $project -ErrorAction SilentlyContinue)) { exit 1 }