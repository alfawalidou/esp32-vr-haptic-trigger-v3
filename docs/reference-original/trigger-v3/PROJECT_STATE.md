# Trigger V3 — état de référence du projet

> Document de continuité technique. À consulter avant toute modification du firmware ou du câblage.

## Référence actuelle

- Branche stable du dépôt : `main`, contenant la référence historique intacte et la baseline Trigger V2 validée.
- Branche de développement courante : `feature/trigger-v3`.
- Trigger V3 a été initialisée depuis l'état Trigger V2 validé au commit `a12cb55fb74f9841d71e40638f5904380e004428`.
- Le commit initial du dossier V3 est `154a5cb48b905daf665866fdc142a089f3674c4a`.
- Implémentation V2 de référence validée sur le socle historique : commit `b3d1bc43e25ea688fee36b230c95664087f67f58` (`feat: complete Trigger V2 and remove Wi-Fi`).
- Commit de promotion de Trigger V2 dans la baseline stable : `f4d000247aed3f69990724d42138a4a1d4198b03` (`chore: promote Trigger V2 as current baseline`).
- `variants/reference-leds-pwm-solenoid/Vr_gun_code_with_leds_pwm_solenoid_1_.ino` reste la référence historique matérielle intouchable ; ne pas la modifier pendant le développement Trigger V3.
- Cible : ESP32 classique `ESP32-D0WD-V3`, révision matérielle observée v3.1, flash physique 4 MB, quartz 40 MHz.
- Framework : PioArduino / Arduino Core ESP32 3.3.11, cible PlatformIO `esp32dev`.
- Canal externe runtime : **Bluetooth Classic SPP uniquement**.
- **Trigger V3 n'utilise aucun Wi-Fi.**

## Mise à jour V3 deux boutons — 25/08/2026

Configuration actuellement flashée et testée en SAFE :

```text
GPIO13 = TRIGGER
GPIO14 = PROFILE
GPIO4  = libre
```

PROFILE GPIO14 : court = profil suivant ; long ~1 s = `HAPTIC_ONLY` ↔ `TRIGGER_FALLBACK`.

Validé sur le vrai prototype :

- OLED SSD1306 `0x3C` sur GPIO21/22 ;
- Bluetooth SPP / ForceTube ;
- PROFILE court et boucle complète des profils ;
- PROFILE long et HAPTIC/TRIG toggle ;
- Trigger GPIO13 PRESS/RELEASE ;
- gate SAFE ;
- GPIO4 non requis.

Le test d'une demande de tir locale GPIO13 pendant `TRIGGER_FALLBACK` reste à effectuer en SAFE. SINGLE/AUTO local ne sont donc pas encore physiquement validés.

## Ce qui a été réellement validé sur le prototype

Le prototype historique a déjà servi de base fonctionnelle : Bluetooth ForceTube, BTS7960, solénoïde, rumble et deux WS2812.

### SAFE validé sur le vrai ESP32

`trigger-v2-safe` a été flashé sur le prototype réel. Les commandes Bluetooth sont reçues et parsées, tandis que le verrou SAFE bloque correctement les actionneurs. Exemples observés :

```text
[FT] KICK intensity=255 result=INHIBITED
[SAFE] actuator request blocked source=BLUETOOTH_KICK
[FT] RUMBLE intensity=255 result=INHIBITED
[SAFE] actuator request blocked source=BLUETOOTH_RUMBLE
```

La connexion Bluetooth avec l'application Android de test fonctionne.

### COMPAT validé sur le prototype historique

`trigger-v2-compat` a ensuite été flashé. Le boot est stable, Bluetooth se connecte, et les KICK/RUMBLE ForceTube sont acceptés. Exemples observés :

```text
[BT] connected
[FT] RUMBLE intensity=205 result=ACCEPTED
[FT] KICK intensity=255 result=ACCEPTED
```

Le solénoïde 1564B piloté par BTS7960 est maintenant validé avec 30 ms Forward, puis 2 ms Reverse à 25 %. Le retour physique est très propre, sans deuxième gros choc, blocage, brownout, reboot ou instabilité Bluetooth rapportés.

- SNIPER : `KICK 255`, tir unique validé ;
- M16 : `KICK 79`, rafale de trois coups validée ;
- P90 : `KICK 56` et `RUMBLE 125`, rafale soutenue validée.

L'ancien rate limiter ForceTube de 150 ms inhibait des kicks de rafale M16/P90 légitimes. Il a été retiré ; la protection naturelle contre le chevauchement reste `kickState_ != KickState::Idle`.

La correction Bluetooth validée arrête temporairement la consommation du buffer lorsqu'un KICK non nul démarre. Les commandes ForceTube suivantes restent en attente pendant Forward + Reverse au lieu d'être consommées prématurément, puis sont traitées après le cycle actif.

### Ce qui n'est PAS encore physiquement validé

À partir de la validation SAFE du 25/08/2026, restent à valider :

- demande de tir locale GPIO13 pendant `TRIGGER_FALLBACK` ;
- SINGLE local complet ;
- AUTO local complet ;
- premier tir local COMPAT ;
- buzzer GPIO27 ;
- coexistence buzzer/rumble LEDC ;
- tuning final M16/P90 ;
- calibration locale LASER ;
- session Meta Quest 3 / jeu réel.

Le logiciel peut être implémenté sans être encore matériellement validé.

## Architecture finale

```text
Meta Quest 3 OU application Android de test
                 |
          Bluetooth Classic SPP
                 |
          ForceTube parser
                 |
          HapticController
             |          |
          Rumble      BTS7960
                         |
                      Solenoid

GPIO13 Trigger
      |
Trigger logic
      |
ProfileBehavior (SINGLE / AUTO / CHARGE_RELEASE)
      |
TRIGGER_FALLBACK
      |
HapticController

UI locale :
- GPIO14 Profile : court = profil suivant ; long ~1 s = HAPTIC/TRIG
- GPIO4 libre
- GPIO21/22 OLED
- GPIO27 Buzzer
- GPIO16 2x WS2812
```

Bluetooth reste prioritaire. La gâchette locale est un complément pour les jeux qui n'envoient pas d'événement ForceTube.

## Protocole Bluetooth immuable

Nom SPP :

```text
ForceTubeVR 1187883197
```

Paquet ForceTube :

```text
byte 0 = 0x2A
byte 1 = 0xB0
byte 2 = channel
byte 3 = intensity
```

Canaux :

- `0x00` = KICK ;
- `0x01` = RUMBLE.

Le parser est streaming : il supporte paquets partiels, paquets adjacents, octets invalides et resynchronisation. Ne pas changer ce protocole sans preuve qu'un client réel l'exige.

## Baseline matérielle historique à préserver

| Fonction | Valeur |
|---|---|
| Rumble PWM | GPIO17 |
| BTS7960 RPWM | GPIO23 |
| BTS7960 LPWM | GPIO5 |
| WS2812 data | GPIO16 |
| LEDs | 2, brightness 50 |
| Rumble | 175 Hz, 8 bit |
| Solenoid PWM mapped | 215 à 255 |
| Forward kick | 30 ms |
| Reverse strength | 25 % |
| Reverse duration | 2 ms, validée physiquement à 25 % sur 1564B + BTS7960 |
| Rumble watchdog | 500 ms |

### Reconstruction du câblage rumble Xbox 360

- deux moteurs Xbox 360 récupérés de l'historique du prototype ;
- étage MOSFET IRLZ44N commandé par GPIO17 ;
- buck 12 V vers 5 V probablement utilisé pour l'ESP32 et les moteurs ;
- topologie la plus probable : commutation low-side, moteurs en parallèle ;
- masse commune obligatoire entre batterie, buck, ESP32 et Source du MOSFET.

Restent à vérifier physiquement : valeurs exactes des résistances, référence de la diode flyback, tension réellement mesurée aux moteurs, connexion parallèle/série, point 5V/VIN de l'ESP32, câblage IRLZ44N monté et routage réel des masses.

Voir [XBOX_RUMBLE_WIRING.md](XBOX_RUMBLE_WIRING.md) pour le schéma reconstruit, l'audit des illustrations et la distinction entre diode flyback rumble et TVS bidirectionnelle 1.5KE24CA du solénoïde.

### BTS7960 direct

Trigger V3 n'utilise pas l'ancienne bibliothèque BTS7960 et ne touche pas GPIO0.

Forward :

```cpp
analogWrite(RPWM, 0);
delayMicroseconds(100);
analogWrite(LPWM, pwm);
```

Reverse :

```cpp
analogWrite(LPWM, 0);
delayMicroseconds(100);
analogWrite(RPWM, pwm);
```

Stop :

```cpp
analogWrite(LPWM, 0);
analogWrite(RPWM, 0);
```

### Découverte historique : le pulse inverse sert au recentrage / anti-stall

Une relecture de la discussion Discord historique du projet apporte un contexte important qui manquait à la première reconstruction de Trigger V2.

En juin 2024, le problème identifié était que le solénoïde pouvait **ne pas être revenu à sa position initiale lorsque le kick suivant commençait**. La discussion envisage explicitement l'inversion momentanée de polarité pour accélérer la disparition du champ magnétique, tout en rappelant que le ressort et les éléments purement mécaniques gardent leur propre temps de retour.

En septembre 2025, un prototype comparable a ensuite été modifié d'un pilotage MOSFET simple vers un BTS7960. Son auteur indique avoir ajouté **un court pulse inverse pour recentrer le solénoïde**, et rapporte que cette modification a supprimé ses problèmes de stalling. Le code historique récupéré plus tard contient précisément :

```cpp
motor.TurnLeft(pwmValue);
delay(kickToReverseDelay);

int reversePWM = pwmValue * reverseKickPercent / 100;
motor.TurnRight(reversePWM);
motor.Stop();
```

avec `kickToReverseDelay` voisin de 30 ms et `reverseKickPercent = 25`.

Cette découverte change l'interprétation du reverse : il ne faut plus considérer la capacité Reverse de `HapticController` comme un vestige inutile. Elle a une motivation expérimentale réelle : **favoriser le retour/recentrage du plongeur et éviter qu'un tir suivant arrive alors que le mécanisme n'est pas revenu à son état initial**, particulièrement en rafale.

Cela ne signifie pas qu'inverser la polarité pousse forcément le plongeur mécaniquement dans l'autre sens comme un moteur DC. Selon le solénoïde, l'effet utile peut surtout être une démagnétisation active plus rapide, permettant ensuite au ressort de retour de faire son travail plus tôt.

La durée exacte du pulse historique n'est pas établie par les archives disponibles. Les essais physiques réalisés depuis sur le 1564B + BTS7960 fournissent maintenant la preuve spécifique au prototype actuel.

**Décision actuelle validée :**

- Reverse 1 ms à 25 % : fonctionnel ;
- Reverse 2 ms à 25 % : retour utilisateur très propre ;
- valeur retenue : `kReverseDurationMs = 2` et `kReverseKickPercent = 25` ;
- phase Forward conservée à `kKickDurationMs = 30` ;
- cycle conservé : `Forward -> Reverse -> Idle`, avec arrêt et dead-time avant changement de direction ;
- un `KICK=0` ne peut pas interrompre Forward ou Reverse ;
- LPWM et RPWM ne doivent jamais être actifs simultanément.

Cette phase doit préserver ces paramètres et le comportement Bluetooth validé ; elle ne doit pas relancer un tuning ou une refactorisation sans nouvelle preuve physique.

## SAFE et COMPAT

### `trigger-v3-safe`

Compile avec `TRIGGER_V3_ACTUATORS_ENABLED=0`.

Le firmware exécute Bluetooth, parser, boutons, trigger, profils, OLED, buzzer, LEDs et diagnostics, mais les demandes non nulles KICK/RUMBLE ne peuvent pas activer physiquement GPIO17/GPIO23/GPIO5.

### `trigger-v3-compat`

Compile avec `TRIGGER_V3_ACTUATORS_ENABLED=1`.

Le même logiciel est utilisé, mais les actionneurs historiques sont autorisés si l'initialisation PWM a réussi.

Tous les chemins — Bluetooth, trigger local, SINGLE, AUTO — passent par le même `HapticController`. Aucun de ces chemins ne doit écrire directement les GPIO d'actionneur.

## Trigger V3 optionnel

| Fonction | GPIO | Logique |
|---|---:|---|
| Trigger | 13 | bouton vers GND, `INPUT_PULLUP`, actif LOW |
| Profile | 14 | court = profil suivant ; long ~1 s = HAPTIC/TRIG |
| GPIO4 | — | libre / inutilisé |
| Buzzer | 27 | sortie tone, idle LOW |
| OLED SDA | 21 | I2C |
| OLED SCL | 22 | I2C |
| OLED | 0x3C | SSD1306 128x64 |

Trigger et Profile ont un debounce de 35 ms et un **boot-release guard** : une entrée maintenue LOW au reset n'est pas acceptée comme appui. Elle doit d'abord revenir HIGH puis être pressée de nouveau.

## Modes de fonctionnement

État au boot :

```text
Operating mode = HAPTIC_ONLY
Profile        = PISTOL
Behavior       = SINGLE
```

### HAPTIC_ONLY

- ForceTube KICK/RUMBLE fonctionne normalement.
- GPIO13 peut être diagnostiqué mais ne génère aucune action locale.

### TRIGGER_FALLBACK

- ForceTube reste inchangé.
- GPIO13 exécute le `ProfileBehavior` du profil sélectionné.

### Commandes GPIO14 / PROFILE

- appui court : profil suivant ;
- appui long ~1 s : `HAPTIC_ONLY` ↔ `TRIGGER_FALLBACK`.

GPIO14 n'est plus un bouton MODE indépendant. GPIO4 est libre.

Un changement de mode ou de profil, ainsi qu'une déconnexion Bluetooth, invalide le cycle local maintenu. Il faut relâcher puis rappuyer.

## ProfileBehavior

- `Single` : une demande KICK sur la pression, sans répétition au maintien ni second kick au release.
- `Auto` : demande immédiate puis répétition non bloquante selon `autoRepeatMs`; un refus pendant Forward/Reverse reste simplement `INHIBITED`.
- `ChargeRelease` : montée rumble discrète générique, maintien au maximum avec refresh inférieur au watchdog, puis release selon `ChargeReleaseOrder`.

## Profils locaux

[HapticProfiles.h](include/HapticProfiles.h) est l'unique catalogue des armes locales. Ajouter une arme qui utilise un comportement existant nécessite seulement de modifier ce fichier puis de recompiler.

| Profil | Behavior | Kick | AUTO | Charge | Couleur |
|---|---|---:|---:|---|---|
| PISTOL | SINGLE | 230 | 0 | désactivée | 0,0,255 |
| SNIPER | SINGLE | 255 | 0 | désactivée | 0,128,0 |
| M16 | AUTO | 240 | 150 ms | désactivée | 255,0,0 |
| P90 | AUTO | 220 | 150 ms | désactivée | 255,255,0 |
| LASER | CHARGE_RELEASE | 255 | 0 | 0→255, 20 steps, durée 0, ordre Uncalibrated | 255,255,255 |

Les 150 ms de M16/P90 sont une valeur héritée temporaire et non une cadence mesurée. Pour LASER, les 20 niveaux proviennent de l'observation APK, mais la durée totale et l'ordre physique de release ne sont pas calibrés. `chargeDurationMs=0` et `ChargeReleaseOrder::Uncalibrated` interdisent donc tout rumble local non nul et tout kick release local ; seule une télémétrie explicite est produite.

Le catalogue local ne modifie aucune intensité ForceTube. Le parser, le buffering Bluetooth des rafales M16/P90, le guard KICK=0 et `HapticController` restent inchangés.


## OLED

- SSD1306 128x64 ;
- adresse `0x3C` ;
- SDA 21, SCL 22 ;
- I2C 400 kHz ;
- timeout 20 ms ;
- rafraîchissement maximal toutes les 100 ms et seulement lorsque l'état UI est dirty.

Si aucun ACK n'est reçu, le firmware affiche `NOT DETECTED` et continue sans OLED.

La vue manuelle affiche le nom du profil et son `ProfileBehavior`, les barres dynamiques KICK/RUMBLE, RATE, `REV 2ms 25%`, l'état Bluetooth et le mode `HAPT`/`TRIG`. Une demande KICK locale non nulle est visible 150 ms même en SAFE. SINGLE affiche `RATE SINGLE`, AUTO calcule `~RPM = 60000 / autoRepeatMs` (M16 et P90 : `~400 RPM` actuellement), et LASER affiche `RATE UNCAL`.

Chaque commande ForceTube complète active `FT LIVE` pendant 1 500 ms. Cette vue représente les intensités KICK/RUMBLE reçues, y compris en SAFE, sans déduire d'identité d'arme. Le KICK non nul est latched 150 ms et un `KICK=0` ne l'efface pas prématurément. La cadence `~RPM` est une approximation de la cadence traitée, lissée sur les quatre derniers intervalles KICK non nuls valides et réinitialisée après 1 500 ms sans KICK.

GPIO13 conserve SINGLE/AUTO/CHARGE_RELEASE uniquement en `TRIGGER_FALLBACK`, GPIO4 conserve le cycle générique de profil, et GPIO14 conserve le toggle court ainsi que le long press réservé. Les LEDs et les fréquences/durée du buzzer sont inchangées. LASER reste `Uncalibrated`, sans action physique locale ni kick proportionnel.

À surveiller lors du vrai montage : câbles longs, mauvaise masse ou module marginal à 400 kHz. En cas d'instabilité I2C, tester temporairement 100 kHz avant de modifier davantage le firmware.

## Buzzer

GPIO27 produit des tonalités courtes non bloquantes :

- Profile : 1000 Hz ;
- Operating mode : 1200 Hz ;
- appui long GPIO14 réservé : 1600 Hz ;
- durée : 60 ms.

Le code utilise `tone()`. Un petit piezo/buzzer passif est le cas naturel. Ne pas alimenter directement depuis GPIO27 un buzzer qui demande un courant significatif : utiliser un transistor/driver si nécessaire. Un buzzer actif ne reproduira pas nécessairement les fréquences comme prévu.

Le buzzer et le rumble utilisent tous deux des ressources PWM/LEDC du framework. La coexistence doit être vérifiée physiquement lorsque le buzzer sera monté.

## LEDs

- LED 0 : rouge sans client Bluetooth, bleu connecté ;
- LED 1 : BootHealth pendant le boot ; après boot, état haptique, orange en SAFE ou couleur du profil en COMPAT.

Les animations sont non bloquantes.

## Watchdogs et arrêt sûr

- Rumble non nul : timestamp mis à jour uniquement lorsqu'une nouvelle commande RUMBLE valide arrive.
- Sans nouvelle commande, rumble coupé après 500 ms.
- Déconnexion Bluetooth : parser reset + `gHaptics.stopAll()`.
- Solénoïde : un kick ne peut démarrer que si le contrôleur est armé et `kickState_ == KickState::Idle`.
- SAFE : `setArmed()` reste false même si le reste du système est prêt.

## Décision Wi-Fi — NE PAS le réintroduire implicitement

Une version expérimentale avait ajouté Wi-Fi, dashboard et portail de provisioning. Sur le vrai ESP32, la combinaison Bluetooth Classic + pile Wi-Fi a provoqué des reboots en boucle. Le backtrace observé passait notamment par :

```text
NetworkEvents::postEvent()
operator new(std::nothrow)
operator new()
std::terminate()
abort()
```

L'AP apparaissait/disparaissait et Bluetooth devenait inutilisable. La décision finale a été de **retirer tout Wi-Fi de Trigger V2** et de réserver les ressources au gameplay haptique Bluetooth.

Une future IA ne doit donc pas réintroduire Wi-Fi, HTTP, dashboard, mDNS, DNSServer ou provisioning comme « amélioration » sans demande explicite et nouvelle décision d'architecture.

## Mémoire de la version complète sans Wi-Fi

Mesures rapportées au commit d'implémentation `b3d1bc43…` :

| Build | RAM | Flash | Marge slot OTA |
|---|---:|---:|---:|
| SAFE | 45 764 / 327 680 (14,0 %) | 1 141 184 / 1 966 080 (58,0 %) | 824 896 |
| COMPAT | 45 764 / 327 680 (14,0 %) | 1 141 112 / 1 966 080 (58,0 %) | 824 968 |

La table `min_spiffs.csv` est conservée temporairement avec deux slots OTA de 1 966 080 octets. Ne pas la modifier pendant la phase de validation Trigger V3 ; un retour vers une table standard doit être une phase séparée et volontaire.

## Sauvegarde et récupération

Un dump complet 4 MB du firmware historique connu fonctionnel est conservé localement par le propriétaire, hors GitHub. Il sert uniquement de récupération d'urgence pour l'ESP32 original.

Ne pas ajouter ce dump au dépôt. Ne pas considérer le dump comme source de développement. La source historique lisible reste la référence de comportement.

## Ordre de câblage recommandé

Ne pas connecter tous les nouveaux composants à la fois.

1. **GPIO13 Trigger + GPIO14 Mode uniquement.**
2. Revenir en `trigger-v3-safe` pour vérifier presses/releases, HAPTIC_ONLY, TRIGGER_FALLBACK, SINGLE/AUTO et la télémétrie CHARGE_RELEASE non calibrée sans mouvement.
3. Une fois SAFE validé, passer en COMPAT pour tester un seul kick local puis la cadence.
4. Ajouter GPIO4 Profile et valider le cycle des profils.
5. Ajouter l'OLED et valider l'I2C/affichage sans perturber Bluetooth.
6. Ajouter le buzzer en dernier et vérifier sa coexistence avec rumble/LEDC.

Cette séquence permet d'identifier immédiatement le composant responsable si un problème apparaît.

## Signaux d'arrêt immédiat pendant tests

Arrêter le test et revenir en SAFE si l'un de ces symptômes apparaît :

- reboot, brownout ou panic ;
- solénoïde activé dans le build SAFE ;
- rumble actif dans SAFE ;
- tir au boot sans nouvelle transition de bouton ;
- sortie solénoïde qui reste active ;
- driver, câbles ou bobine qui chauffent anormalement ;
- alimentation qui s'effondre ;
- Bluetooth qui se déconnecte systématiquement lors des kicks ;
- AUTO irrégulier ou mécanique trop rapide ;
- changement de mode qui déclenche un tir.

## Commandes de build et flash

Depuis `variants/trigger-v3` :

```powershell
$env:PLATFORMIO_CORE_DIR = "<LOCAL_PLATFORMIO_CORE_DIR>"

pio run -e trigger-v3-safe
pio run -e trigger-v3-compat
```

Flash SAFE :

```powershell
pio run -e trigger-v3-safe -t upload --upload-port COMx
pio device monitor -p COMx -b 115200
```

Flash COMPAT uniquement après validation SAFE :

```powershell
pio run -e trigger-v3-compat -t upload --upload-port COMx
pio device monitor -p COMx -b 115200
```

`COMx` est le port observé pendant cette phase de développement ; toujours refaire `pio device list` si le système change.

## État de sortie de cette phase

Trigger V3 a été initialisée depuis l'architecture Trigger V2 validée et se trouve maintenant en phase :

**câblage → validation progressive → tuning**.

L'objectif des prochaines sessions n'est pas de réécrire la V3, mais de diagnostiquer les écarts entre le comportement prévu et le vrai matériel, en conservant les invariants Bluetooth/ForceTube/HapticController tant qu'aucune preuve ne justifie de les changer.

## Validation COMPAT locale — 25/08/2026

Après validation complète des commandes locales en SAFE, `trigger-v3-compat` a été compilé, flashé et testé sur le prototype réel.

Boot COMPAT observé :

```text
[MODE] HAPTIC_ONLY
[PROFILE] PISTOL behavior=SINGLE kick=230
[ACTUATORS] COMPAT / ENABLED
```

Profils locaux physiquement validés en `TRIGGER_FALLBACK` :

| Profil | Behavior | Kick | Résultat physique |
|---|---|---:|---|
| PISTOL | SINGLE | 230 | validé, impact propre, retour solénoïde propre |
| SNIPER | SINGLE | 255 | validé, pleine intensité propre |
| M16 | AUTO | 240 | validé en courte rafale, répétition et arrêt au relâchement propres |
| P90 | AUTO | 220 | validé en courte rafale, répétition et arrêt au relâchement propres |

Exemples observés :

```text
[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=PISTOL behavior=SINGLE
[TRIGGER] local kick profile=PISTOL intensity=230 result=ACCEPTED
[TRIGGER] released

[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=SNIPER behavior=SINGLE
[TRIGGER] local kick profile=SNIPER intensity=255 result=ACCEPTED
[TRIGGER] released

[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=M16 behavior=AUTO
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] local kick profile=M16 intensity=240 result=ACCEPTED
[TRIGGER] released

[TRIGGER] pressed mode=TRIGGER_FALLBACK profile=P90 behavior=AUTO
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] local kick profile=P90 intensity=220 result=ACCEPTED
[TRIGGER] released
```

Aucun stalling, maintien du solénoïde, brownout ou reboot n'a été rapporté pendant ces essais.

### Point important — moteurs rumble Xbox 360

Les moteurs rumble n'ont pas tourné pendant les essais locaux PISTOL / SNIPER / M16 / P90. Ce comportement n'indique pas à lui seul une panne : les comportements locaux `SINGLE` et `AUTO` actuels demandent uniquement des KICK au `HapticController`; ils ne commandent pas de RUMBLE.

Une validation séparée du chemin rumble GPIO17 / IRLZ44N / moteurs Xbox 360 reste donc obligatoire.

Cette validation doit être faite **avant de calibrer physiquement LASER**, car `CHARGE_RELEASE` utilise justement une rampe de RUMBLE pendant la charge.

### Prochaines priorités

1. Valider puis corriger si nécessaire les moteurs rumble Xbox 360 sur GPIO17.
2. Une fois le rumble physique confirmé, calibrer LASER `CHARGE_RELEASE`.
3. Conserver M16/P90 `autoRepeatMs = 150` comme valeur temporaire jusqu'au tuning de cadence.
