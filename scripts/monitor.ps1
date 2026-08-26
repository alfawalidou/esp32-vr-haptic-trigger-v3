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

$Pio = Resolve-PlatformIO
Write-Host "Serial monitor: $Port @ 115200 - RTS/DTR inactive"
& $Pio device monitor -p $Port -b 115200 --rts 0 --dtr 0
exit $LASTEXITCODE
