# Architecture

## Gestion des commandes
Nous avons fait le choix de créer des exécutables pour chaque commandes
qui est un exécutable à l'origine
(ce ne sera donc pas le cas pour `cd` par exemple). Ainsi lors d'un `make`
(on peut aussi créer uniquement les commandes avec `make cmd`). \
Les exécutables sont sauvegarder dans `target/bin/` puis copier dans
`$HOME/.tsh/bin/` ainsi `tsh` sait toujours où se trouve les commandes
pour les tarball.

## Gestion des chemins
Les chemins sont gérés à l'aide d'une variable `twd` (*tar working directory*),
celle ci n'est pour l'instant à aucun moment modifié après son initialisation
(où elle est initialisé au répertoire courant) mais l'être par la suite
même avec des chemins qui sont dans des tarball
(sans modifications par la suite des exécutables comme expliqués dans la partie
suivantes).

### Chemin pour les commandes de tsh

Lorsque une commande implémenté pour les tarball est lancé
(il n'y a pour le moment que `cat` et `ls` avec ou sans l'option `-l`),
on lance l'exécutable correspondant en faisant:
1. Concaténation de `twd` et du chemin passé en paramètre (pour les *n* arguments)
2. Simplification du chemin

  Notons qu'aucune vérification de l'existence du chemin n'est pour l'instant
  faites avant sa simplification. Ainsi `dir/dir_inexistant/../` donnera `dir/`.
  Cela sera modifié pour le rendu final.
3. Puis dans l'exécutable on sépare le chemin en 2 parties,
intérieur et extérieur du tarball

Notons que c'est les exécutables qui gèrent s'ils suffit de lancer l'exécutable
déjà existant (si le chemins ne met pas en jeu de tarball par exemple).
Vous pouvez retrouver les fonctions de gestion des chemins dans [path_lib](src/include/path_lib.h).

## Arborescence de `src/`
`src/` contient 5 dossiers:

1. `main/`: \
C'est dans ce dossier que ce trouve tout les fichiers `.c` primordiales au lancement de `tsh` (sauf les commandes).

2. `cmd/`: \
Ce dossier contient le code source des commandes demandé dans le sujet.

3. `include/`: \
C'est ici que se trouve tout les fichiers `.h` correspondant au main,
(les commandes n'ont pas de fichiers d'en têtes).

4. `test/`: \
On y trouve le code source des tests qui sont lancés.

5. `test_include`: \
On y trouve les fichiers d'en têtes nécessaires uniquement aux tests.

## Structure de donnés utilisés
Pour le moment aucune structure de données n'a été utilisés.
