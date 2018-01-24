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

#define TAILLE 50

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

extern ParamIndex params;

void HttpRequest::GetResponse(SOCKET sd)
{
	std::cout << "Debut requete client" << std::endl; 

	char buffer[TAILLE]; // Buffer de la requête client
	int nb, cpt=0;
	std::string reponseServeur, requeteClient, requete="";

	// Envoi de la page de recherche pour le client
	SetHeader(reponseServeur, (char*) "200", "text/html");
	nb = send(sd, reponseServeur.c_str(), reponseServeur.size(), 0);
	
	// On attend la fin de la requête client et la stock pour l'analyser
	nb = recv(sd, buffer, TAILLE, 0);
	while (nb == TAILLE) {
		nb = recv(sd, buffer, TAILLE, 0);
		requeteClient += buffer;
	} 

	// Analyse de la requête par la machine à état ==> Récupération des éléments du champ de recherche
	requete = RequeteMachineEtats(requeteClient);

	SearchPages(sd, requete);
	
	std::cout << "Fin de requete" << std::endl;


	/*
	
	/* Etudier la requête client pour stocker les paramètres éventuels 
	
	requeteOk = true;
	
	// Préparation de la réponse 

	if(requeteOk) SetHeader(reponseServeur, (char*) "200", "text/html"); // Dans le cas ou il n'y a pas d'erreur
	else SetHeader(reponseServeur, (char*) "404", "text/html"); // Dans le cas ou il y a une erreur de fichier manquant



	
	// Récupérer l'ensemble de la requete. "http:// /moteur?q=val+
	// Récupérer la partie moteur dans 
	// Derrière le ? on a tous les paramètres qui sont de la forme : param = contenu
	// le contenu devant être parsé pour 

	// générer entete ici avec une fonction
	// Faire un code html dans le corps
	*/
	
}

std::string HttpRequest::RequeteMachineEtats(std::string requeteClient)
{
	int state = 0, tmp=0;
	std::string result = "";

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

	return result;
}



void HttpRequest::SetHeader(std::string &AnswerBuf, char *httpCode, const char *mime)
{
	std::stringstream header;
	std::ifstream fichier("./index.htm");
	std::string ligne;

	// GetTime
	auto t=std::time(nullptr);

	// Set HTTP header
	header << "HTTP/1.0 " << httpCode << "\r\n";
	header << "Date: " << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	header << "Server: WebSearchServer/1.0" << "\r\n";
	header << "Content-Type: " << mime << "\r\n";
	header << "\r\n";

	while (std::getline(fichier, ligne)) header << ligne;

	AnswerBuf=header.str();
	
}
