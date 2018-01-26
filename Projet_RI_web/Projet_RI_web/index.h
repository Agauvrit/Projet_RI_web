/* Projet de recherche d'informations (Application de la recherche d'information au web) */
/* Auteurs : - Aurélien Gauvrit */
/*			 - Jimmy Doré */
/* Groupe : ID5 */

/* INDEX.h */
/* Fichier d'indexation des données dans la base */

/* Les informations d'utilisations sont disponibles en haut du fichier HTTP.cpp */

#pragma once

// Importation des librairies
#include<map>

void IndexData();
std::map<int, float> computePageRank(std::string linksFile);
std::string getSelectWordID(std::string mot);