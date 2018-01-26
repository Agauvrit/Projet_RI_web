/* Mini projet - Application de la recherche d'information au web */
/* Auteurs : - Aurélien Gauvrit */
/*			 - Jimmy Doré */
/* Groupe : ID5 */

/* INDEX.cpp */
/* Fichier d'indexation et de requêtage sur la base de données */

/* Les informations d'utilisations sont disponibles en haut du fichier HTTP.cpp */

// Importation des librairies
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <functional>
#include <set>
#include <fstream>
#include <cmath>

#include "mysql.h"
#include "params.h"
#include "index.h"


extern ParamIndex params;

// Methode d'indexation des données
void IndexData()
{
	MYSQL *conn = nullptr;

	// Stockage des résultats de requête MySql    
	MYSQL_RES *res_page_id;
	MYSQL_ROW row_page_id;
	MYSQL_RES *res_word_id;
	MYSQL_ROW row_word_id;

	conn = mysql_init(conn);
	if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
	{
		// Noms des tables
		std::vector<std::string> tables = { "word_page","page","word" };
		std::string requete;

		std::string lastID = "SELECT LAST_INSERT_ID() AS id"; // Requête de récupération du dernier identifiant enregistré
		std::string pageId;
		std::string wordId;

		// Nombre de fichiers à lire
		unsigned int nbFichiers = 39;

		// Variables temporaires de fichiers à traiter
		std::string nomFichier;
		std::string resume;
		std::ifstream file;
		std::string ligneTmp;
		std::string mots;
		std::string mot;

		// Stockage du PageRank
		std::map<int, float> pageRanks;


		// On vide les tables
		for (auto &table : tables)
		{
			std::string sql = "TRUNCATE TABLE `";
			sql += table;
			sql += "`";
			if (!mysql_query(conn, sql.c_str())) std::cout << "Empty table '" << table << "'" << std::endl;
			else std::cerr << mysql_error(conn) << std::endl;
		}


		// On forme les noms de fichiers
		std::string linksFile = params.BaseFiles.c_str();
		std::string pagesFiles = params.BaseFiles.c_str();
		linksFile += "links.txt";
		pagesFiles += "files/";


		// Calcul du PageRank à partir du fichier de liens
		pageRanks = computePageRank(linksFile);


		// Tous les fichiers jusqu'au nombre renseigné
		for (unsigned int i = 0; i < nbFichiers; i++) {

			// Reset du résumé et de la requete
			resume = "";
			requete = "";

			// Récupération de l'URL du fichier
			nomFichier = pagesFiles + static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str() + ".txt";

			// Vérification de l'ouverture du fichier
			file.open(nomFichier);
			if (!file.is_open()) {
				std::cout << "Erreur lors de l'ouverture d'un fichier texte de donnees" << std::endl;
				exit(EXIT_FAILURE);
			}


			// Récupération du résumé (Les 2 premières lignes)
			for (unsigned int k = 0; k < 2; k++) {
				std::getline(file, ligneTmp);
				if (k >= 1) resume += ligneTmp;
			}
			file.close();
			resume += " ...";


			// Requete d'insertion d'une page dans la base
			requete = "INSERT INTO `page` (url, pr, resume) VALUES ('" + nomFichier + "', ";
			requete += std::to_string(pageRanks[i]) + ", '" + resume + "');";


			// Insertion dans la table Page
			if (!mysql_query(conn, requete.c_str())) {
				std::cout << "Ajout de la page " << i << " la base de donnees " << std::endl;


				// Récupération de l'identifiant de la page dans la base
				if (!mysql_query(conn, lastID.c_str())) {
					if (res_page_id = mysql_store_result(conn)) {
						row_page_id = mysql_fetch_row(res_page_id); // Récupération du résultat de la requête
						pageId = row_page_id[0]; // Identifiant de la dernière page insérée
					}
					else {
						printf("%s\n", mysql_error(conn));
						exit(EXIT_FAILURE);
					} mysql_free_result(res_page_id);
				}
				else {
					printf("%s\n", mysql_error(conn));
					exit(EXIT_FAILURE);
				}


				// Parcours des mots
				file.open(nomFichier);
				while (std::getline(file, mots)) {


					// Nettoyage des mots
					for (unsigned int i = 0; i < mots.size(); i++) {
						int valChar = (int)mots[i];
						if (mots[i] == 39) {
							mots.insert(i + 1, "'");
							mots[i] = '\\';	// Echapement des caractère d'apostrophe
							i++;
						}
						else if (!((valChar >= 48 && valChar <= 57) || (valChar >= 65 && valChar <= 90) ||
							(valChar >= 97 && valChar <= 122) || (valChar >= 128 && valChar <= 167) ||
							(valChar >= -32 && valChar <= -10) ||
							(valChar >= -7 && valChar <= -4) ||
							(valChar >= -64 && valChar <= -35))) {
							mots[i] = ' ';	// Suppression des caractères inutiles
						}
					}


					std::stringstream iss(mots); // Récupération mot par mot
					std::string mot;

					while (iss >> mot) {


						std::transform(mot.begin(), mot.end(), mot.begin(), ::tolower); // On ajoute les mots en minuscule
						if (mot.size() > 1) { // On n'ajoute pas les mots trop petits


							// Requete d'insertion d'un mot dans la table mot
							requete = "INSERT IGNORE INTO word (word) SELECT * FROM (SELECT BINARY '" + mot + "') as word";
							requete += " WHERE NOT EXISTS (SELECT word FROM word WHERE BINARY word = BINARY '" + mot + "') LIMIT 1;";

							// Ajout du mot dans la base
							if (!mysql_query(conn, requete.c_str()))
								requete = "";
							else {
								printf("%s\n", mysql_error(conn));
								exit(EXIT_FAILURE);
							}

							// Récupération de l'identifiant dans la base du mot ajouté
							if (!mysql_query(conn, getSelectWordID(mot).c_str())) {
								if (res_word_id = mysql_store_result(conn)) {
									row_word_id = mysql_fetch_row(res_word_id); 
									wordId = row_word_id[0]; // Identifiant du mot dans la base
								}
								else {
									printf("%s\n", mysql_error(conn));
									exit(EXIT_FAILURE);
								} mysql_free_result(res_word_id);
							}
							else {
								printf("%s\n", mysql_error(conn));
								exit(EXIT_FAILURE);
							}


							// Requete d'insertion de la relation entre le mot et la page
							requete = "INSERT IGNORE INTO word_page(id_word, id_page) SELECT * FROM (SELECT " + wordId;
							requete += ") as id1, (SELECT " + pageId + ") as id2 WHERE ";
							requete += "NOT EXISTS (SELECT id_word FROM word_page WHERE id_word = " + wordId + " AND id_page = " + pageId + ") AND ";
							requete += "NOT EXISTS (SELECT id_page FROM word_page WHERE id_word = " + wordId + " AND id_page = " + pageId + ") ";
							requete += "LIMIT 1;";


							// Ajout de la relation (mot - page) dans la base
							if (!mysql_query(conn, requete.c_str()))
								requete = "";
							else {
								printf("%s\n", mysql_error(conn));
								exit(EXIT_FAILURE);
							}
						}
					}
					//}
				}
				std::cout << "Fin d'insertion dans la base de la page " << i << std::endl;
			}
			else {
				printf("%s\n", mysql_error(conn));
				exit(EXIT_FAILURE);
			}
			file.close(); // Fermeture du fichier
		}
		mysql_close(conn);
		std::cout << "Indexation terminee " << std::endl;
	}
	else std::cerr << mysql_error(conn) << std::endl;
}

// Methode de calcul du PageRank pour chaque fichier
std::map<int, float> computePageRank(std::string linksFile)
{
	// Stockage du PageRank (Clé ==> Numéro de fichier, Valeur ==> Valeur du pageRank pour ce fichier)
	std::map<int, float> pageRanks;
	std::map<int, float> pageRanksTmp;

	std::vector<std::vector<int>> MatriceRelations; // Stockage de la matrice des relations
	std::map<int, int> Ci; // Représente les liens sortants vers les autres documents

	int nbFichiers = 0; // Nombre de fichiers correspondant à la matrice


	bool converge = false; // la convergence est-elle atteinte.
	float d = 0.85f; // Valeur permettant de converger plus ou moins rapidement
	float seuilConvergence = 0.00001f;


	// Fichier à lire
	std::ifstream fichier(linksFile);


	// Variables temporaires de traitement du fichier
	std::string ligne;
	std::string mot; // Mot temporaire (Va contenir le nombre correspondant au lien ou non vers le prochain fichier)

	// Variables temporaires de comptage
	int tmp = 0;
	int cpt = 0;
	int nbFilesConverge = 0;

	// Parcours du fichiers
	while (std::getline(fichier, ligne)) {


		// Vérification si on est à la première ligne du fichier pour récupérer le nombre de fichiers.
		if (nbFichiers > 0) {
			std::stringstream ss(ligne); // Récupération de la ligne courante

			for (int i = 0; i < nbFichiers; i++) { // On sait qu'il y a nbFichiers valeurs derrières

				ss >> mot;  // Parsage de la ligne
				if (!mot.compare("1")) { // Si on trouve un "1"

					MatriceRelations.at(i).push_back(tmp); // Ajout du numéro de fichier 
					Ci[tmp] = Ci[tmp] + 1; // On ajoute un lien sortant pour ce fichier
				}
			}
			tmp++;
		}
		else { // Première ligne


			nbFichiers = stoi(ligne); // Récupération du nombre de fichiers
			for (int i = 0; i < nbFichiers; i++) { // Initialisation des éléments

				pageRanks[i] = 0.0f;
				pageRanksTmp[i] = 0.0f;
				Ci[i] = 0;
				MatriceRelations.push_back(std::vector<int>());
			}
		}
	}

	while (!converge) { // Tant que l'on ne converge pas
		cpt++;

		for (int j = 0; j < nbFichiers; j++) { // Calcul du PR

			float I = 0.0f;
			for (unsigned int k = 0; k < MatriceRelations.at(j).size(); k++) {

				int numFile = MatriceRelations.at(j).at(k);
				I += pageRanks[numFile] / Ci.at(numFile); // Calcul du nouveau PR
			}

			pageRanksTmp[j] = pageRanks[j]; // Stockage du PR précédent
			pageRanks[j] = (1 - d) + (d * I); // Ajout du nouveau PR calculé
		}

		// Vérification de la convergence
		for (int l = 0; l < nbFichiers; l++) {

			if (abs(pageRanks[l] - pageRanksTmp[l]) <= seuilConvergence)
				nbFilesConverge++;
		}

		if (nbFilesConverge == nbFichiers) // Si tous les fichiers respecent le seuil de convergence alors on arrête
			converge = true;

		nbFilesConverge = 0;
	}

	std::cout << "Convergence apres : " << cpt << " iterations" << std::endl;

	return pageRanks;

}

// Methode de récupération de l'identifiant d'un mot dans la base
std::string getSelectWordID(std::string mot) {
	return "SELECT id_word FROM `word` WHERE BINARY word = BINARY '" + mot + "'";
}

