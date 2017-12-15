#include <string>
#include <iostream>
#include <vector>
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
		std::vector<std::string> tables={"word_page","page","word"};

		for (auto &table : tables)
		{
			std::string sql="TRUNCATE TABLE `";
			sql+=table;
			sql+="`";
			if (!mysql_query(conn, sql.c_str())) std::cout << "Empty table '" << table << "'" << std::endl;
			else std::cerr << mysql_error(conn) << std::endl;
		}
	
		basefile = params.BaseFiles.append("files/").c_str();

		std::vector<std::string> filesnames;
		for (int i = 0; i <= 39; i++) {
			// Ajouter les num�ros de fichier avec le lien de base vers les fichiers
			filesnames.push_back();
		}


		// Calcul du pageRank uniquement a partir de la matrice dans link.txt

		// Rajout des mots des pages et des PageRank

//TODO: � vous de jouer ...

		mysql_close(conn);
	}
	else std::cerr << mysql_error(conn) << std::endl;
}
