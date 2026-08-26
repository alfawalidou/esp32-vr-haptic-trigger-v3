# Trigger V3 — journal de validation

Ce fichier conserve les faits observés pendant le développement afin de distinguer ce qui est **confirmé sur le vrai prototype** de ce qui est seulement prévu ou compilé.

Trigger V3 a été initialisée depuis l'état Trigger V2 validé au commit `a12cb55fb74f9841d71e40638f5904380e004428`. Le commit initial du dossier V3 est `154a5cb48b905daf665866fdc142a089f3674c4a`. Les entrées V2 ci-dessous restent des preuves historiques.

## Références Git importantes

| SHA | Rôle |
|---|---|
| `f3a64d1ed18c12d1cf94405fae246c9c5e585362` | `main`, archive/documentation de la référence historique connue fonctionnelle |
| `8a453f704627a490db1c56e89ce9f1777836d596` | étape expérimentale avec dashboard Wi-Fi |
| `b3d1bc43e25ea688fee36b230c95664087f67f58` | Trigger V2 complète sans Wi-Fi, baseline historique utilisée pour construire V3 |

Les commits de documentation postérieurs à `b3d1bc43…` ne changent pas le firmware sauf mention contraire.

## 1. Référence historique récupérée

Source connue pour avoir été compilée, flashée et utilisée avec succès sur le prototype :

```text
Vr_gun_code_with_leds_pwm_solenoid_1_
```

Comportement historique retenu :

- Bluetooth Classic SPP ;
- nom `ForceTubeVR 1187883197` ;
- protocole 4 octets `0x2A 0xB0 channel intensity` ;
- channel 0 KICK ;
- channel 1 RUMBLE ;
- GPIO17 rumble ;
- GPIO23 RPWM ;
- GPIO5 LPWM ;
- GPIO16 WS2812 ;
- solénoïde min PWM 215, max 255 ;
- forward environ 30 ms.

La bibliothèque BTS7960 historique de Luis Llamas a été retrouvée et inspectée. Son comportement de direction a été reproduit directement dans Trigger V2, tandis que son `Stop()` historique `(0,1)` a été remplacé volontairement par `(0,0)`. L'effet secondaire GPIO0 de l'ancienne construction a été supprimé.

## 2. Backup complet du prototype historique

Un dump full-flash 4 MB du même ESP32/prototype a été créé avec Poco Firmware Center. Il est stocké localement, hors GitHub, comme image de récupération d'urgence uniquement.

Le vrai ESP32 a été interrogé avec esptool :

```text
Chip type: ESP32-D0WD-V3 (revision v3.1)
Features: Wi-Fi, BT, Dual Core + LP Core, 240MHz
Crystal frequency: 40MHz
Detected flash size: 4MB
Flash voltage: 3.3V
```

Cette vérification a justifié l'utilisation temporaire de la table `min_spiffs.csv` à deux slots OTA.

## 3. Trigger V2 SAFE — première validation

Le firmware SAFE a été compilé et flashé sur le vrai prototype.

Les actionneurs ont été confirmés comme inhibés par build. L'application Android de test a pu se connecter en Bluetooth et envoyer des commandes ForceTube.

Logs réels observés :

```text
[FT] KICK intensity=255 result=INHIBITED
[SAFE] actuator request blocked source=BLUETOOTH_KICK
[FT] RUMBLE intensity=255 result=INHIBITED
[SAFE] actuator request blocked source=BLUETOOTH_RUMBLE
[FT] KICK intensity=0 result=ACCEPTED
[FT] RUMBLE intensity=0 result=ACCEPTED
```

Interprétation :

- transport Bluetooth OK ;
- parser ForceTube OK ;
- intensités décodées ;
- gate SAFE actif ;
- les commandes zéro restent acceptées comme état d'arrêt.

## 4. Expérience Wi-Fi/dashboard — abandonnée

Une étape intermédiaire avait ajouté dashboard, SoftAP et provisioning Wi-Fi.

Premier dashboard sans provisioning : flash très élevé, environ 1,748 MB dans le slot de 1,966 MB.

Après provisioning : environ 1,802 MB et ~78 KB RAM statique.

Sur le vrai prototype, le firmware entrait dans une boucle de reboot. Log :

```text
[WIFI] Provisioning fallback: VR-HapticGun-58E8
[WIFI] Portal: http://192.168.4.1/
...
abort() was called ...
Rebooting...
```

Le backtrace décodé montrait notamment :

```text
NetworkEvents::postEvent(...)
_arduino_event_cb(...)
operator new(std::nothrow)
operator new(...)
std::terminate()
abort()
```

Symptômes physiques :

- AP apparaissait puis disparaissait ;
- client n'obtenait pas une connexion durable ;
- Bluetooth devenait inutilisable pendant les resets.

Décision : supprimer définitivement Wi-Fi/dashboard/provisioning de Trigger V2 et concentrer le firmware sur Bluetooth/haptique.

## 5. Trigger V2 complète sans Wi-Fi

Commit :

```text
b3d1bc43e25ea688fee36b230c95664087f67f58
feat: complete Trigger V2 and remove Wi-Fi
```

Builds rapportés :

```text
SAFE
RAM   45 764 / 327 680 = 14.0 %
Flash 1 141 184 / 1 966 080 = 58.0 %
Margin 824 896

COMPAT
RAM   45 764 / 327 680 = 14.0 %
Flash 1 141 112 / 1 966 080 = 58.0 %
Margin 824 968
```

Fonctions logicielles présentes :

- HAPTIC_ONLY ;
- TRIGGER_FALLBACK ;
- SINGLE ;
- AUTO ;
- profils PISTOL/SNIPER/M16/P90/LASER ;
- GPIO13 Trigger ;
- GPIO4 Profile ;
- GPIO14 Mode ;
- GPIO27 buzzer ;
- OLED SSD1306 0x3C ;
- BootHealth ;
- watchdog rumble corrigé ;
- rate limiter kick partagé ;
- Bluetooth ForceTube prioritaire ;
- zéro Wi-Fi.

## 6. Trigger V2 COMPAT — validation réelle du socle historique

`trigger-v2-compat` a été flashé sur le vrai prototype.

Boot réel :

```text
[BOOT] Trigger V2
[HEALTH] WS2812 GPIO16            CONFIGURED
[HEALTH] Trigger GPIO13           RESERVED
[HEALTH] Profile GPIO4            RESERVED
[HEALTH] Mode GPIO14              RESERVED
[HEALTH] Buzzer GPIO27            RESERVED
[HEALTH] I2C GPIO21/22            CONFIGURED
[HEALTH] OLED I2C 0x3C            NOT DETECTED
[HEALTH] Rumble GPIO17            CONFIGURED
[HEALTH] BTS7960 GPIO5/23         CONFIGURED
[HEALTH] Solenoid                 CONFIGURED
[HEALTH] Bluetooth SPP            CONFIGURED
[HEALTH] ForceTube protocol       CONFIGURED
[BT] ready name=ForceTubeVR 1187883197
[MODE] HAPTIC_ONLY
[FIRE] SINGLE
[PROFILE] PISTOL kick=230 auto_ms=150
[ACTUATORS] COMPAT / ENABLED
```

Connexion Android :

```text
[BT] connected
```

Commandes réelles observées :

```text
[FT] RUMBLE intensity=205 result=ACCEPTED
[FT] RUMBLE intensity=0 result=ACCEPTED
[FT] KICK intensity=255 result=ACCEPTED
[FT] RUMBLE intensity=255 result=ACCEPTED
[FT] KICK intensity=0 result=ACCEPTED
[FT] RUMBLE intensity=0 result=ACCEPTED
```

Plusieurs séries ont été reçues sans le crash Wi-Fi précédent.

Une occurrence :

```text
[FT] KICK intensity=255 result=INHIBITED
```

est apparue au milieu de kicks rapprochés. L'explication attendue est le rate limiter global de 150 ms ; ce résultat n'est pas un défaut à lui seul.

Le propriétaire a confirmé apprécier le résultat physique de cette version COMPAT.

## 7. État physique actuel à la fin de cette phase

**Validé réellement :**

- ESP32 cible ;
- flash 4 MB ;
- Bluetooth Classic SPP ;
- nom ForceTube ;
- parser ForceTube ;
- application Android de test ;
- SAFE gate ;
- fonctionnement COMPAT du socle historique ;
- BTS7960/solénoïde via commandes Bluetooth ;
- rumble via commandes Bluetooth ;
- LEDs historiques ;
- absence OLED gérée sans erreur.

**À valider après câblage :**

- GPIO13 Trigger ;
- GPIO14 Mode ;
- GPIO4 Profile ;
- SINGLE local ;
- AUTO local ;
- profils locaux ;
- OLED présent ;
- buzzer ;
- coexistence buzzer/rumble LEDC ;
- tuning intensités et cadences ;
- session Meta Quest 3/jeu réel ;
- watchdog rumble isolé de 500 ms si l'application de test envoie normalement `RUMBLE 0` trop vite pour le démontrer.

## 8. Prochaine étape officielle

Le développement d'architecture est considéré comme terminé jusqu'à preuve contraire.

La prochaine étape est :

```text
GPIO13 Trigger + GPIO14 Mode
        ↓
trigger-v2-safe
        ↓
boot guard / press-release
        ↓
HAPTIC_ONLY / TRIGGER_FALLBACK
        ↓
SINGLE / AUTO logique
        ↓
trigger-v2-compat SINGLE
        ↓
validation mécanique
        ↓
Profile GPIO4
        ↓
OLED
        ↓
Buzzer
        ↓
tuning
```

Pour toute nouvelle session de diagnostic, utiliser `AI_HANDOFF_PROMPT.md` et fournir les nouveaux logs/mesures plutôt que de repartir d'une architecture vierge.

## 9. Trigger V3 — OLED et migration deux boutons — 25/08/2026

Configuration physique :

```text
GPIO13 = TRIGGER
GPIO14 = PROFILE
GPIO4  = libre
```

Le firmware deux boutons a été compilé avec succès en `trigger-v3-safe`, puis flashé sur le vrai ESP32.

Boot observé :

```text
[BOOT] Trigger V3
[HEALTH] Trigger GPIO13           RESERVED
[HEALTH] Profile GPIO14           RESERVED
[HEALTH] OLED I2C 0x3C            DETECTED     I2C ACK; UI initialized
[HEALTH] Bluetooth SPP            CONFIGURED
[HEALTH] ForceTube protocol       CONFIGURED
[BT] ready name=ForceTubeVR 1187883197
[MODE] HAPTIC_ONLY
[PROFILE] PISTOL behavior=SINGLE kick=230
[ACTUATORS] SAFE / DISABLED
```

PROFILE court a parcouru :

```text
PISTOL -> SNIPER -> M16 -> P90 -> LASER -> PISTOL
```

PROFILE long :

```text
[PROFILE] long press -> toggle operating mode
[MODE] TRIGGER_FALLBACK
[PROFILE] long press -> toggle operating mode
[MODE] HAPTIC_ONLY
```

Le Trigger GPIO13 a été détecté en `HAPTIC_ONLY` :

```text
[TRIGGER] pressed mode=HAPTIC_ONLY profile=SNIPER behavior=SINGLE
[TRIGGER] released
```

Validé à cette date :

- OLED physique ;
- PROFILE GPIO14 court ;
- PROFILE GPIO14 long ;
- boucle des profils ;
- HAPTIC/TRIG toggle ;
- TRIGGER GPIO13 PRESS/RELEASE ;
- Bluetooth / ForceTube ;
- gate SAFE ;
- GPIO4 non requis.

Reste à valider :

- tir local GPIO13 en `TRIGGER_FALLBACK` avec SAFE ;
- SINGLE local complet ;
- AUTO local complet ;
- premier trigger local COMPAT ;
- buzzer ;
- tuning final M16/P90 ;
- LASER local.

## 10. Trigger V3 — validation locale COMPAT PISTOL / SNIPER / M16 / P90 — 25/08/2026

`trigger-v3-compat` a été compilé avec succès puis flashé sur le prototype réel.

Boot :

```text
[MODE] HAPTIC_ONLY
[PROFILE] PISTOL behavior=SINGLE kick=230
[ACTUATORS] COMPAT / ENABLED
```

### PISTOL / SINGLE

```text
[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=PISTOL behavior=SINGLE
[TRIGGER] local kick profile=PISTOL intensity=230 result=ACCEPTED
[TRIGGER] released
```

Retour utilisateur : physiquement nickel.

Résultat : **VALIDÉ COMPAT**.

### SNIPER / SINGLE

```text
[PROFILE] SNIPER behavior=SINGLE kick=255
[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=SNIPER behavior=SINGLE
[TRIGGER] local kick profile=SNIPER intensity=255 result=ACCEPTED
[TRIGGER] released
```

Retour utilisateur : physiquement nickel.

Résultat : **VALIDÉ COMPAT**.

### M16 / AUTO

Courte rafale observée :

```text
[PROFILE] M16 behavior=AUTO kick=240 auto_ms=150
[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=M16 behavior=AUTO
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] released
```

Retour utilisateur : physiquement nickel.

Résultat : **AUTO local COMPAT validé fonctionnellement**. La valeur `150 ms` reste un tuning temporaire et n'est pas considérée comme cadence réelle mesurée.

### P90 / AUTO

Courte rafale observée :

```text
[PROFILE] P90 behavior=AUTO kick=220 auto_ms=150
[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=P90 behavior=AUTO
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] released
```

Retour utilisateur : physiquement nickel.

Résultat : **AUTO local COMPAT validé fonctionnellement**.

### État après ces tests

```text
PISTOL SINGLE local COMPAT = VALIDÉ
SNIPER SINGLE local COMPAT = VALIDÉ
M16 AUTO local COMPAT      = VALIDÉ
P90 AUTO local COMPAT      = VALIDÉ
```

Aucun stalling, maintien solénoïde, brownout ou reboot rapporté.

### Rumble Xbox 360 — à traiter avant LASER

Les moteurs rumble n'ont pas été observés pendant ces essais. C'est cohérent avec le firmware local actuel : PISTOL/SNIPER/M16/P90 `SINGLE`/`AUTO` demandent des KICK mais pas de RUMBLE.

Le chemin rumble physique doit maintenant être validé séparément :

```text
GPIO17
 -> PWM 175 Hz / 8 bit
 -> IRLZ44N
 -> moteurs Xbox 360
```

Cette étape précède la calibration LASER, puisque LASER `CHARGE_RELEASE` dépend d'une rampe RUMBLE.
