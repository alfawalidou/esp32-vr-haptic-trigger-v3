# Profiles and local controls

Local firing is active only in `TRIGGER_FALLBACK`.

## Operating modes

### HAPTIC_ONLY

The physical trigger is observed by the UI/telemetry but does not create a local recoil event. Haptic output comes from the Bluetooth ForceTube-compatible stream.

### TRIGGER_FALLBACK

The physical trigger creates haptic output using the selected local profile.

Hold GPIO14 for about one second to toggle modes.

## Profiles

| Profile | Type | Kick | Manual rumble | Behavior |
|---|---|---:|---:|---|
| PISTOL | SINGLE | 120 | 0 | one reduced kick, no rumble |
| SNIPER | SINGLE | 255 | 128 | full kick + 120 ms rumble pulse; holding trigger does not extend rumble |
| M16 | AUTO | 240 | 125 | kick every 150 ms; rumble remains active while held |
| P90 | AUTO | 220 | 159 | kick every 150 ms; rumble remains active while held |
| PKM | AUTO | 129 | 255 | kick every 150 ms; rumble remains active while held |
| LASER | CHARGE_RELEASE | 255 | 0â†’255 | 20-step charge over 2600 ms; release performs KICK then stops rumble |

The 150 ms automatic cadence is an inherited tuning value, not a claim about real firearm cadence.

## Profile button

```text
short press  -> next profile
long press   -> HAPTIC_ONLY <-> TRIGGER_FALLBACK
```

Profile order:

```text
PISTOL -> SNIPER -> M16 -> P90 -> PKM -> LASER -> PISTOL
```