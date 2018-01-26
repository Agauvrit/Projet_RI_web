/* Projet de recherche d'informations (Application de la recherche d'information au web) */
/* Auteurs : - Aurélien Gauvrit */
/*			 - Jimmy Doré */
/* Groupe : ID5 */

/* HTTP.h */
/* Fichier de gestion de la requête client à interpréter */

/* Les informations d'utilisations sont disponibles en haut du fichier HTTP.cpp */

#pragma once

// Importation des librairies
#include "socket.h"
#include <string>

class HttpRequest
{
public:
	HttpRequest() {}
	~HttpRequest() {}
	void GetResponse(SOCKET sd);

private:
	void SetHeader(std::string &AnswerBuf,char *httpCode, const char *mime="text/html");
	std::string RequeteMachineEtats(std::string requeteClient);
	std::string SearchPages(SOCKET sd, std::string requete);
};
