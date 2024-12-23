#!/bin/bash

# Vérifier si le chemin du dossier est spécifié en tant que paramètre
if [ $# -ne 1 ]; then
    echo "Missing directory name." >&2
    exit 1
fi

# Vérifier si le chemin spécifié est un dossier
if [ ! -d "$1" ]; then
    echo "The specified path is not a directory." >&2
    exit 2
fi

# Liste des fichiers dans le dossier (ignore les sous-dossiers) et les trie
files=$(find "$1" -maxdepth 1 -type f -exec basename {} \;)

# Vérifier si la liste est vide
if [ -z "$files" ]; then
    echo "No files found in the directory."
else
    echo "$files"
fi