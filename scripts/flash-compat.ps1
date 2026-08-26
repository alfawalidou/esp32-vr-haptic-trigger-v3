param(
  [Parameter(Mandatory = $true)]
  [string]$Port
)

& (Join-Path $PSScriptRoot "flash.ps1") -Port $Port
exit $LASTEXITCODE
