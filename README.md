# PokemonScan
 Identification d'Images
  ![image](https://github.com/user-attachments/assets/549c166a-85b5-41b0-81f0-77d85c037b4b)

## Description
   Ce projet a pour objectif de développer un système capable d'identifier des Pokémon à partir d'images. Il utilise un code de hachage perceptif pour comparer les images et déterminer les correspondances. Le système est conçu pour être robuste face aux images partiellement dégradées ou présentant des variations de couleurs. Il utilise également plusieurs processus pour traiter un grand nombre d'images de manière concurrente.

## Structure du Projet
Le projet est organisé en plusieurs composants :

- `img-dist` : Un programme en C qui génère le code de hachage perceptif pour les images.
- `list-file` : Un script Bash qui liste les fichiers d'un dossier.
- `img-search` : Un programme en C qui recherche les images les plus similaires à une image donnée en utilisant deux processus enfants pour effectuer la recherche de manière concurrente.
- `launcher` : Un script Bash qui lance le programme img-search avec différentes options (mode automatique et interactif).

## Arborescence du Projet
```bash
img/
img-dist/
    bmp-endian.h
    bmp.c
    bmp.h
    main-img-dist.c
    Makefile
    pHash.c
    pHash.h
    verbose.c
    verbose.h
img-search.c
launcher.sh
list-file.sh
Makefile
PDF/
    Projet_OS.pdf
test/
    img/
    range-name-except
    test-bdd-images.data
    test-new-images.data
    tests
```

## Compilation
Pour compiler les programmes, exécutez les commandes suivantes :
```bash
cd img-dist/
make

cd ..
make
```

## Utilisation
### Mode Automatique
Pour lancer le programme en mode automatique, utilisez l'option `-a` suivie du chemin de l'image à comparer :
```bash
./launcher.sh -a chemin_image
```
### Mode Interactif
Pour lancer le programme en mode interactif, utilisez l'option `-i` suivie du chemin de l'image à comparer :
```bash
./launcher.sh -i chemin_image
```
Ensuite, entrez les chemins des images de la banque d'images, une par ligne.

## Auteurs
- paug0002
- jche0027
- rrab0007
