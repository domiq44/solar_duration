# Tests et vérifications

## Tests automatisés

Exécuter les huit suites unitaires et les tests d'intégration avec :

```sh
make test
```

Les suites unitaires couvrent la lecture et la validation de configuration, les
dates, la latitude, le logger, la simulation, les modèles solaires et les
chaînes. Les tests solaires vérifient aussi les durées à ±90° et à ±89,999°,
notamment les cas d'équinoxe et de jour/nuit polaires. Les suites compilent dans
`build/tests/` et ne modifient pas
`config/solar_duration.dat`.

Le test de référence compare les trois modèles à 20 déclinaisons apparentes
géocentriques de NASA/JPL Horizons (éphéméride DE441) : quatre dates
saisonnières pour chacune des années 1900, 2000, 2100, 2500 et 3000.

Les erreurs maximales observées sur ces points sont de 2° pour le modèle
sinusoïdal, 0,7° pour Spencer et 0,05° pour Meeus. Elles ne sont pas des bornes
garanties pour les autres dates et ne certifient pas chaque jour de la période.

`make test_unit` exécute uniquement les TU. `make test_integration` lance le
binaire principal dans des répertoires temporaires avec ses propres fichiers de
configuration.

Les scénarios d'intégration couvrent une période traversant le Nouvel An,
l'utilisation des chemins par défaut, les options de fichier, `--help` et le
mode `--check` avec des configurations valides ou invalides. Ils vérifient aussi
le rejet d'une option inconnue et d'une date impossible, ainsi que la visibilité
des résultats quotidiens lorsque le niveau de journalisation est `WARN`.

Les tests du lecteur vérifient le rejet des lignes trop longues. `make test`
exécute les suites unitaires et d'intégration.

`make generate_solar_data` lance un outil distinct qui produit
`output/solar_model_comparison.csv` pour comparer les trois modèles. Le CSV
contient également les durées calculées par les trois modèles à la latitude
définie dans `config/solar_duration.dat`.

Ce générateur produit des données; il ne remplace pas les tests avec assertions.

Les vérifications complémentaires sont `make lint` et `make cppcheck`.

Pour construire sans lancer l'application, utiliser `make`. Pour exécuter le
flux principal, `make run` lit les fichiers de configuration dans `config/`.

## Graphes (optionnel)

`make plot_duration` lance la simulation avec un export CSV dédié puis crée
`output/solar_duration.png`, qui compare les trois modèles et indique la
latitude, le modèle configuré, le seuil solaire et les dates exportées.

Le CSV contient les durées des trois modèles; le graphique met le modèle
configuré en évidence. Vérifier les dates et durées du CSV, puis ouvrir l'image.
Cette cible requiert Python 3 et Matplotlib.

`make plot_solar_models` régénère `output/solar_model_comparison.csv`, conserve
les logs dans `output/solar_data_generator.log` et produit
`output/solar_model_comparison.png` avec trois panneaux : déclinaisons, écarts
de déclinaison à Spencer et écarts de durée en minutes à la latitude configurée.
Le graphique se limite à la dernière année présente dans le CSV et indique les
écarts absolus maximaux de durée dans les légendes.

Cette cible est une visualisation, pas une validation par assertions. Le tracé
requiert Python 3 et Matplotlib.

`make plot_duration_heatmap` produit `output/solar_duration_heatmap.png`, avec
une ligne par année et une colonne par jour ordinal; l'échelle de couleurs est
ajustée aux durées présentes.

`make plot_all` exécute toutes les cibles de graphiques. Les données
comparatives sont générées une seule fois.

## Configuration de test

Les dates de début et de fin comportent chacune jour, mois et année. Par
exemple, cette période inclut les trois jours du 31 décembre au 2 janvier :

```ini
jour_debut = 31
mois_debut = 12
annee_debut = 2025
jour_fin = 2
mois_fin = 1
annee_fin = 2026
latitude = 47 deg 17 min 48 sec Nord
mode_solaire = 2
```

Les années sont limitées à 1900–3000. Les dates doivent exister et la date de
début doit précéder ou égaler la date de fin.

La latitude doit être comprise entre -90° et 90°. Elle s'exprime en degrés,
minutes et secondes, avec `Nord` ou `Sud`. `altitude_solaire_seuil` est
facultatif (-0,833° par défaut) et limité à [-90°, 90°].

Les clés ne sont pas sensibles à la casse. Les espaces, les lignes vides et
les commentaires `#` ou `//` sont acceptés.

## Ajouter un test

Les suites sont dans `tests/test_<module>.c`. Un cas retourne un `TestResult`,
est déclaré dans le tableau `test_cases`, puis exécuté par `run_test_suite()`.
Suivre le schéma Given / When / Then défini dans
[AGENTS.md](../tests/AGENTS.md).
Créer un état neuf par cas; pour tester le lecteur, utiliser les utilitaires de
fichiers temporaires plutôt que modifier `config/solar_duration.dat`.

Les tests de `tests/test_simulation.c` vérifient notamment les bornes
inclusives et le passage d'une année à la suivante. Les tests unitaires
n'exécutent pas `main.c`; le lancement de `make run` vérifie le câblage avec les
fichiers de configuration actifs.

## Nettoyage et sorties

Les exécutables sont placés dans `build/`. `make clean` supprime ce répertoire,
les journaux et les sorties de graphiques/CSV générées par les cibles Make.
Avant le nettoyage, vérifier les fichiers produits à conserver.
