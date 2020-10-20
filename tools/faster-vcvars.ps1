param([string]$target = 64)

if($target -eq 32) { $target = 86 }

$target = "x${target}"

$path_vs = "C:\Program Files (x86)\Microsoft Visual Studio"

$path_winkit = "C:\Program Files (x86)\Windows Kits\10"

$latest_vs = (get-childItem $path_vs | ? { $_.name -match "^20\d\d$" } | sort -Descending)[0].fullName
$latest_vs = (get-item "$latest_vs\Community\VC\Tools\MSVC\*").fullName

$msvc = "$latest_vs\bin\Hostx64\$target\"
$msvc_lib = "$latest_vs\lib\$target\"
$msvc_include = "$latest_vs\include\"

$winkit_version = (get-childItem "$path_winkit\bin" | ? { $_.name -match "^10\..*$" } | sort -Descending)[0].Name

$latest_winkit = "$path_winkit\bin\$winkit_version\x64"
$lib_ucrt = "$path_winkit\lib\$winkit_version\ucrt\$target"
$lib_um = "$path_winkit\lib\$winkit_version\um\$target"
$libpath_metadata = "$path_winkit\UnionMetadata\$winkit_version"
$libpath_references = "$path_winkit\References\$winkit_version"
$include = "$path_winkit\include\$winkit_version"

$env:path += "$msvc;$latest_winkit;$path_winkit\bin\x64;$latest_vs\Community\MSBuild\Current\Bin;"
$env:lib += "$msvc_lib;$lib_ucrt;$lib_um;"
$env:libpath += "$msvc_lib;$msvc_lib\store\references;$libpath_metadata;$libpath_references;"
$env:include += "$msvc_include;$include\ucrt;$include\shared;$include\um;$include\winrt;$include\cppwinrt;"