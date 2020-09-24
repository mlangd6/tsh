# tsh

Projet de SYSTÈME L3 2020/2021

## Sujet : `tsh`, un shell pour les tarballs

Le but du projet est de faire tourner un shell qui permet à
l'utilisateur de traiter les tarballs comme s'il s'agissait de
répertoires, **sans que les tarballs ne soient désarchivés**. Le
format précis d'un tarball est décrit sur
[https://fr.wikipedia.org/wiki/Tar_(informatique)](https://fr.wikipedia.org/wiki/Tar_%28informatique%29).

Le shell demandé doit avoir les fonctionnalités suivantes :

* les commandes `cd` et `exit` doivent exister (avec leur comportement habituel)
* toutes les commandes externes doivent fonctionner normalement si leur déroulement n'implique pas l'utilisation d'un fichier (au sens large) dans un tarball
* `pwd` doit fonctionner y compris si le répertoire courant passe dans un tarball
* `mkdir`, `rmdir` et `mv` doivent fonctionner y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option
* `cp` et `rm` doivent fonctionner y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option ou avec l'option `-r`
* `ls` doit fonctionner y compris avec des chemins impliquant des tarball quand il est utilisé sans option ou avec l'option `-l`
* `cat` doit fonctionner y compris avec des chemins impliquant des tarball quand il est utilisé sans option
* les redirections de l'entrée, de la sortie et de la sortie erreur (y
  compris sur des fichiers d'un tarball) doivent fonctionner
* les combinaisons de commandes avec `|` doivent fonctionner

On ne vous demande pas de gérer les cas de tarballs imbriqués (un tarball dans un autre tarball).

Tous les processus lancés à partir de votre shell le seront en premier-plan.

## Modalités d'exécution (et de test)

Les projets seront testés sur la distribution [https://mirrors.ircam.fr/pub/mx/isos/ANTIX/Final/antiX-19/antiX-19.2.1_386-base.iso](https://mirrors.ircam.fr/pub/mx/isos/ANTIX/Final/antiX-19/antiX-19.2.1_386-base.iso). À vous de faire en sorte que ça fonctionne (nous n'accepterons pas d'argument de type "ça fonctionne chez moi").

La présence de tests dans le projet sera grandement appréciée.

## Modalités de rendu

Le projet est à faire par équipes de 2 ou 3 étudiants. Aucune exception ne sera
tolérée et nous avons une nette préférence pour les équipes de 3 (les équipes de 2 ne pourront pas utiliser le fait qu'elles ont moins de membres pour demander une quelconque indulgence car c'est elles qui auront fait ce choix-là). La composition de
chaque équipe devra être envoyée par mail à l'enseignante responsable
du cours de systèmes **au plus tard le 1er octobre 2020**, avec copie à
chaque membre de l'équipe.


En plus du programme demandé, vous devez fournir un `Makefile` utilisable, un
mode d'emploi, et un fichier `ARCHITECTURE` (idéalement en format Markdown,
donc avec extension `.md`) expliquant la stratégie adoptée pour répondre au
sujet (notamment l'architecture logicielle, les structures de
données et les algorithmes implémentés).

Les seules interdictions strictes sont les suivantes : plagiat (d'un
autre projet ou d'une source extérieure à la licence), utilisation de
la fonction `system` de la `stdlib` et de la commande `tar` (vous
pouvez utiliser la commande `tar` pour créer des archives afin de
tester le projet, mais pas dans le code du projet).

Pour rappel, l'emprunt de code sans citer sa source est un
plagiat. L'emprunt de code en citant sa source est autorisé, mais bien
sûr vous n'êtes notés que sur ce que vous avez effectivement produit.
Donc si vous copiez l'intégralité de votre projet en le spécifiant
clairement, vous aurez quand même 0 (mais vous éviterez une demande de
convocation de la section disciplinaire).


### Premier rendu
Le premier rendu est au choix de chaque groupe. Il est là pour vous
aider dans l'avancée de votre projet et vous éviter de passer à côté du
sujet. Si vous fournissez des documents explicatifs sur les structures
de données et les algorithmes que vous utiliserez, nous vous donnerons
notre avis dessus. Si vous fournissez du code en documentant ce qu'il
est censé faire, nous le testerons. Si vous n'avez pas bien compris le
sujet, nous vous expliquerons en quoi.

Nous nous attendons à un volume de travail représentant au moins le
tiers du travail final. À noter que le sujet porte à la fois sur le
SGF et les processus, mais qu'au moment du premier rendu nous n'aurons
pas traité en cours la partie processus, il semble donc plus logique
que votre travail porte de préférence sur la partie SGF. Ce travail
peut être constitué de fonctions qui seront utiles au projet, sans
pour autant prendre la forme d'un shell dès le départ ; par exemple un
ensemble de fonctions qui permettent de faire toutes les manipulations
qui seront nécessaires dans un tarball est un travail tout à fait
acceptable pour ce premier rendu.


Récupéré depuis https://gaufre.informatique.univ-paris-diderot.fr/klimann/systL3_2020-2021/blob/master/Projet/README.md