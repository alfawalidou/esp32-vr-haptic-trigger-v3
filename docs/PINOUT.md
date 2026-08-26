# Final Trigger V3 pinout and breakout mapping

> Current standalone authority: GPIO13 = TRIGGER, GPIO14 = PROFILE/MODE, GPIO4 = unused. The original wiring document below is preserved because it contains the detailed 30-pin breakout mapping. Its older SAFE-only validation notes are historical; the final COMPAT build has since been physically validated.

# Trigger V3 — câblage final à deux boutons physiques

> **Statut : architecture deux boutons implémentée et testée physiquement en SAFE le 25/08/2026.**

Configuration retenue :

```text
GPIO13 = TRIGGER
GPIO14 = PROFILE
GPIO4  = libre
```

PROFILE GPIO14 :

```text
appui court     -> profil suivant
appui long ~1 s -> HAPTIC_ONLY <-> TRIGGER_FALLBACK
```

Il n'y a plus de bouton MODE séparé.

## Schémas et mapping

Le mapping corrigé du shield DIY reste la référence pour identifier les borniers :

![Mapping corrigé du shield DIY](assets/wiring/trigger-v3-diy-shield-pinmap-corrected.svg)

Version raster :

- [`trigger-v3-diy-shield-pinmap-corrected.png`](assets/wiring/trigger-v3-diy-shield-pinmap-corrected.png)

> **Important :** l'ancien schéma `trigger-v3-simplified-one-button-wiring.svg/png` montre encore PROFILE sur GPIO4. Il est désormais obsolète pour le câblage des boutons.

## Commandes

| Commande | GPIO | Fonction |
|---|---:|---|
| Trigger physique | 13 | Exécute le `ProfileBehavior` uniquement en `TRIGGER_FALLBACK` |
| PROFILE — appui court | 14 | Profil suivant |
| PROFILE — appui long ~1 s | 14 | `HAPTIC_ONLY` ↔ `TRIGGER_FALLBACK` |
| Bouton MODE séparé | — | supprimé |
| GPIO4 | — | libre / inutilisé |

Le comportement de tir est défini par le profil :

- `PISTOL` → `SINGLE`
- `SNIPER` → `SINGLE`
- `M16` → `AUTO`
- `P90` → `AUTO`
- `LASER` → `CHARGE_RELEASE` (`Uncalibrated`)

## Mapping exact des borniers du shield DIY

**Orientation : USB en bas.** Ne pas déduire le GPIO par proximité visuelle avec la rangée de broches ESP32.

### Rangée horizontale supérieure — gauche → droite

1. `D35 / GPIO35`
2. `D34 / GPIO34`
3. `VN / GPIO39`
4. `VP / GPIO36`
5. `EN`
6. `D23 / GPIO23`
7. `D22 / GPIO22`
8. `TX0 / GPIO1`
9. `RX0 / GPIO3`
10. `D21 / GPIO21`

### Rangée verticale gauche — haut → bas

1. `D32 / GPIO32`
2. `D33 / GPIO33`
3. `D25 / GPIO25`
4. `D26 / GPIO26`
5. `D27 / GPIO27`
6. `D14 / GPIO14`
7. `D12 / GPIO12`
8. `D13 / GPIO13`
9. `GND`
10. `VIN / 5V`

### Rangée verticale droite — haut → bas

1. `D19 / GPIO19`
2. `D18 / GPIO18`
3. `D5 / GPIO5`
4. `TX2 / GPIO17`
5. `RX2 / GPIO16`
6. `D4 / GPIO4`
7. `D2 / GPIO2`
8. `D15 / GPIO15`
9. `GND`
10. `3V3`

## Borniers utilisés par Trigger V3

| Fonction | GPIO ESP32 | Bornier du shield |
|---|---:|---|
| BTS7960 RPWM | 23 | `D23` — rangée haute, position 6 |
| OLED SCL | 22 | `D22` — rangée haute, position 7 |
| OLED SDA | 21 | `D21` — rangée haute, position 10 |
| BTS7960 LPWM | 5 | `D5` — côté droit, position 3 |
| Rumble MOSFET gate | 17 | `TX2` — côté droit, position 4 |
| WS2812 data | 16 | `RX2` — côté droit, position 5 |
| PROFILE | 14 | `D14` — côté gauche, position 6 |
| Buzzer | 27 | `D27` — côté gauche, position 5 |
| Trigger | 13 | `D13` — côté gauche, position 8 |
| OLED VCC | 3.3 V | `3V3` — côté droit, position 10 |
| OLED GND / boutons | GND | l'un des borniers `GND` |

### GPIO4

`D4 / GPIO4` est libre et n'est plus utilisé par les commandes physiques Trigger V3.

### GPIO14

`D14 / GPIO14` est le bouton **PROFILE** :

- appui court : profil suivant ;
- appui long ~1 s : `HAPTIC_ONLY` ↔ `TRIGGER_FALLBACK`.

## OLED SSD1306

```text
OLED GND -> GND
OLED VCC -> 3V3
OLED SDA -> GPIO21 / D21
OLED SCL -> GPIO22 / D22
```

Paramètres : SSD1306 `128x64`, adresse `0x3C`, SDA21/SCL22, I2C 400 kHz, timeout 20 ms.

## Trigger et PROFILE

Les deux contacts sont normalement ouverts et utilisent `INPUT_PULLUP` :

```text
GPIO13 ---- Trigger NO ---- GND
GPIO14 ---- PROFILE NO ---- GND
```

Aucune résistance pull-up externe n'est requise pour la logique actuelle.

## Alimentation et masse

- Masse commune obligatoire : ESP32, BTS7960 logique, MOSFET rumble, buck, OLED et boutons.
- OLED : utiliser `3V3`.
- Ne jamais alimenter le solénoïde ou les moteurs rumble depuis une GPIO ESP32.
- La puissance solénoïde reste séparée via BTS7960.

## Validation SAFE — 25/08/2026

Validé physiquement :

- OLED SSD1306 détecté à `0x3C` ;
- Bluetooth SPP connecté ;
- gate SAFE actif ;
- PROFILE GPIO14 court : boucle `PISTOL -> SNIPER -> M16 -> P90 -> LASER -> PISTOL` ;
- PROFILE GPIO14 long ~1 s : `HAPTIC_ONLY` ↔ `TRIGGER_FALLBACK` ;
- aucun changement de profil parasite pendant le long press ;
- Trigger GPIO13 : PRESS/RELEASE ;
- GPIO4 non requis.

Logs observés :

```text
[HEALTH] Trigger GPIO13           RESERVED
[HEALTH] Profile GPIO14           RESERVED
[HEALTH] OLED I2C 0x3C            DETECTED     I2C ACK; UI initialized

[PROFILE] long press -> toggle operating mode
[MODE] TRIGGER_FALLBACK
[PROFILE] long press -> toggle operating mode
[MODE] HAPTIC_ONLY

[TRIGGER] pressed mode=HAPTIC_ONLY profile=SNIPER behavior=SINGLE
[TRIGGER] released
```

## Encore à valider

1. passer en `TRIGGER_FALLBACK` avec PROFILE long ;
2. appuyer une fois sur Trigger GPIO13 en SAFE ;
3. vérifier une demande locale `INHIBITED` par le gate SAFE ;
4. tester SINGLE puis AUTO en SAFE ;
5. seulement ensuite envisager le premier test local COMPAT.

## Invariants à ne pas toucher

- Bluetooth Classic SPP `ForceTubeVR 1187883197`
- aucun Wi-Fi
- Forward 30 ms
- Reverse 2 ms à 25 %
- dead-time 100 µs
- watchdog rumble 500 ms
- guard `KICK=0`
- buffering ForceTube
- `HapticProfiles.h` reste la source de vérité des comportements
