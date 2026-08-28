# ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall
# GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
# Discord: .mcwall | Telegram: @McWall07
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

param(
  [Parameter(Mandatory = $true)]
  [string]$Port
)

$ErrorActionPreference = "Stop"

function Resolve-PlatformIO {
  $cmd = Get-Command pio -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }

  $cmd = Get-Command platformio -ErrorAction SilentlyContinue
  if ($cmd) { return $cmd.Source }

  throw "PlatformIO CLI not found. Install it with: python -m pip install --upgrade platformio"
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Pio = Resolve-PlatformIO
$Environment = "trigger-v3-compat"

Write-Host "Repository : $RepoRoot"
Write-Host "Environment: $Environment"
Write-Host "Port       : $Port"

Write-Host "==> Clean"
& $Pio run -d $RepoRoot -e $Environment -t clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Build"
& $Pio run -d $RepoRoot -e $Environment
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Full flash erase"
& $Pio run -d $RepoRoot -e $Environment -t erase --upload-port $Port
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Upload"
& $Pio run -d $RepoRoot -e $Environment -t upload --upload-port $Port
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Flash completed successfully."
Write-Host "Monitor: powershell -ExecutionPolicy Bypass -File .\scripts\monitor.ps1 -Port $Port"
