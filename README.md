# tsh
Projet de SYSTÈME L3 2020/2021.
`tsh` est un shell pour les tarballs.

## Dépendances
- docker
- doxygen (pour générer la documentation)

## Utilisation docker
Pour créer un conteneur ayant toutes les dépendances nécessaires au projet
lancer:
```
docker build -t tsh .
docker run -it tsh sh
```

Puis lancer dans le conteneur: `cd /home/tsh`

## Compilation
Pour compiler le projet taper : `make`.  
Ou bien `make clean all`.

## Exécution
- Pour lancer le projet : `./tsh` 
- Pour lancer les tests : `./tsh_test`

## Documentation
Pour générer la documentation : `make doc`. Il faut ensuite ouvrir le fichier
`doc/html/index.html`