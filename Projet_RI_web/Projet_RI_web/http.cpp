/* Mini projet - Application de la recherche d'information au web */
/* Auteurs : - Aur�lien Gauvrit */
/*			 - Jimmy Dor� */
/* Groupe : ID5 */

/* HTTP.cpp */

/* Fichier de gestion de la requ�te client � interpr�ter */

/* Commentaires d'utilisation : 

  - Concernant l'indexation, le nombre de fichier � �t� fix� � 39 car le dossier de donn�es contenait 39 fichiers. 
	Ce dernier est modifiable � travers la variable 'nbFichiers' du fichier 'INDEX.cpp'

  - Il faut rajouter l'option "i" lors du lancement du programme si on veut r�indexer tous les fichiers, sinon on n'indique pas cet argument

  - V�rifier le chemin d'acc�s aux donn�es � travers le fichier de config : 
    Important pour �viter tout probl�me de lecture, il y a deux contraintes sur ce chemin :
	      Le chemin devant �tre de la forme : X:\\.../.../.../data/
	      Le chemin doit �tre pass� en format absolu

  - Concernant la requ�te client, un bool�en � �t� d�fini. Plus de pr�cisions sont disponibles ci dessous concernant la variable 'QUERY_AND'
  
  */


// Importation des librairies
#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include "http.h"
#include "index.h"
#include "params.h"
#include "mysql.h"


// Modifier la constante "QUERY_AND" au d�but du fichier pour avoir une requ�te qui effectue entre les diff�rents termes de la requ�te : 
// - une UNION (OR --> QUERY_AND false) 
// - une INTERSECTION (AND --> QUERY_AND true) 
// Exemple : Si on recherche "people technologies" : 
// - avec l'union, on va renvoyer tous les textes qui contiennent people 'OU' technologies
// - avec l'intersection, on va renvoyer tous les textes qui contiennent � la fois people 'ET' technologies
#define QUERY_AND true
#define TAILLE 50


#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

extern ParamIndex params;


void HttpRequest::GetResponse(SOCKET sd)
{
	char buffer[TAILLE]; // Buffer de la requ�te client
	int nb, cpt=0;
	std::string reponseServeur, requeteClient, resultRequete;

	std::cout << "Debut requete client" << std::endl;

	// R�cup�ration de la requ�te client
	do {
		nb = recv(sd, buffer, TAILLE, 0);
		requeteClient += buffer;
	} while (nb == TAILLE);

	// Analyse de la requ�te par la machine � �tat ==> R�cup�ration des �l�ments du champ de recherche
	std::string requete = RequeteMachineEtats(requeteClient);

	// R�cup�ration du r�sultat de la requ�te
	resultRequete = SearchPages(sd, requete);

	
	SetHeader(reponseServeur, (char*) "200", "text/html"); // Mise en place du header
	reponseServeur += resultRequete;	// Ajout du r�sultat de la requ�te � la r�ponse
	nb = send(sd, reponseServeur.c_str(), reponseServeur.size(), 0);	// Envoi de la requ�te
	
	std::cout << "Fin de requete" << std::endl;

}

// Methode de r�cup�ration de l'�l�ment requ�t� 
std::string HttpRequest::RequeteMachineEtats(std::string requeteClient)
{

	unsigned int state = 0, tmp=0;
	std::string result = "";

	// Si la requ�te n'est pas vide
	if (requeteClient.compare("") != 0) {

		// Parsage de la requ�te
		while (state != 4 && (tmp < requeteClient.size() - 1)) {

			switch (state) {
			case 0:
				if (requeteClient[tmp] == '/') {
					state = 1;
				} break;
			case 1:
				if (requeteClient[tmp] == '?') {
					state = 2;
				}
				else if (requeteClient[tmp] == ' ') {
					state = 4;
				}
				break;
			case 2:
				if (requeteClient[tmp] == '=') {
					state = 3;
				}
				break;
			case 3:
				if (requeteClient[tmp] != ' ') {
					result += requeteClient[tmp];
				}
				else state = 4;
				break;
			}
			tmp++;
		}
	}
	return result;
}


// M�thode de cr�ation du header pour la r�ponse
void HttpRequest::SetHeader(std::string &AnswerBuf, char *httpCode, const char *mime)
{
	std::stringstream header;
	std::ifstream fichier("./index.htm");	// Fichier html (champs de recherche et bouton d'envoi)
	std::string ligne;

	// GetTime
	auto t=std::time(nullptr);

	// Set HTTP header
	header << "HTTP/1.0 " << httpCode << "\r\n";
	header << "Date: " << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	header << "Server: WebSearchServer/1.0" << "\r\n";
	header << "Content-Type: " << mime << "\r\n";
	header << "\r\n";

	while (std::getline(fichier, ligne)) header << ligne;	// Ajout du fichier html

	AnswerBuf=header.str();
	
}

// Methode de recherche de pages suivant des termes renseign�s
std::string HttpRequest::SearchPages(SOCKET sd, std::string requete)
{
	std::string to_print = "";

	// Echange des '+' par des ' ' dans les mots recherch�s
	std::replace(requete.begin(), requete.end(), '+', ' ');

	// Stockage des mots dans un vector
	std::vector<std::string> mots;

	// D�coupage de la requete
	std::stringstream ss(requete);
	std::string mot;
	while (ss >> mot) mots.push_back(mot);	// Ajout de tous les mots dans un vecteur

	// Si la requ�te n'est pas vide
	if (mots.size() >= 1) {

		//Connecteurs � la base SQL 
		MYSQL *conn = nullptr;
		MYSQL_RES *res_set;
		MYSQL_ROW row;
		//D�claration des variables
		int nbResults = 0;
		int i = 0;

		//On initialise la connexion � la base
		conn = mysql_init(conn);
		if (mysql_real_connect(conn, params.ServerName.c_str(), params.Login.c_str(), params.Password.c_str(), params.SchemeName.c_str(), 0, NULL, 0))
		{

			//On s�lectionne l'url de la page s�lectionn�e, ainsi que son r�sum�
			std::string query = "SELECT p.url, p.resume, p.id_page ";

			// Clauses From and Where, Jointures par rapport aux id_page et id_word 
			query += "FROM `page` p, `word` w, `word_page` wp ";
			query += "WHERE p.id_page = wp.id_page AND wp.id_word = w.id_word AND(";

			//On s�lectionne les diff�rents mots de la requ�te 
			for (unsigned int i = 0; i < mots.size(); i++) {
				query += " BINARY w.word = BINARY '";
				query += mots.at(i);
				query += "'";
				if (i < mots.size() - 1) {
					query += " OR ";
				}
			}
			query += ") ";

			//Clauses Group By 
			query += "GROUP BY p.id_page";

			// Modifier la constante au d�but du fichier pour avoir une requ�te qui effectue entre les diff�rents termes de la requ�te : 
			// - une UNION (OR --> QUERY_AND false) 
			// - une INTERSECTION (AND --> QUERY_AND true) 
			if (QUERY_AND) {
				query += " HAVING COUNT(*) = ";
				query += std::to_string(mots.size());
			}

			//On trie les r�sultats par le PageRank
			query += " ORDER BY p.pr DESC";

	
			//On parcourt les r�sultats de la requ�te
			if (!mysql_query(conn, query.c_str()))
			{
				std::string to_print2 = "";
				if (res_set = mysql_store_result(conn))
				{
					//Tant qu'il reste des lignes dans le r�sultat
					while (row = mysql_fetch_row(res_set)) {

						//Mise en forme de l'affichage des diff�rents r�sultats
						to_print2 += "<h4 style=\"color:blue; margin-bottom:2px\" >";
						to_print2 += row[0];
						to_print2 += "</h4><p style=\"color:grey; margin-top:2px\">";
						to_print2 += row[1];
						to_print2 += "</p>";
						nbResults++;
					}

					mysql_free_result(res_set);
					std::ostringstream oss;
					oss << nbResults;
					to_print += "<h2> Nombre de r�sultats : " + oss.str();
					to_print += "</h2>" + to_print2;
				}
				else printf("%s\n", mysql_error(conn));
			}
			else printf("%s\n", mysql_error(conn));
			mysql_close(conn);
		}
		std::cerr << mysql_error(conn) << std::endl;
	}

	//On renvoie les r�sultats de la requ�te
	return to_print;
}