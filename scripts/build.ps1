param(
  [ValidateSet("Safe", "Compat")]
  [string]$Mode = "Compat"
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
$Environment = if ($Mode -eq "Safe") { "trigger-v3-safe" } else { "trigger-v3-compat" }

Write-Host "Repository : $RepoRoot"
Write-Host "Environment: $Environment"
Write-Host "PlatformIO : $Pio"

& $Pio run -d $RepoRoot -e $Environment -t clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $Pio run -d $RepoRoot -e $Environment
exit $LASTEXITCODE
