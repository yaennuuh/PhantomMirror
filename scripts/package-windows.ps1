param(
  [string]$BuildDir = "build",
  [string]$Config = "Release",
  [string]$PackageDir = "package",
  [string]$ArchivePath = "Phantom-Mirror-windows-x64.zip",
  [switch]$IncludeNdiRuntime,
  [string]$ChecksumPath = ""
)

$ErrorActionPreference = "Stop"

$exePath = Join-Path $BuildDir "$Config\Phantom Mirror.exe"
if (-not (Test-Path $exePath)) {
  throw "Phantom Mirror.exe not found at $exePath. Build the project first."
}

function Get-CMakeCacheValue([string]$Name) {
  $cachePath = Join-Path $BuildDir "CMakeCache.txt"
  if (-not (Test-Path $cachePath)) {
    return $null
  }

  $line = Get-Content $cachePath | Where-Object { $_ -match "^$([regex]::Escape($Name)):" } | Select-Object -First 1
  if (-not $line) {
    return $null
  }

  return ($line -replace "^[^=]+=", "").Trim()
}

$windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
if (-not $windeployqt) {
  $qtPrefix = Get-CMakeCacheValue "CMAKE_PREFIX_PATH"
  if (-not $qtPrefix) {
    $qt6Dir = Get-CMakeCacheValue "Qt6_DIR"
    if ($qt6Dir) {
      $qtPrefix = Split-Path (Split-Path (Split-Path $qt6Dir -Parent) -Parent) -Parent
    }
  }

  if ($qtPrefix) {
    $candidate = Join-Path $qtPrefix "bin\windeployqt.exe"
    if (Test-Path $candidate) {
      $windeployqt = Get-Item $candidate
    }
  }
}

if (-not $windeployqt) {
  throw "windeployqt was not found. Add the Qt MSVC bin directory to PATH."
}

$windeployqtPath = if ($windeployqt.Source) { $windeployqt.Source } else { $windeployqt.FullName }

if (-not $ChecksumPath) {
  $ChecksumPath = Join-Path $PackageDir "SHA256SUMS.txt"
}

Remove-Item -Recurse -Force $PackageDir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $PackageDir | Out-Null
Copy-Item $exePath $PackageDir

$buildOutputDir = Split-Path $exePath -Parent
if ($IncludeNdiRuntime) {
  foreach ($extraRuntime in @("Processing.NDI.Lib.x64.dll", "Processing.NDI.Lib.Licenses.txt")) {
    $runtimePath = Join-Path $buildOutputDir $extraRuntime
    if (Test-Path $runtimePath) {
      Copy-Item $runtimePath $PackageDir -Force
    }
  }
}

& $windeployqtPath --release --no-compiler-runtime (Join-Path $PackageDir "Phantom Mirror.exe")
if ($LASTEXITCODE -ne 0) {
  throw "windeployqt failed with exit code $LASTEXITCODE."
}

$qtBin = Split-Path $windeployqtPath -Parent
$qtRoot = Split-Path $qtBin -Parent
$licenseCandidates = @(
  (Join-Path $qtRoot "Licenses"),
  (Join-Path (Split-Path $qtRoot -Parent) "Licenses")
) | Where-Object { Test-Path $_ } | Select-Object -First 1

$noticeDir = Join-Path $PackageDir "licenses"
New-Item -ItemType Directory -Force $noticeDir | Out-Null

if ($licenseCandidates) {
  Get-ChildItem $licenseCandidates -File |
    Where-Object { $_.Name -match "LGPL|GPL|LICENSE|QT" } |
    Copy-Item -Destination $noticeDir -Force
}

@"
Phantom Mirror uses Qt 6 Widgets dynamically through Qt DLLs.

Qt is provided under the GNU Lesser General Public License version 3 for the
Qt modules used by this application. Phantom Mirror does not modify Qt and does not
statically link Qt. Users may replace the shipped Qt DLLs with compatible
modified versions.

Qt licensing information:
https://doc.qt.io/qt-6/licensing.html
https://www.qt.io/licensing/open-source-lgpl-obligations
"@ | Set-Content (Join-Path $noticeDir "QT-LGPL-NOTICE.txt") -Encoding UTF8

Remove-Item $ArchivePath -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $PackageDir "*") -DestinationPath $ArchivePath -Force

$checksums = @()

$archiveHash = Get-FileHash -Algorithm SHA256 $ArchivePath
$checksums += "{0} *{1}" -f $archiveHash.Hash.ToLowerInvariant(), (Split-Path $ArchivePath -Leaf)

$packagedExe = Join-Path $PackageDir "Phantom Mirror.exe"
if (Test-Path $packagedExe) {
  $exeHash = Get-FileHash -Algorithm SHA256 $packagedExe
  $checksums += "{0} *{1}" -f $exeHash.Hash.ToLowerInvariant(), ("package/" + (Split-Path $packagedExe -Leaf))
}

$checksums | Set-Content $ChecksumPath -Encoding UTF8
$checksums | ForEach-Object { Write-Host $_ }
