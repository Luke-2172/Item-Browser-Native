$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'test-native.ps1')
$package = Join-Path $PSScriptRoot 'package'
foreach ($file in @('NVSE/Plugins/LukesItemBrowser.dll','NVSE/Plugins/LukesItemBrowser/zlib1.dll')) {
    if (!(Test-Path -LiteralPath (Join-Path $package $file))) { throw 'Run build.cmd before packaging' }
}
$out = Join-Path $PSScriptRoot 'dist'
New-Item -ItemType Directory -Force $out | Out-Null
Add-Type -AssemblyName System.IO.Compression
$path = Join-Path $out 'Lukes-Item-Browser-FNV-1.0.10-native3.zip'
$stream = [IO.File]::Open($path,[IO.FileMode]::Create)
$zip = [IO.Compression.ZipArchive]::new($stream,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($file in Get-ChildItem $package -Recurse -File) {
        if ($file.FullName -match '\\Fonts\\') { continue }
        if ($file.DirectoryName -eq $package -and $file.Name -notin @('README.txt','NATIVE-UI-EDITION.txt')) { continue }
        if ($file.Extension -notin @('.dll','.ini','.txt','.gek','.json','.ttf','.xml','.dds')) { continue }
        $relative = $file.FullName.Substring($package.Length+1).Replace('\','/')
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip,$file.FullName,$relative,[IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
} finally { $zip.Dispose(); $stream.Dispose() }
Write-Output $path

