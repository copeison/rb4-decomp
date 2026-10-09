[CmdletBinding()]
param(
    [string]$SdkDir = $env:SCE_ORBIS_SDK_DIR,
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$EmitDisassembly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repositoryRoot = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($SdkDir)) {
    $installedSdkRoot = "C:\Program Files (x86)\SCE\ORBIS SDKs"
    if (Test-Path -LiteralPath $installedSdkRoot) {
        $SdkDir = Get-ChildItem -LiteralPath $installedSdkRoot -Directory |
            Sort-Object { [version]$_.Name } -Descending |
            Select-Object -First 1 -ExpandProperty FullName
    }
}

if ([string]::IsNullOrWhiteSpace($SdkDir) -or
    -not (Test-Path -LiteralPath $SdkDir -PathType Container)) {
    throw "Set SCE_ORBIS_SDK_DIR or pass -SdkDir with a complete Orbis SDK."
}

$SdkDir = (Resolve-Path -LiteralPath $SdkDir).Path
$toolDirectory = Join-Path $SdkDir "host_tools\bin"
$compiler = Join-Path $toolDirectory "orbis-clang++.exe"
$archiver = Join-Path $toolDirectory "orbis-ar.exe"
$linker = Join-Path $toolDirectory "orbis-ld.exe"
$objdump = Join-Path $toolDirectory "orbis-objdump.exe"
$symbolTool = Join-Path $toolDirectory "orbis-nm.exe"

foreach ($tool in @($compiler, $archiver, $linker, $objdump, $symbolTool)) {
    if (-not (Test-Path -LiteralPath $tool -PathType Leaf)) {
        throw "Required SDK tool is missing: $tool"
    }
}

$env:SCE_ORBIS_SDK_DIR = $SdkDir
$outputDirectory = Join-Path $repositoryRoot "build\orbis\$Configuration"
$objectDirectory = Join-Path $outputDirectory "obj"
New-Item -ItemType Directory -Force -Path $objectDirectory | Out-Null

$commonFlags = @(
    "-std=c++1z",
    "-Wall",
    "-Wextra",
    "-Wno-missing-braces",
    "-Wno-unused-private-field",
    "-Wno-invalid-offsetof",
    "-I$($repositoryRoot)\src"
)
if ($Configuration -eq "Release") {
    $commonFlags += @("-O2", "-DNDEBUG")
} else {
    $commonFlags += @("-O0", "-g")
}

$sourceRoot = Join-Path $repositoryRoot "src"
$sources = Get-ChildItem -LiteralPath $sourceRoot -Recurse -Filter "*.cpp" |
    Sort-Object FullName
$objects = [System.Collections.Generic.List[string]]::new()
$manifest = [System.Collections.Generic.List[object]]::new()

foreach ($source in $sources) {
    $relativePath = $source.FullName.Substring($sourceRoot.Length + 1)
    $objectPath = Join-Path $objectDirectory ($relativePath -replace "\.cpp$", ".o")
    New-Item -ItemType Directory -Force `
        -Path (Split-Path -Parent $objectPath) | Out-Null

    Write-Host "Compiling $relativePath"
    & $compiler @commonFlags -c $source.FullName -o $objectPath
    if ($LASTEXITCODE -ne 0) {
        throw "Compilation failed: $relativePath"
    }

    $objects.Add($objectPath)
    $object = Get-Item -LiteralPath $objectPath
    $manifest.Add([pscustomobject]@{
        Source = ($relativePath -replace "\\", "/")
        Object = (
            $objectPath.Substring($repositoryRoot.Length + 1) -replace "\\", "/"
        )
        Size = $object.Length
        Sha256 = (Get-FileHash -LiteralPath $objectPath -Algorithm SHA256).Hash
    })

    if ($EmitDisassembly) {
        $assemblyPath = [System.IO.Path]::ChangeExtension($objectPath, ".asm")
        & $objdump -d -C $objectPath | Set-Content -LiteralPath $assemblyPath
        if ($LASTEXITCODE -ne 0) {
            throw "Disassembly failed: $relativePath"
        }
    }
}

$archivePath = Join-Path $outputDirectory "librb4_reconstruction.a"
if (Test-Path -LiteralPath $archivePath) {
    Remove-Item -LiteralPath $archivePath
}
& $archiver rcs $archivePath @objects
if ($LASTEXITCODE -ne 0) {
    throw "Archive creation failed: $archivePath"
}

$manifestPath = Join-Path $outputDirectory "objects.csv"
$manifest | Export-Csv -LiteralPath $manifestPath -NoTypeInformation

$combinedObjectPath = Join-Path $outputDirectory "rb4_reconstruction.o"
& $linker -r -o $combinedObjectPath @objects
if ($LASTEXITCODE -ne 0) {
    throw "Relocatable link failed: $combinedObjectPath"
}

$undefinedSymbolsPath = Join-Path $outputDirectory "undefined-symbols.txt"
& $symbolTool -u -C $combinedObjectPath |
    Set-Content -LiteralPath $undefinedSymbolsPath
if ($LASTEXITCODE -ne 0) {
    throw "Symbol extraction failed: $combinedObjectPath"
}

Write-Host "Compiled $($sources.Count) PS4 translation units."
Write-Host "Archive: $archivePath"
Write-Host "Combined object: $combinedObjectPath"
Write-Host "Manifest: $manifestPath"
Write-Host "Undefined symbols: $undefinedSymbolsPath"
