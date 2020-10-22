# tsh
Projet de SYSTÈME L3 2020/2021.
`tsh` est un shell pour les tarballs.

## Dépendances
- docker

## Utilisation docker
Pour créer un conteneur ayant toutes les dépendances nécessaire au projet lancer:

```
docker build -t tsh .
docker run -it tsh sh
```

## Compilation
Pour compiler le projet taper : `make`
## Exécution
Pour lancer le projet : `./tsh` \
Pour lancer les tests : `./tsh_test`
