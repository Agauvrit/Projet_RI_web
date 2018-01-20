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

}