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

	conn=mysql_init(conn);
	if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
	{
		
		// Noms des tables
		std::vector<std::string> tables={"word_page","page","word"};

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
		basefile = params.BaseFiles.append("files/").c_str();

		std::map<int, std::string> filesnames;
		// Tous les fichiers sont intitulés de 0 a 38
		for (int i = 0; i <= 38; i++) {
			// Ajouter les numéros de fichier avec le lien de base vers les fichiers
			std::string filename = basefile + static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str() + ".txt";
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
			
		




		// C:\Users\Game\Source\Repos\Projet_RI_web\Projet_RI_web\Projet_RI_web\data\files/0.txt

		// Pagerank seuil = 1/10 000
		
		// Calcul du pageRank uniquement a partir de la matrice dans link.txt

		// Rajout des mots des pages et des PageRank

//TODO: à vous de jouer ...

		mysql_close(conn);
	}
	else std::cerr << mysql_error(conn) << std::endl;
}
