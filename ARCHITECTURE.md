# Architecture de TSH

## Commandes
Nous avons fait le choix de créer un exécutable pour chaque commande de TSH.

**NB :** Les commandes `cd`, `pwd` et `exit` sont internes à TSH et 
sont donc appelées dans TSH par leur fonction.

### Emplacement des commandes
Lors d'un `make` (on peut aussi créer uniquement les commandes avec `make cmd`),
les exécutables sont placés dans `target/bin/` puis copier dans
`$HOME/.tsh/bin/` ainsi TSH sait toujours où se trouve les commandes
pour les tarball. 


### Appel d'une commande
Lorsqu'une commande TSH est appelée, on doit appeler l'exécutable correspondant
dans `$HOME/.tsh/bin/` même si l'utilisateur n'a donné que des arguments à 
l'extérieur des tar.

La commande TSH exécutée appelle alors à son tour le *command_handler*

#### Command handler
On a remarqué que les commandes TSH avaient des structures similaires, on a donc
décidé de créer le *command_handler*. Le *command handler* permet de gérer
les arguments tar, non-tar et les options dans n'importe quel ordre.

Un argument qui n'est pas une option est transformé, comme suit:
1. Transformation en chemin absolu (si ce n'est pas déjà le cas)
2. Simplification du chemin

Une fois les arguments transformés, pour chaque argument
(qui n'est pas une option), le *command handler* s'occupe d'appeler la version
TSH de la commande s'il s'agit d'un fichier dans un tar. La version externe de
la commande sinon 

## Redirections
Dès qu'une redirection fait intervenir des fichiers dans des tar, on passe par
un *pipe*

## Arborescence de `src/`
`src/` contient 5 dossiers:

1. `main/`: \
C'est dans ce dossier que ce trouve tout les fichiers `.c` primordiales au
lancement de `tsh` (sauf les commandes).

2. `cmd/`: \
Ce dossier contient le code source des commandes demandé dans le sujet.

3. `include/`: \
C'est ici que se trouve tout les fichiers `.h` correspondant au main,
(les commandes n'ont pas de fichiers d'en têtes).

4. `test/`: \
On y trouve le code source des tests qui sont lancés.

5. `test_include/`: \
On y trouve les fichiers d'en têtes nécessaires uniquement aux tests.

