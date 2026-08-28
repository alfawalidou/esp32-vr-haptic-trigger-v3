# ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall
# GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
# Discord: .mcwall | Telegram: @McWall07
# SPDX-License-Identifier: Apache-2.0

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

Write-Host ""
Write-Host "ESP32 VR Haptic Trigger V3 - Beginner First Flash"
Write-Host "by alfawalidou / McWall"
Write-Host "Discord .mcwall | Telegram @McWall07"
Write-Host "=================================================="
Write-Host "Repository: $RepoRoot"
Write-Host ""
Write-Host "IMPORTANT:" -ForegroundColor Yellow
Write-Host "- External battery / main actuator power must be OFF during USB flashing." -ForegroundColor Yellow
Write-Host "- Do not intentionally power this prototype from USB and the external main supply at the same time." -ForegroundColor Yellow
Write-Host "- OLED VCC must use the validated external regulated 5 V buck output, not ESP32 3V3." -ForegroundColor Yellow
Write-Host "- Verify the recoil and rumble driver wiring before enabling the COMPAT build." -ForegroundColor Yellow
Write-Host ""

Write-Host "Detected serial devices:"
& $Pio device list
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
$Port = (Read-Host "Enter the ESP32 COM port (example: COM12)").Trim().ToUpperInvariant()

if ($Port -notmatch '^COM[0-9]+$') {
  throw "Invalid COM port '$Port'. Expected a value such as COM12."
}

Write-Host ""
Write-Host "Final wiring checklist:" -ForegroundColor Cyan
Write-Host "  GPIO13 = Trigger"
Write-Host "  GPIO14 = Profile / Mode"
Write-Host "  GPIO17 = Rumble MOSFET control"
Write-Host "  GPIO23 / GPIO5 = BTS7960 control"
Write-Host "  GPIO21 / GPIO22 = OLED SDA / SCL"
Write-Host "  OLED VCC = external regulated 5 V buck OUT"
Write-Host "  GPIO4 = unused"
Write-Host ""

$Confirm = (Read-Host "Type YES only if the wiring and power stages have been verified").Trim()
if ($Confirm -cne "YES") {
  Write-Host "Cancelled. No erase or flash was performed."
  exit 1
}

Write-Host ""
Write-Host "Starting validated COMPAT clean + build + full erase + upload..." -ForegroundColor Green

& (Join-Path $PSScriptRoot "flash.ps1") -Port $Port
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "FIRST FLASH COMPLETE" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:"
Write-Host "1. Disconnect USB."
Write-Host "2. Power the completed unit from its normal external power system."
Write-Host "3. Connect Bluetooth to: ForceTubeVR 1187883197"
Write-Host "4. Test Bluetooth haptics in HAPTIC_ONLY."
Write-Host "5. Hold GPIO14 for about 1 second to enter TRIGGER_FALLBACK."
Write-Host "6. Test GPIO13 with the PISTOL profile first."
Write-Host ""
Write-Host "Beginner walkthrough: START_HERE.md"
Write-Host "Safe serial monitor if needed:"
Write-Host "powershell -ExecutionPolicy Bypass -File .\scripts\monitor.ps1 -Port $Port"
