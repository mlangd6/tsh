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

### tsh
Pour lancer le projet : `./tsh`, puis lancer n'importe quelle commande.  
**Attention:** Les redirections, commandes, arguments et pipes doivent être
séparés par des espaces pour bien être *parsé*.

### Test
Pour lancer les tests : `./tsh_test`. Il est aussi possible de lancer qu'une
seule partie des tests en passant un argument à `./tsh_test`. Pour voir la liste
des arguments possible il suffit de lancer `./tsh_test --help`. Plusieurs de ces
arguments peuvent être combinés.

## Documentation
Pour générer la documentation : `make doc`. Il faut ensuite ouvrir le fichier
`doc/html/index.html`
