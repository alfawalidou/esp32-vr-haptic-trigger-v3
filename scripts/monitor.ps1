param(
  [string]$Port = "COMx"
)

pio device monitor -p $Port -b 115200 --rts 0 --dtr 0