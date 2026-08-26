# Trigger V3 — guide de câblage et validation matérielle

Ce document décrit la méthode recommandée pour passer du prototype historique déjà fonctionnel à la V3 équipée de sa gâchette, de ses boutons, de l'OLED et du buzzer.

La règle principale est simple : **ajouter un sous-système à la fois et revenir systématiquement en SAFE avant le premier test d'un nouveau câblage.**

## 1. Invariants avant de toucher au matériel

Ne pas modifier en même temps le câblage et plusieurs paramètres logiciels. La version de référence est la branche `feature/trigger-v3` avec le code complet sans Wi-Fi.

Les éléments historiques déjà utilisés sur le prototype sont :

| Fonction | GPIO |
|---|---:|
| Rumble | 17 |
| BTS7960 RPWM | 23 |
| BTS7960 LPWM | 5 |
| WS2812 | 16 |

Les nouveaux composants prévus sont :

| Fonction | GPIO | Câblage logique |
|---|---:|---|
| Trigger | 13 | bouton NO entre GPIO13 et GND |
| Profile | 4 | bouton NO entre GPIO4 et GND |
| Mode | 14 | bouton NO entre GPIO14 et GND |
| Buzzer | 27 | selon type, avec driver si courant excessif |
| OLED SDA | 21 | SDA |
| OLED SCL | 22 | SCL |
| OLED | 3.3V/GND | alimentation commune, adresse attendue 0x3C |

Les boutons sont en `INPUT_PULLUP`, donc **aucune résistance pull-up externe n'est nécessaire pour le fonctionnement logique normal**. Le bouton relie l'entrée à GND lorsqu'il est pressé.

## 2. Contrôle visuel avant mise sous tension

Avant chaque nouvelle phase :

- couper l'alimentation du prototype ;
- vérifier qu'aucun fil ne peut toucher une broche voisine ;
- confirmer la masse commune ESP32 / logique / drivers ;
- vérifier le sens VCC/GND de l'OLED ;
- vérifier que le bouton Profile GPIO4 est normalement ouvert ;
- ne pas maintenir GPIO4 à GND pendant reset/boot ;
- vérifier que le buzzer ne demande pas plus de courant que ce qu'un GPIO ESP32 peut fournir ;
- garder l'alimentation puissance du solénoïde coupable rapidement ou déconnectable ;
- ne jamais alimenter directement moteur/solénoïde depuis une GPIO ESP32.

## 3. Phase A — Trigger GPIO13 + Mode GPIO14, SAFE

C'est la première extension recommandée.

### Câblage

```text
GPIO13 ---- bouton Trigger NO ---- GND
GPIO14 ---- bouton Mode NO ------- GND
```

### Firmware

Flasher uniquement :

```powershell
pio run -e trigger-v3-safe -t upload --upload-port COMx
pio device monitor -p COMx -b 115200
```

### Test boot-release guard

1. Boot avec les deux boutons relâchés : aucun événement spontané.
2. Appuyer puis relâcher Trigger : attendre exactement un PRESS puis RELEASE.
3. Éteindre/resetter en maintenant Trigger appuyé : aucun tir ni PRESS valide ne doit être accepté au boot.
4. Relâcher Trigger après boot puis le presser à nouveau : cette nouvelle transition doit être acceptée.

Le comportement attendu est conçu précisément pour empêcher un tir au démarrage.

### HAPTIC_ONLY

Au boot :

```text
[MODE] HAPTIC_ONLY
[FIRE] SINGLE
[PROFILE] PISTOL ...
```

Dans ce mode, une pression Trigger peut être journalisée mais **ne doit pas demander de kick local**.

### TRIGGER_FALLBACK

Faire un appui court sur GPIO14. Attendre :

```text
[MODE] TRIGGER_FALLBACK
```

Puis presser GPIO13. En SAFE, attendre une demande locale suivie d'un blocage SAFE :

```text
[TRIGGER] local kick profile=PISTOL intensity=230 result=INHIBITED
[SAFE] actuator request blocked source=TRIGGER
```

Aucun mouvement physique ne doit se produire.

## 4. Phase B — SINGLE et AUTO, toujours SAFE

### SINGLE

État initial : `SINGLE`.

- une pression Trigger = une demande ;
- maintenir le Trigger = aucune répétition ;
- relâcher puis rappuyer = nouvelle demande.

### AUTO

Maintenir GPIO14 au moins 800 ms pour basculer :

```text
[FIRE] AUTO
```

Dans `TRIGGER_FALLBACK`, maintenir GPIO13 :

- une demande immédiate ;
- puis environ une demande toutes les 150 ms ;
- Bluetooth doit rester réactif ;
- aucune sortie physique en SAFE.

Une cadence réelle irrégulière en COMPAT ne doit pas être corrigée avant d'avoir distingué : cycle mécanique déjà actif, alimentation, mécanique et fréquence réelle des événements.

## 5. Phase C — premier Trigger local en COMPAT

Cette phase ne commence qu'après validation complète de GPIO13/GPIO14 en SAFE.

### Recommandation de démarrage

- revenir en `SINGLE` ;
- profil `PISTOL` ;
- sécuriser mécaniquement le prototype ;
- utiliser une alimentation du solénoïde protégée et facilement interrompable ;
- ne pas commencer par AUTO ou SNIPER.

Flasher :

```powershell
pio run -e trigger-v3-compat -t upload --upload-port COMx
pio device monitor -p COMx -b 115200
```

Passer ensuite par appui court en `TRIGGER_FALLBACK`, puis produire **un seul tir**.

Vérifier :

- impulsion unique ;
- retour à l'arrêt ;
- aucun maintien solénoïde ;
- pas de reset/brownout ;
- pas de chauffe immédiate ;
- Bluetooth reste connecté.

Ensuite seulement tester plusieurs tirs espacés.

## 6. Phase D — bouton Profile GPIO4

### Câblage

```text
GPIO4 ---- bouton Profile NO ---- GND
```

### Précaution GPIO4

GPIO4 est une broche de strapping du classique ESP32. Ne pas la maintenir LOW pendant le boot/reset. Si après montage l'ESP32 :

- ne boot plus correctement ;
- entre dans un comportement de démarrage inhabituel ;
- semble dépendre de l'état du bouton Profile ;

retirer temporairement le bouton de GPIO4 et retester le boot avant toute modification logicielle.

### Validation SAFE recommandée

Un appui doit avancer d'un seul cran :

```text
PISTOL -> SNIPER -> M16 -> P90 -> LASER -> PISTOL
```

Vérifier absence de double changement causé par rebond.

Un changement de profil ne doit jamais déclencher un kick.

## 7. Phase E — OLED SSD1306

### Câblage typique

```text
ESP32 GPIO21 ---- SDA OLED
ESP32 GPIO22 ---- SCL OLED
ESP32 3.3V ------ VCC OLED
ESP32 GND ------- GND OLED
```

Utiliser le niveau d'alimentation réellement supporté par le module. Pour un module SSD1306 annoncé 3.3/5 V, 3.3 V reste le choix naturel avec l'ESP32.

### Détection attendue

Adresse : `0x3C`.

Sans OLED :

```text
[HEALTH] OLED I2C 0x3C NOT DETECTED ...
```

Avec OLED répondant correctement : `DETECTED` puis interface avec BT/mode/profile/fire/state.

### Si OLED instable

Avant de modifier l'architecture :

1. vérifier VCC/GND ;
2. vérifier SDA/SCL inversés ;
3. vérifier adresse réelle du module ;
4. raccourcir les fils ;
5. vérifier la masse commune ;
6. vérifier pull-ups I2C du module ;
7. tester temporairement une fréquence I2C plus basse (ex. 100 kHz) si 400 kHz est marginal.

La priorité reste Bluetooth. Un problème OLED ne doit jamais bloquer le gun.

## 8. Phase F — buzzer GPIO27

Le firmware produit des tonalités `tone()` de 1000, 1200 et 1600 Hz pendant 60 ms.

### Type recommandé

Un petit piezo/buzzer passif est le cas le plus simple pour reproduire réellement les fréquences.

### Ne pas supposer qu'un buzzer est directement compatible GPIO

Si le composant demande un courant significatif, utiliser un transistor/MOSFET et éventuellement une résistance adaptée au composant. Ne pas dépasser les capacités de la GPIO.

### Validation

- aucun son au boot ;
- Profile : bip court ;
- changement Operating mode : fréquence distincte ;
- changement Fire mode : fréquence distincte ;
- son arrêté après environ 60 ms ;
- aucune interruption Bluetooth.

### Point à surveiller : LEDC/PWM

Le rumble utilise LEDC via `ledcAttach/ledcWrite`, tandis que `tone()` utilise lui aussi des ressources de timers/PWM du framework. Tester explicitement :

1. rumble Bluetooth seul ;
2. buzzer seul ;
3. changement de mode pendant rumble ;
4. vérifier que la fréquence/duty rumble reste correcte après un bip ;
5. vérifier qu'aucun canal LEDC n'est réassigné de manière indésirable.

Si une interaction apparaît, diagnostiquer l'allocation LEDC avant de changer les pins ou le protocole.

## 9. Validation du watchdog RUMBLE

L'application Android envoie souvent `RUMBLE 0` rapidement après un événement ; ce scénario ne prouve pas le watchdog.

Pour valider le timeout réel :

1. envoyer une seule commande RUMBLE non nulle ;
2. ne plus envoyer de rafraîchissement RUMBLE ;
3. mesurer l'arrêt ;
4. l'arrêt attendu est environ 500 ms après la dernière commande valide non nulle.

Puis tester une séquence de commandes non nulles rafraîchies : le timeout doit repartir de la dernière commande valide, pas de l'état moteur courant.

## 10. Validation de la protection de chevauchement KICK

L'ancien rate limiter ForceTube de 150 ms a été retiré parce qu'il inhibait des kicks de rafale M16 et P90 légitimes.

La protection restante est l'état mécanique actif :

```cpp
if (!armed_ || kickState_ != KickState::Idle) {
  return false;
}
```

Un nouveau KICK non nul reste donc `INHIBITED` pendant Forward ou Reverse, puis peut être accepté dès le retour à Idle. Un `KICK=0` reçu pendant Forward ou Reverse ne peut pas interrompre le cycle en cours.

Ce comportement a été validé avec la rafale M16 à `KICK 79` et la rafale soutenue P90 à `KICK 56` / `RUMBLE 125`.

## 11. Tuning des profils

Valeurs initiales :

| Profil | Intensité | AUTO |
|---|---:|---:|
| PISTOL | 230 | 150 ms |
| SNIPER | 255 | 150 ms |
| M16 | 240 | 150 ms |
| P90 | 220 | 150 ms |
| LASER | 0 | 150 ms |

Ne pas considérer les noms comme une simulation physiquement calibrée. Ils servent actuellement de points de départ.

Pour tuner :

- changer une seule variable à la fois ;
- commencer par SINGLE ;
- noter intensité, sensation, courant, tension d'alimentation et comportement mécanique ;
- seulement ensuite modifier cadence AUTO ;
- conserver les valeurs dans `TriggerV3Config.h`, pas dispersées dans le code.

## 12. Méthode de diagnostic si un problème apparaît

Toujours établir d'abord quelle couche échoue :

```text
Bouton / câblage
    ↓
Debounce / événement Serial
    ↓
Operating mode / Fire mode
    ↓
Profile
    ↓
HapticController request
    ↓
SAFE gate / active kick state
    ↓
GPIO PWM
    ↓
Driver BTS7960 / driver rumble
    ↓
Alimentation
    ↓
Actionneur / mécanique
```

Exemples :

- aucun `[TRIGGER] pressed` → chercher bouton/pin/debounce avant le solénoïde ;
- `local kick ... INHIBITED` en SAFE → comportement normal ;
- `local kick ... INHIBITED` en COMPAT → regarder l'état armé et si Forward/Reverse est encore actif ;
- `ACCEPTED` mais aucun mouvement → investiguer sortie électrique/driver/alimentation/mécanique ;
- mouvement puis reset → suspecter alimentation/brownout/EMI avant de modifier le parser Bluetooth ;
- Bluetooth se déconnecte seulement pendant kick → suspecter alimentation/bruit/masse avant le protocole.

## 13. Informations à collecter avant de demander de l'aide à une IA

Pour accélérer le diagnostic, fournir :

- SHA Git exact (`git rev-parse HEAD`) ;
- environnement flashé (`safe` ou `compat`) ;
- liste exacte des composants nouvellement branchés ;
- schéma/photo ou description fil par fil ;
- type exact du buzzer/OLED/boutons si connu ;
- sortie Serial depuis le boot jusqu'au problème ;
- action précise qui déclenche le problème ;
- comportement physique observé ;
- alimentation utilisée (tension, courant max, architecture des masses) ;
- résultat en revenant à la configuration précédente ;
- si possible mesures au multimètre/oscilloscope sur les GPIO/rails concernés.

Utiliser ensuite `AI_HANDOFF_PROMPT.md` comme porte d'entrée d'une nouvelle session IA.

## 14. Découverte historique — pourquoi retuner le pulse Reverse du solénoïde

La discussion Discord historique du projet montre que le problème de retour du solénoïde était connu bien avant la version actuelle de Trigger V3.

Chronologie utile :

- **10 juin 2024** : constat qu'un solénoïde peut ne pas être revenu à sa position initiale quand le kick suivant commence ;
- **11 juin 2024** : proposition d'inverser momentanément la polarité pour accélérer le retour ; la discussion précise que cela peut surtout accélérer la décroissance/inversion du champ magnétique, alors que le ressort et la mécanique gardent leur propre dynamique ;
- **août 2024** : sur un solénoïde 65 N comparable, des essais rapportent qu'environ 35 ms de phase principale permettent un meilleur reset entre tirs, mais des stalling restent observés ;
- **26 septembre 2025** : après passage à un BTS7960, ajout d'un **short reverse pulse to center the solenoid** ; l'auteur rapporte que cela a corrigé ses problèmes de stalling.

Cette séquence explique le choix du BTS7960 : contrairement à un simple interrupteur MOSFET unidirectionnel, le pont en H permet de commander brièvement la polarité opposée.

### Ce qui est certain et ce qui ne l'est pas

**Établi par les archives :**

- le pulse inverse avait pour objectif le recentrage / anti-stall entre deux coups ;
- une force reverse de `25 %` apparaît dans le firmware historique récupéré ;
- une phase principale autour de `30–35 ms` a été utilisée/testée sur des prototypes comparables ;
- un reverse court avec BTS7960 a été rapporté comme efficace contre le stalling.

**Non établi par les archives seules :**

- la durée exacte du pulse inverse historique ;
- si le bénéfice principal vient d'une démagnétisation plus rapide, d'un effet mécanique supplémentaire, ou des deux dans l'assemblage réel.

### Solenoid reverse validation

Hardware réellement utilisé :

- solénoïde 1564B / modèle 65 N du prototype ;
- driver BTS7960 ;
- batterie 3S ;
- 1.5KE24CA, TVS bidirectionnelle de la partie solénoïde (pas une Schottky).

Essais physiques rapportés :

- Reverse 1 ms à 25 % : fonctionnel ;
- Reverse 2 ms à 25 % : retour utilisateur très propre ;
- valeur retenue : 2 ms à 25 % ;
- Forward conservé à 30 ms.

```cpp
static constexpr uint8_t kReverseKickPercent = 25;
static constexpr uint32_t kKickDurationMs = 30;
static constexpr uint32_t kReverseDurationMs = 2;
```

Avec 2 ms à 25 %, aucun deuxième gros choc, blocage, brownout, reboot ou problème Bluetooth n'a été rapporté. Aucune température instrumentée n'a été fournie ; ne pas transformer l'absence d'observation de chauffe en mesure thermique.

### ForceTube weapon validation

- **SNIPER** : `KICK 255`, validation single-shot OK ;
- **M16** : `KICK 79`, rafale de trois coups OK après suppression de l'ancien limiteur 150 ms ;
- **P90** : `KICK 56`, `RUMBLE 125`, test de rafale soutenue réussi.

Après la correction de consommation du buffer Bluetooth, les commandes KICK sont restées `ACCEPTED` pendant le test soutenu fourni. Un `KICK=0` reçu pendant Forward ou Reverse ne peut toujours pas interrompre le cycle.

### Recovered rumble wiring understanding

- deux moteurs de vibration Xbox 360 ;
- étage MOSFET IRLZ44N ;
- PWM rumble sur GPIO17 ;
- buck 12 V vers 5 V ;
- masse commune ;
- topologie low-side avec moteurs probablement en parallèle.

Restent à vérifier sur le prototype : valeurs exactes des résistances, référence de diode flyback, tension moteur mesurée, connexion parallèle/série, point ESP32 5V/VIN, câblage IRLZ44N monté et routage physique des masses.

Voir [XBOX_RUMBLE_WIRING.md](XBOX_RUMBLE_WIRING.md). La diode flyback rumble y est distinguée explicitement de la TVS bidirectionnelle 1.5KE24CA du solénoïde.
