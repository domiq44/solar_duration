# Architecture

Le programme suit un flux en quatre étapes : lecture de la configuration,
conversion des chaînes, validation métier, puis simulation jour par jour.

`main.c` orchestre ces étapes. Les calculs sont répartis entre des modules C
avec interfaces dans `include/`.

## Flux d'exécution

1. `config_reader.c` lit `config/solar_duration.dat` et remplit un `RawConfig`
   de chaînes allouées. Les clés requises comprennent les composantes de début
   et de fin, leur année respective, la latitude et le mode solaire.
2. `config_validator.c` convertit les entiers, la latitude et le seuil solaire
   optionnel (-0,833° par défaut), sélectionne la fonction de déclinaison, puis
   vérifie les bornes et les deux dates.
3. La date de début doit être antérieure ou égale à la date de fin. La
   comparaison inclut l'année; une période peut traverser plusieurs années.
   Avec `--check`, le programme s'arrête après cette validation.
4. `simulation.c` traite chaque date validée, de façon inclusive, avance avec
   `advance_day()` et transmet l'année et le jour ordinal au modèle solaire.
   Toute date invalide rencontrée pendant le parcours fait échouer la
   simulation; les dates validées produisent nécessairement un ordinal de 1 à
   366.
5. `logger.c` écrit les diagnostics et les résultats sur la sortie standard ou
   d'erreur selon leur niveau. Si l'option `--csv` est utilisée, la simulation
   écrit également chaque résultat dans un CSV indépendant des journaux.

Les chaînes brutes sont libérées après conversion avec `free_raw_config()`.

`FinalConfig` contient des valeurs numériques et un pointeur vers la fonction
de déclinaison choisie. Il ne possède pas de mémoire dynamique.

## Responsabilités des modules

- `main.c` : initialise le logger et orchestre lecture, validation et
   simulation.
- `config_reader.c` : lit les paires clé-valeur et gère `RawConfig`.
- `config_validator.c` : convertit et valide les champs, puis choisit le
   modèle solaire.
- `date.c` : valide les dates, calcule leur ordinal et avance d'un jour.
- `geo.c` : parse et formate la latitude en degrés, minutes et secondes.
- `solar.c` : calcule la déclinaison (sinusoïdal, Spencer, Meeus) et la durée
   du jour.
- `simulation.c` : parcourt la période et journalise le résultat quotidien.
- `logger.c` : filtre et formate les messages selon le niveau configuré.
- `string_utils.c` : fournit les utilitaires de chaîne du parsing.

## Configuration validée

Les dates sont représentées séparément :

```c
int jour_debut;
int mois_debut;
int annee_debut;
int jour_fin;
int mois_fin;
int annee_fin;
```

Les deux années sont bornées par `MIN_YEAR` et `MAX_YEAR` dans
`include/config.h`. Les dates doivent exister dans le calendrier grégorien.
La latitude doit rester dans [-90°, 90°].

Le seuil d'altitude solaire, compris entre -90° et 90°, détermine quand le
Soleil est considéré au-dessus de l'horizon.

Le mode associe `FinalConfig` à une des trois fonctions déclarées dans
`include/solar.h`.

## Build et tests

Le `Makefile` compile l'application dans `build/solar_duration`, les huit
exécutables unitaires et le runner d'intégration dans `build/tests/`.

```sh
make
make test
```

`make test` exécute les suites unitaires et lance le programme principal dans
un répertoire temporaire.

`make test_unit` et `make test_integration` permettent de lancer chaque niveau
séparément. Les détails figurent dans [TESTING.md](TESTING.md).
