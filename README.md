# Simulateur de durée du jour

> **Démo web** : [ouvrir le calculateur](https://domiq44.github.io/solar_duration/)
> après le premier déploiement GitHub Pages. Pour le lancer localement ou
> comprendre ses réglages, voir [Page web statique](#page-web-statique).

Petit programme en C qui estime la durée d'ensoleillement pour chaque jour
d'une période et d'une latitude données. La période peut traverser une ou
plusieurs années. Trois modèles de déclinaison solaire sont disponibles.

## Démarrage

Prérequis : GCC, GNU Make et un environnement compatible POSIX.1-2008
(Linux, macOS ou équivalent). Windows natif n'est pas pris en charge; sous
Windows, exécuter le projet dans WSL.

Les tests d'intégration utilisent notamment `fork`, `mkdtemp` et les API de
fichiers POSIX.

```sh
make run
```

Cette commande compile puis lance `build/solar_duration`. Le programme lit
`config/solar_duration.dat` et le niveau de journalisation dans
`config/solar_duration.cfg`.

Pour enregistrer la sortie dans `output/solar_duration.log`, lancer
`make run_log`.

Si la configuration du logger est absente ou invalide, l'application signale
l'erreur et continue avec le niveau `INFO`.

Les chemins peuvent être remplacés avec `--config`, `--logger-config` et
`--csv`. Par exemple :

```sh
build/solar_duration --config ma_config.dat --logger-config mon_logger.cfg \
  --csv resultats.csv
```

Ajouter `--check` valide la configuration puis quitte sans lancer la simulation
ni créer de fichier CSV :

```sh
build/solar_duration --check --config ma_config.dat
```

## Page web statique

Ouvrir [`web/index.html`](web/index.html) dans un navigateur. La page fonctionne
sans compilation, serveur ni dépendance externe. Elle calcule la durée du jour
pour une date unique, avec Meeus sélectionné par défaut.

La recherche propose les villes intégrées à la page et renseigne leurs
coordonnées et leur fuseau; les coordonnées restent modifiables. Le bouton
Position peut utiliser la géolocalisation du navigateur après autorisation.
La durée du jour dépend de la latitude; les heures civiles estimées de lever et
de coucher tiennent aussi compte de la longitude, de l'équation du temps et du
fuseau choisi, notamment des règles CET/CEST selon la date.

Le seuil solaire est modifiable et peut être rétabli à `-0.833°`. La date et
les autres réglages sont conservés dans le stockage local du navigateur; le
bouton « Réinitialiser les réglages » restaure les valeurs initiales. Rien
n'est envoyé à un serveur. Selon le navigateur, le stockage local peut être
restreint pour les pages ouvertes directement en `file://`; dans ce cas, le
calcul reste disponible, mais les réglages ne sont pas mémorisés.

Pour publier une démo, le workflow `.github/workflows/pages.yml` déploie le
dossier `web/` sur GitHub Pages à chaque push sur `main` et peut aussi être
lancé manuellement. Dans les paramètres du dépôt, choisir **Settings > Pages >
Build and deployment > Source: GitHub Actions**. Après le premier déploiement,
GitHub affiche l'URL publique dans l'environnement `github-pages` de l'onglet
Actions.

Les villes utilisent des coordonnées représentatives de leur centre. Les
horaires sont des estimations géométriques et ne tiennent pas compte du relief,
de l'horizon ni des conditions atmosphériques locales.

## Configuration

Chaque date possède son propre jour, mois et année. Exemple de période qui
traverse le Nouvel An :

```ini
jour_debut = 31
mois_debut = 12
annee_debut = 2025
jour_fin = 2
mois_fin = 1
annee_fin = 2026
latitude = 48 deg 51 min 24 sec Nord
mode_solaire = 2
altitude_solaire_seuil = -0.833
```

Les clés sont insensibles à la casse. Les espaces, les lignes vides et les
commentaires `#` ou `//` sont acceptés.

Les années doivent être comprises entre 1900 et 3000. La latitude doit rester
entre -90° et 90°, et `mode_solaire` vaut 1, 2 ou 3.

La latitude s'écrit en degrés, minutes et secondes, suivis de `Nord` ou `Sud`.
`altitude_solaire_seuil` est facultatif, compris entre -90° et 90°, et vaut
`-0.833°` par défaut.

La durée calculée correspond au temps pendant lequel le centre du Soleil dépasse
ce seuil. Cette approximation combine le rayon apparent du Soleil et une
réfraction atmosphérique moyenne; elle ne modélise pas précisément l'atmosphère
locale.

## Modèles

| Mode | Modèle |
| --- | --- |
| `1` | Sinusoïdal, approximation simple |
| `2` | Spencer |
| `3` | Meeus |

Le calcul combine la déclinaison solaire du modèle choisi, la latitude et le
seuil d'altitude pour estimer la durée du jour. Les résultats quotidiens sont
émis au niveau `RESULT` et restent visibles quel que soit le niveau de log;
les messages de suivi, eux, sont filtrés selon `config/solar_duration.cfg`.

## Commandes utiles

- `make` : compiler l'application.
- `make run` : compiler et lancer la simulation.
- `make run_log` : lancer la simulation et enregistrer la sortie.
- `make plot_duration` : comparer les modèles sur la période configurée.
- `make plot_latitudes` : générer `output/solar_latitudes.png`.
- `make plot_duration_heatmap` : visualiser la durée par année et jour.
- `make plot_all` : générer tous les graphiques.
- `make test` : exécuter les tests unitaires et d'intégration.
- `make test_unit` : exécuter uniquement les tests unitaires.
- `make test_integration` : tester avec des configurations isolées.
- `make generate_solar_data` : générer `output/solar_model_comparison.csv`.
- `make plot_solar_models` : comparer les modèles sur la dernière année.
- `make help` : afficher les autres cibles disponibles.

Les configurations résident dans `config/`. Les journaux, CSV et graphiques
générés vont dans `output/`, ignoré par Git et nettoyé par `make clean`.

Le graphique multi-latitudes utilise Spencer pour la dernière année disponible,
de l'équateur aux régions polaires du nord.

Le graphique de durée superpose les trois modèles pour la latitude et la
période configurées; le modèle actif est mis en évidence. Le comparatif indique
les écarts absolus maximaux en minutes. La carte thermique adapte son échelle
de couleurs aux valeurs présentes.

## Précision des modèles

Les dates acceptées couvrent les années 1900 à 3000. Cette plage ne garantit pas
une précision astronomique uniforme sur toute la période.

Les tests comparent les trois modèles à 20 déclinaisons apparentes géocentriques
de NASA/JPL Horizons (éphéméride DE441). Ils couvrent quatre dates saisonnières
pour chacune des années 1900, 2000, 2100, 2500 et 3000.

Les erreurs maximales observées sur cet échantillon sont :

- Modèle sinusoïdal : 2°
- Spencer : 0,7°
- Meeus : 0,05°

Un point de contrôle supplémentaire couvre le 13 octobre 1992 à midi.

Ces maxima décrivent uniquement les 20 points saisonniers et le contrôle
supplémentaire de 1992; ils ne constituent pas des bornes garanties pour les
autres dates ni une certification de toute la plage 1900–3000. Pour un usage
scientifique, validez les résultats sur toute la période utile avec des
éphémérides de référence.

Les tests unitaires sont décrits dans [docs/TESTING.md](docs/TESTING.md).

L'architecture et les modules sont présentés dans
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

Pour les fichiers générés, voir [l'inventaire](docs/FILES_AND_OUTPUTS.md).
