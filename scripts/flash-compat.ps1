# ESP32 VR Haptic Trigger V3 — by alfawalidou / McWall
# GitHub: https://github.com/alfawalidou/esp32-vr-haptic-trigger-v3
# Discord: .mcwall | Telegram: @McWall07
# SPDX-License-Identifier: Apache-2.0

param(
  [Parameter(Mandatory = $true)]
  [string]$Port
)

& (Join-Path $PSScriptRoot "flash.ps1") -Port $Port
exit $LASTEXITCODE
