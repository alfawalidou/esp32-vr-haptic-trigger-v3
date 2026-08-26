param(
  [string]$Port = "COMx"
)

$ErrorActionPreference = "Stop"

pio run -e trigger-v3-compat -t clean
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

pio run -e trigger-v3-compat
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

pio run -e trigger-v3-compat -t erase --upload-port $Port
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

pio run -e trigger-v3-compat -t upload --upload-port $Port
exit $LASTEXITCODE