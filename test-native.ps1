$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
 foreach($name in @('nativecheck','controllercheck','chaincheck')){
  & ".\build\$name.exe"
  if($LASTEXITCODE -ne 0){throw "$name failed: $LASTEXITCODE"}
 }
 $isActor=Test-Path 'package/MCM/LukesActorBrowser.json'
 $kind=if($isActor){'Actor'}else{'Item'}
 $code=Get-Content 'src/browser.cpp' -Raw
 if($code -match 'Direct3DCreate9|DrawPrimitiveUP|presentHook|endHook|GetProcAddress",'){throw 'Graphics interception remains in native edition'}
 [xml]$xml=Get-Content "package/menus/prefabs/Lukes$($kind)Browser/Native.xml" -Raw
 if($xml.rect.name -ne "Lukes$($kind)Native"){throw 'Wrong native root'}
 for($i=0;$i -lt 256;$i++){
  if(!($xml.rect.image | Where-Object name -eq "R$i") -or !($xml.rect.text | Where-Object name -eq "T$i")){throw "Missing tile $i"}
 }
 foreach($name in @('White','Cursor','NativeBackground')){
  $data=[IO.File]::ReadAllBytes((Join-Path $PWD "package/textures/Interface/Lukes$($kind)Browser/$name.dds"))
  if([Text.Encoding]::ASCII.GetString($data,0,4) -ne 'DDS '){throw 'Invalid texture'}
  $w=[BitConverter]::ToUInt32($data,16);$h=[BitConverter]::ToUInt32($data,12)
  if($data.Length -ne 128+4*$w*$h){throw 'Truncated texture'}
 }
 $m=Get-Content "package/MCM/Lukes$($kind)Browser.json" -Raw|ConvertFrom-Json
 if(($m.submenus.'0'.options.PSObject.Properties.Value.vars | Where-Object configINI -eq 'Display:RenderScale')){throw 'Obsolete overlay resolution setting'}
 foreach($script in Get-ChildItem "package/NVSE" -Recurse -File | Where-Object Extension -in @('.gek','.txt')){
  foreach($line in Get-Content $script.FullName){
   $prefix=($line -split ';',2)[0]
   if(([regex]::Matches($prefix,'"')).Count%2){throw "Unbalanced script quotes in $($script.Name)"}
  }
 }
 if($isActor){& .\build\actorcheck.exe;if($LASTEXITCODE -ne 0){throw 'Actor validation failed'}}
 else {& .\test-security.ps1}
 Write-Output 'PASS: native XML/assets, no graphics-hook code, MCM native settings, script quote checks and browser regressions. Actual game XML rendering remains unverified.'
} finally {Pop-Location}

