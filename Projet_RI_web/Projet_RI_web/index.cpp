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

void IndexData()
{
	MYSQL *conn=nullptr;

	MYSQL_RES *res_page_id;
	MYSQL_ROW row_page_id;
	MYSQL_RES *res_word_id;
	MYSQL_ROW row_word_id;
	
	conn=mysql_init(conn);
	if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
	{
		// Noms des tables
		std::vector<std::string> tables={"word_page","page","word"};
		std::string requete;

		std::string lastID = "SELECT LAST_INSERT_ID() AS id";
		std::string pageId;
		std::string wordId;

		// Nombre de fichiers � lire
		unsigned int nbFichiers = 39;

		// Noms des fichiers � traiter
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
			std::string sql="TRUNCATE TABLE `";
			sql+=table;
			sql+="`";
			if (!mysql_query(conn, sql.c_str())) std::cout << "Empty table '" << table << "'" << std::endl;
			else std::cerr << mysql_error(conn) << std::endl;
		}
	
		// On forme les noms de fichiers
		std::string linksFile = params.BaseFiles.c_str();
		std::string pagesFiles = params.BaseFiles.c_str();
		linksFile += "links.txt";
		pagesFiles += "files/";
		
		// Calcul du PageRank � partir du fichier de liens
		pageRanks = computePageRank(linksFile);

		
		// Tous les fichiers jusqu'au nombre renseign�
		for (unsigned int i = 0; i < nbFichiers; i++) {

			// Reset du r�sum� et de la requete
			resume = "";
			requete = "";

			// R�cup�ration de l'URL du fichier
			nomFichier = pagesFiles + static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str() + ".txt";

			// V�rification de l'ouverture du fichier
			file.open(nomFichier);
			if (!file.is_open()){
				std::cout << "Erreur lors de l'ouverture d'un fichier texte de donnees" << std::endl;
				exit(EXIT_FAILURE);
			}

			// R�cup�ration du r�sum� (Les 2 premi�res lignes)
			for (unsigned int k = 0; k < 2; k++) {
				std::getline(file, ligneTmp);
				resume += ligneTmp;
			}
			file.close();
			resume += " ...";

			// Cr�ation de la requ�te pour ajout dans la base des pages
			requete = "INSERT INTO `page` (url, pr, resume) VALUES ('" + nomFichier + "', ";
			requete += std::to_string(pageRanks[i]) + ", '" + resume + "');";
			
			if (!mysql_query(conn, requete.c_str())) {
				std::cout << "Ajout dans la table page de la page " << i << " done" << std::endl;

				// R�cup�ration de l'identifiant de la page dans la base
				if (!mysql_query(conn, lastID.c_str())) {
					if (res_page_id = mysql_store_result(conn)) {
						row_page_id = mysql_fetch_row(res_page_id); // R�cup�ration du r�sultat de la requ�te
						pageId = row_page_id[0]; // Identifiant de la derni�re page ins�r�e
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
					//while (ligneTmp >> mots) {
						if (i == 0)
							std::cout << mots << std::endl;

						// Nettoyage des mots
						for (unsigned int i = 0; i < mots.size(); i++) {
							int valChar = (int)mots[i];
							if (mots[i] == 39) {
								mots.insert(i + 1, "'");
								mots[i] = '\\';
								i++;
							}
							else if (!((valChar >= 48 && valChar <= 57) || (valChar >= 65 && valChar <= 90) ||
								(valChar >= 97 && valChar <= 122) || (valChar >= 128 && valChar <= 167) ||
								(valChar >= -32 && valChar <= -10 ) ||
								(valChar >= -7 && valChar <= -4 ) ||
								(valChar >= -64 && valChar <= -35) )) {
								mots[i] = ' ';
							}
						}

						std::stringstream iss(mots); // R�cup�ration mot par mot
						std::string mot;
						while (iss >> mot) {

							std::transform(mot.begin(), mot.end(), mot.begin(), ::tolower); // On ajoute les mots en minuscule
							if (mot.size() > 1) { // On n'ajoute pas les mots trop petits
								
								requete = "INSERT IGNORE INTO word (word) SELECT * FROM (SELECT BINARY '" + mot + "') as word";
								requete += " WHERE NOT EXISTS (SELECT word FROM word WHERE BINARY word = BINARY '" + mot + "') LIMIT 1;";

								// Ajout du mot dans la base
								if (!mysql_query(conn, requete.c_str())) 
									std::cout << "Ajout dans la table mot du mot " << mot << " realise avec succes" << std::endl;
								else {
									printf("%s\n", mysql_error(conn));
									exit(EXIT_FAILURE);
								}

								if (!mysql_query(conn, getSelectWordID(mot).c_str())) {
									if (res_word_id = mysql_store_result(conn)) {
										row_word_id = mysql_fetch_row(res_word_id); // R�cup�ration du r�sultat de la requ�te
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

								// Ajout de la relation mot et page
								requete = "INSERT IGNORE INTO word_page(id_word, id_page) SELECT * FROM (SELECT " + wordId;
								requete += ") as id1, (SELECT " + pageId + ") as id2 WHERE ";
								requete += "NOT EXISTS (SELECT id_word FROM word_page WHERE id_word = " + wordId + " AND id_page = " + pageId + ") AND ";
								requete += "NOT EXISTS (SELECT id_page FROM word_page WHERE id_word = " + wordId + " AND id_page = " + pageId + ") ";
								requete += "LIMIT 1;";

								// Ajout de la relation (mot - page) dans la base
								if (!mysql_query(conn, requete.c_str())) 
									std::cout << "Ajout dans la table mot_page du mot " << mot  << " et de la page : " << pageId << std::endl;
								else {
									printf("%s\n", mysql_error(conn));
									exit(EXIT_FAILURE);
								}
							}
						}
					//}
				}
			}
			else {
				printf("%s\n", mysql_error(conn));
				exit(EXIT_FAILURE);
			}
			file.close();
		}	
		mysql_close(conn);
	}
	else std::cerr << mysql_error(conn) << std::endl;
	
}

std::map<int, float> computePageRank(std::string linksFile)
{
	// Stockage du PageRank (Cl� ==> Num�ro de fichier, Valeur ==> Valeur du pageRank pour ce fichier)
	std::map<int, float> pageRanks;
	std::map<int, float> pageRanksTmp;

	std::vector<std::vector<int>> MatriceRelations; // Stockage de la matrice des relations
	std::map<int, int> Ci; // Repr�sente les liens sortants vers les autres documents

	int nbFichiers = 0; // Nombre de fichiers correspondant � la matrice

	// Variables propres � l'algorithme PageRank
	bool converge = false; // la convergence est-elle atteinte.
	float d = 0.85f; // Valeur permettant de converger plus ou moins rapidement
	float seuilConvergence = 0.00001f;

	// Fichier � lire
	std::ifstream fichier(linksFile);

	// Variables temporaires de traitement du fichier
	std::string ligne;
	std::string mot; // Mot temporaire (Va contenir le nombre correspondant au lien ou non vers le prochain fichier)
	
	// Variables temporaires de comptage
	int tmp = 0;
	int cpt = 0;
	int nbOk = 0;
	
	

	
	// Parcours du fichiers
	while (std::getline(fichier, ligne)) {

		// V�rification si on est � la premi�re ligne du fichier pour r�cup�rer le nombre de fichiers.
		if (nbFichiers > 0) {
			std::stringstream ss(ligne); // R�cup�ration de la ligne courante

			for (int i = 0; i < nbFichiers; i++) { // On sait qu'il y a nbFichiers valeurs derri�res

				ss >> mot;  // Parsage de la ligne
				if (!mot.compare("1")) { // Si on trouve un "1"

					MatriceRelations.at(i).push_back(tmp); // Ajout du num�ro de fichier 
					Ci[tmp] = Ci[tmp]+1; // On ajoute un lien sortant pour ce fichier
				}
			}
			tmp++;
		}
		else { // Premi�re ligne

			nbFichiers = stoi(ligne); // R�cup�ration du nombre de fichiers
			for (int i = 0; i < nbFichiers; i++) { // Initialisation des �l�ments

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

			pageRanksTmp[j] = pageRanks[j]; // Stockage du PR pr�c�dent
			pageRanks[j] = (1 - d) + (d * I);
		}

		// V�rification de la convergence
		for (int l = 0; l < nbFichiers; l++) {

			if (abs(pageRanks[l] - pageRanksTmp[l]) <= seuilConvergence)
				nbOk++;
		}

		if (nbOk == nbFichiers) // Si tous les fichiers respecent le seuil de convergence alors on arr�te
			converge = true;

		nbOk = 0;	
	}

	std::cout << "Convergence apres : " << cpt << " iterations" << std::endl;

	return pageRanks;

}

std::string getSelectWordID(std::string mot) {
	return "SELECT id_word FROM `word` WHERE BINARY word = BINARY '" + mot + "'";
}


void SearchPages(SOCKET sd, std::string requete)
{
	// Echange des '+' par des ' ' dans les mots recherch�s
	std::replace(requete.begin(), requete.end(), '+', ' ');

	// Stockage des mots dans un vector
	std::vector<std::string> mots;

	// D�coupage de la requete
	std::stringstream ss(requete);
	std::string mot;
	while (ss >> mot) mots.push_back(mot);

	// Si la requ�te n'est pas vide
	if (mots.size() >= 1) {

		//Bool�en nous permettant de d�terminer si on veut une requ�te avec des AND ou des OR --> Laisser � true pour des and, et mettre � false pour des OR
		bool and_query = true;

		//Connecteurs � la base SQL 
		MYSQL *conn = nullptr;
		MYSQL_RES *res_set;
		MYSQL_ROW row;
		int nbResults = 0;
		int nb, i = 0;

		conn = mysql_init(conn);
		if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
		{
			std::vector<std::string> tables = { "word_page","page","word" };

			//FORMATION DE LA REQUETE DE SELECTION DES PAGES INTERESSANTES
			//On s�lectionne l'url de la page s�lectionn�e, ainsi que son r�sum�
			std::string query = "SELECT p.url, p.resume ";
			// Clauses From and Where, Jointures par rapport aux id_page et id_word 
			query += "FROM `page` p, `word` w, `word_page` wp ";
			query += "WHERE p.id_page = wp.id_page AND wp.id_word = w.id_word AND(";

			for (unsigned int i = 0; i < mots.size(); i++) {
				query += " BINARY w.word = BINARY '";
				query += mots.at(i);
				query += "'";
				//Pour lier les diff�rents mots � trouver, avec des OR
				if (i < mots.size() - 1) {
					query += " OR ";
				}
			}
			query += ") ";
			//Clauses Group By 
			query += "GROUP BY p.id_page";
			//Si on veut les AND ou des OR, en fonction de la valeur du bool�en indiqu� pr�c�demment
			if (and_query) {
				query += " HAVING COUNT(*) = ";
				query += std::to_string(mots.size());
			}

			//Clause Order By PageRank, on favorise les meilleurs pages ranks
			query += " ORDER BY p.pr";

			std::cout << query << std::endl;

			//On parcourt les r�sultats de la requ�te
			if (!mysql_query(conn, query.c_str()))
			{
				std::string to_print2 = "";
				if (res_set = mysql_store_result(conn))
				{	//Tant qu'il reste des lignes dans le r�sultat
					while (row = mysql_fetch_row(res_set)) {
						//Mise en forme de l'affichage des diff�rents r�sultats
						to_print2 += "<h4 style=\"color:blue; margin-bottom:2px\" >";
						to_print2 += row[0];
						to_print2 += "</h4><p style=\"color:grey; margin-top:2px\">";
						to_print2 += row[1];
						to_print2 += " ...";
						to_print2 += "</p>";
						nbResults++;
					}
					mysql_free_result(res_set);
					std::string to_print = "<h2> Nombre de r�sultats : " + nbResults; 
					to_print += "</h2>" + to_print2;
					nb = send(sd, to_print.c_str(), to_print.size(), 0);
				}
				else printf("%s\n", mysql_error(conn));
			}
			else printf("%s\n", mysql_error(conn));
			mysql_close(conn);
		}
		std::cerr << mysql_error(conn) << std::endl;
	}
}