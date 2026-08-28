# ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall
# GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
# Discord: .mcwall | Telegram: @McWall07
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

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
$Environment = "trigger-v3-wokwi"

Write-Host "ESP32 VR Haptic Trigger V3 - Wokwi build"
Write-Host "by alfawalidou / McWall"
Write-Host "Repository : $RepoRoot"
Write-Host "Environment: $Environment"
Write-Host ""

& $Pio run -d $RepoRoot -e $Environment -t clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Pio run -d $RepoRoot -e $Environment
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Firmware = Join-Path $RepoRoot ".pio\build\trigger-v3-wokwi\firmware.bin"
$Elf = Join-Path $RepoRoot ".pio\build\trigger-v3-wokwi\firmware.elf"

Write-Host ""
Write-Host "WOKWI BUILD COMPLETE" -ForegroundColor Green
Write-Host "Firmware: $Firmware"
Write-Host "ELF     : $Elf"
Write-Host ""
Write-Host "With the Wokwi VS Code extension installed, open diagram.json and start the simulator."
Write-Host "Serial terminal commands: KICK 255, RUMBLE 128, RUMBLE 0, BT OFF, BT ON, HELP"
Write-Host "See docs\WOKWI_SIMULATION.md for the full walkthrough."
