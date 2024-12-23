#!/bin/bash
# ---------------------------------[Exécute les makes]-------------------------
#cd img-dist/
#make

#cd ..
#make


# ---------------------------------------[Main]------------------------------------

if [ $# -eq 0 ]     # Vérifie que l'utilisateur a donné des arguments lors de l'appel du launcher
  then
    echo "You need to give the option (-a or -i) and the argument(s) that you want. For example : './launcher -i image_file'. "
    exit 1
fi


if [ $1 = -a ] || [ $1 = -automatic ]
then
    if [ $# -lt 2 ]     # Vérifie que l'utilisateur a donné assez d'arguments pour le bon fonctionnement du programme par rapport à l'option choisit
    then
        echo "You need to give the path of an image."
        exit 1
    elif [ $# = 2 ]
    then 
        ./list-file ./img/ | .img-search $2  #cas par defauts
    else 
        ./list-file $3 | .img-search $2
    
    fi


elif [ $1 = -i ] || [ $1 = -interctive ]
then
    if [ $# -lt 2 ]     # Vérifie que l'utilisateur a donné assez d'arguments pour le bon fonctionnement du programme par rapport à l'option choisit
    then
        echo "You need to give the path of an image"
        exit 1
    elif [ $# -gt 1 ]    #si il y a plus de 1 parametre
    then 
        echo "give all the images to constitute the image bank in the following way: img1.bmp \nimg2.bmp \nimg3.bmp"
        read -r bank       #on prends en stdin(de ce code) les fichiers pour la banque d'image
        if [ $# = 2 ]     #si il y a 2 paramètres (avec -i) pas besoin de mettre de prefixe aux noms des fichiers
        then
            echo -e $bank | .img-search $2 
        else           #si il y a un troisieme parametre il faut ajouter celui comme prefixe aux noms des fichiers
            mod_bank=''
            IFS=$"\n"
            for elem in $bank     #rajoute le préfixe 
            do
                if [ -n "$elem" ]   #verifie que elem n'est pas vide, il l'est 1 fois sur 2
                then
                    mod_bank="$mod_bank$3/$elem \n"
                fi
            done
            echo -e "$mod_bank" | .img-search $2  
        fi   
    fi


else
    echo "Error input, option not reconized"
fi