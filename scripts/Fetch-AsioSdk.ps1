[CmdletBinding()]
param(
    [string]$Destination = (Join-Path $PSScriptRoot "..\third_party\asio-sdk"),
    [string]$Version = "2.3.4",
    [string]$ArchiveUrl = "https://download.steinberg.net/sdk_downloads/ASIO-SDK_2.3.4_2025-10-15.zip"
)
$ErrorActionPreference = "Stop"
$Destination = [System.IO.Path]::GetFullPath($Destination)
$Parent = Split-Path -Parent $Destination
$Archive = Join-Path $env:TEMP "ASIO-SDK-$Version.zip"
New-Item -ItemType Directory -Force -Path $Parent | Out-Null
Write-Host "Downloading official Steinberg ASIO SDK $Version..."
Invoke-WebRequest -Uri $ArchiveUrl -OutFile $Archive
if (!(Test-Path $Archive) -or (Get-Item $Archive).Length -lt 100000) {
    throw "SDK download is missing or unexpectedly small."
}
$Stage = Join-Path $env:TEMP ("ASIO-SDK-extract-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force -Path $Stage | Out-Null
try {
    Expand-Archive -LiteralPath $Archive -DestinationPath $Stage -Force
    $Header = Get-ChildItem -Path $Stage -Filter "iasiodrv.h" -Recurse -File | Select-Object -First 1
    if (!$Header) { throw "Downloaded archive does not contain common/iasiodrv.h; refusing to install it." }
    $SdkRoot = Split-Path -Parent $Header.FullName
    if ((Split-Path -Leaf $SdkRoot) -ne "common") { throw "Unexpected SDK directory layout." }
    $SdkRoot = Split-Path -Parent $SdkRoot
    if (Test-Path $Destination) { Remove-Item -LiteralPath $Destination -Recurse -Force }
    Copy-Item -LiteralPath $SdkRoot -Destination $Destination -Recurse
    $License = Join-Path $Destination "LICENSE.txt"
    if (!(Test-Path $License)) { throw "SDK top-level LICENSE.txt is missing." }
    Write-Host "ASIO SDK $Version installed locally at $Destination"
    Write-Host "Review LICENSE.txt and all file-level notices. SDK files are intentionally not committed to this repository."
}
finally {
    if (Test-Path $Stage) { Remove-Item -LiteralPath $Stage -Recurse -Force }
    if (Test-Path $Archive) { Remove-Item -LiteralPath $Archive -Force }
}
