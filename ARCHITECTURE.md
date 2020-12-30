# Architecture de TSH

## Commandes
Nous avons fait le choix de créer un exécutable pour chaque commande de TSH.

**NB :** Les commandes `cd`, `pwd` et `exit` sont internes à TSH et
sont donc appelées dans TSH par leur fonction.

### Emplacement des commandes
Lors d'un `make` (on peut aussi créer uniquement les commandes avec `make cmd`),
les exécutables sont placés dans `target/bin/` puis copier dans
`/tmp/.tsh/bin` ainsi TSH sait toujours où se trouve les commandes
pour les tarball.


### Appel d'une commande
Lorsqu'une commande TSH est appelée, on doit appeler l'exécutable correspondant
dans `/tmp/.tsh/bin/` même si l'utilisateur n'a donné que des arguments à
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
un *tube* et un processus fils.

### Redirections des sorties

#### Intérieur des tar
Tout d'abord on crée le fichier dans le tar si nécessaire. On enlève le contenu
si nécessaire. On déplace le fichier à la fin de l'archive pour éviter des
décalages lorsque l'on lit en même temps dans le tar. Supposons qu'il existe
*a.tar* ayant *toto* et *titi* (dans cette ordre) comme fichier si on lance
`cat a.tar/titi > a.tar/toto`. Si `cat` lit par block alors l'endroit où
commence *titi* dans le tar aura changer. C'est pour cela que l'on place
toujours le fichier à la fin avant la redirection.  
Nous utilisons un *tube* pour ne pas supprimer le contenue d'un tar par exemple.
On redirige la sortie souhaité vers l'entré du tube. Le processus fils lit dans
la sortie du tube et lance une fonction qui permet d’agrandir le contenu du
fichier souhaité sans corrompre le tar.


#### Extérieur des tar
Cela marche avec une ouverture avec différents `flags` en fonction de la
redirection souhaité (`O_TRUNC` ou `O_APPEND`) puis on lance `dup2`.

### Redirection de l'entrée
On utilise aussi un *tube* car sinon la lecture s'arrêterait uniquement lorsque
tout le tar (et non juste le fichier) sera lu. Ainsi on lit le fichier, ce qui
est lu est écrit dans le *tube* et on redirige l'entrée standard vers la sortie
du tube.   
(Le cas à l'exterieur des tar est une redirection basique).

## Arborescence
`src/` contient 5 dossiers:

1. `main/`: C'est dans ce dossier que ce trouve tous les fichiers `.c`
primordiales au lancement de `tsh` (sauf les commandes). `main/types/` est le dossier
contenant les 3 structures de données (`stack`, `list` et `array`) utilisé dans
`tsh`.

2. `cmd/`: Ce dossier contient le code source des commandes demandé dans le
sujet.

3. `include/`: C'est ici que se trouve tout les fichiers `.h` correspondant au
main, (les commandes n'ont pas de fichiers d'en têtes).

4. `test/`: On y trouve le code source des tests qui sont lancés.

5. `test_include/`: On y trouve les fichiers d'en têtes nécessaires uniquement
aux tests.

## Pipe
Les tubes de commandes sont supportés par `tsh`. L'ordre de priorité entre les
redirections est le même que pour `bash` avec quelque cas en plus qui sont
considéré comme des erreurs pour nous mais pas pour `tsh`. Une redirection de
la sortie standard dans une des commandes brisera donc la chaîne de *pipe*.
