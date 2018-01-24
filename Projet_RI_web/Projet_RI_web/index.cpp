#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <fstream>
#include "mysql.h"
#include "params.h"
#include "index.h"

extern ParamIndex params;

void IndexData()
{
	MYSQL *conn=nullptr;
	const char* basefile;
	std::string linksFile;
	
	conn=mysql_init(conn);
	if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
	{
		// Noms des tables
		std::vector<std::string> tables={"word_page","page","word"};

		// Noms des fichiers à traiter
		std::map<int, std::string> filesnames;

		// Stockage du PageRank
		std::map<int, int> pageRanks;

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
		basefile = params.BaseFiles.c_str();
		linksFile = params.BaseFiles.append("links.txt").c_str();
		//pagesFiles = params.BaseFiles.append("files/").c_str();
		
		// Calcul du PageRank à partir du fichier de liens
		pageRanks = computePageRank(linksFile);

		
		
		
		
		/*
		// Tous les fichiers sont intitulés de 0 a 38
		for (int i = 0; i <= 38; i++) {
			// Ajouter les numéros de fichier avec le lien de base vers les fichiers
			std::string filename = pagesFiles + static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str() + ".txt";
			filesnames.insert(std::pair<int, std::string>(i, filename));
		}

		

		for (std::pair<int,std::string> i_filename : filesnames) {
			// Ajout dans la table page
			std::string sql = "INSERT INTO " + tables.at(1) + " (url) VALUES(\"" + i_filename.second + "\")"; // Récupération url
			
			// Ajout dans la table 'PAGE'
			if (!mysql_query(conn, sql.c_str())) std::cout << "INSERT INTO PAGE '" << i_filename.second << "'" << std::endl;
			else std::cerr << mysql_error(conn) << std::endl;

			std::ifstream file;
			std::string tmp = "C:\\Users/Game/Source/Repos/Projet_RI_web/Projet_RI_web/Projet_RI_web/data/files/0.txt";
			file.open(i_filename.second);
			if (!file.is_open()) {std::cout << i_filename.second << std::endl;
				std::cout << "erreur d'ouverture de fichiers" << std::endl;
				exit(EXIT_FAILURE);
			}
			
			std::string word;
			while (file >> word)
			{
				if(i_filename.first == 0)
					std::cout << word << std::endl;
			}
		
		}
		*/

			
			
			mysql_close(conn);
			}
			else std::cerr << mysql_error(conn) << std::endl;
		




		// C:\Users\Game\Source\Repos\Projet_RI_web\Projet_RI_web\Projet_RI_web\data\files/0.txt

		// Pagerank seuil = 1/10 000
		
		// Calcul du pageRank uniquement a partir de la matrice dans link.txt

		// Rajout des mots des pages et des PageRank

//TODO: à vous de jouer ...

		//mysql_close(conn);
		
	//}
	//else std::cerr << mysql_error(conn) << std::endl;

	
}

std::map<int, int> computePageRank(std::string linksFile)
{
	// Stockage du PageRank
	std::map<int, float> pageRanks;
	std::map<int, int> Ranking;

	std::ifstream fichier(linksFile);
	std::string ligne;
	
	int nbFichiers = 0;
	int tmp = 0;
	//bool converge = false;
	int t = 5;
	float d = 0.85f;
	std::vector<std::vector<int>> MatriceRelations;
	std::vector<int> Ci;
	

	/*
	int cpt = 0;
	int t = 5;
	float d = 0.85f;
	std::vector<std::vector<int>> Relations;
	std::vector<int> relationLine;
	*/


	while (std::getline(fichier, ligne)) {
		if (nbFichiers > 0) {
			std::stringstream ss(ligne);
			std::string mot;
			for (int i = 0; i < nbFichiers; i++) {
				ss >> mot;
				if (!mot.compare("1")) {
					MatriceRelations.at(i).push_back(tmp);
					Ci.at(tmp)++;
				}
			}
			tmp++;
		}
		else {
			nbFichiers = stoi(ligne);
			for (int i = 0; i < nbFichiers; i++) {
				pageRanks[i] = 0.0f;
				Ci.push_back(0);
				Ranking[i] = 0;
				//PRi.push_back(0);
				MatriceRelations.push_back(std::vector<int>());
			}
		}
	}

	// A CHANGER
	// while (!converge)
	for (int i = 0; i < t; i++) {
		for (int j = 0; j < nbFichiers; j++) {
			float I = 0.0f;
			for (unsigned int k = 0; k < MatriceRelations.at(j).size(); k++) {
				int numFile = MatriceRelations.at(j).at(k);
				I += pageRanks[i] / Ci.at(numFile);
			}
			pageRanks[j] = (1 - d) + (d * I);
		}
	}

	int rank = 1;

	for (int i = 0; i < nbFichiers; i++) {
		int indexMax = 0;
		for (int i = 0; i < nbFichiers; i++) {
			if (pageRanks[indexMax] < pageRanks[i]) indexMax = i;
		}
		Ranking.at(indexMax) = rank;
		pageRanks[indexMax] = -1;
		rank++;
	}

	return Ranking;

}


void SearchPages(SOCKET sd, std::string requete)
{
	//Si il y a au moins un mot dans la requête
	if (requete.size() >= 1) {
		// Echange des '+' par des ' ' dans les mots recherchés
		std::replace(requete.begin(), requete.end(), '+', ' ');

		// Stockage des mots dans un vector
		std::vector<std::string> mots;

		// Découpage de la requete
		std::stringstream ss(requete);
		std::string mot;
		while (ss >> mot) mots.push_back(mot);

		//Booléen nous permettant de déterminer si on veut une requête avec des AND ou des OR --> Laisser à true pour des and, et mettre à false pour des OR
		bool and_query = true;

		//Connecteurs à la base SQL 
		MYSQL *conn = nullptr;
		MYSQL_RES *res_set;
		MYSQL_ROW row;
		int nb, i = 0;

		conn = mysql_init(conn);
		if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
		{
			std::vector<std::string> tables = { "word_page","page","word" };

		//FORMATION DE LA REQUETE DE SELECTION DES PAGES INTERESSANTES
			//On sélectionne l'url de la page sélectionnée, ainsi que son résumé
			std::string query = "SELECT Page.url, Page.resume";
			// Clauses From and Where, Jointures par rapport aux id_page et id_word 
			query += "FROM `page` Page, `word` Word, `word_page` WordPage ";
			query += "WHERE Page.id_page = WordPage.id_page AND WordPage.id_word = Word.id_word AND(";

			for (unsigned int i = 0; i < mots.size(); i++) {
				query += "Word.word = '";
				query += mots.at(i);
				query += "'";
				//Pour lier les différents mots à trouver, avec des OR
				if (i < mots.size() - 1) {
					query += " OR ";
				}
			}
			query += ")";
			//Clauses Group By 
			query += "GROUP BY Page.id_page";
			//Si on veut les AND ou des OR, en fonction de la valeur du booléen indiqué précédemment
			if (and_query) {
				query += " HAVING COUNT(*) = ";
				query += std::to_string(mots.size());
			}

			//Clause Order By PageRank, on favorise les meilleurs pages ranks
			query += " ORDER BY Page.PageRank";

			std::cout << query << std::endl;
			
			//On parcourt les résultats de la requête
			if (!mysql_query(conn, query.c_str()))
			{
				if (res_set = mysql_store_result(conn))
				{	//Tant qu'il reste des lignes dans le résultat
					while (row = mysql_fetch_row(res_set)) {
						//Mise en forme de l'affichage des différents résultats
						std::string to_print = "<h4 style=\"color:blue; margin-bottom:2px\" >";
						to_print += row[0];
						to_print += "</h4><p style=\"color:grey; margin-top:2px\">";
						to_print += row[1];
						to_print += "</p>";
						nb = send(sd, to_print.c_str(), to_print.size(), 0);
					}
					mysql_free_result(res_set);
				}
				else printf("%s\n", mysql_error(conn));
			}
			else printf("%s\n", mysql_error(conn));
			mysql_close(conn);
		}
		std::cerr << mysql_error(conn) << std::endl;
	}
}


}