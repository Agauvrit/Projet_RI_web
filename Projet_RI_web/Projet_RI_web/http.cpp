#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "http.h"
#include "params.h"

#define TAILLE 50

#ifdef _MSC_VER
	#pragma warning(disable:4996)
#endif

extern ParamIndex params;

void HttpRequest::GetResponse(SOCKET sd)
{
	std::cout << "Début requête client" << std::endl;

	char buffer[TAILLE], bufferReponse[TAILLE]; // Buffer de la requête client et réponse serveur
	int nb, cpt=0;
	std::string reponseServeur, requeteClient;

	SetHeader(reponseServeur, (char*) "200", "text/html");
	
	/* Envoi de la réponse contenant seulement la barre de recherche au client*/
	// Parcours de la réponse pour la bufferiser et l'envoyer au client
	while (reponseServeur.length() - cpt > TAILLE) {
		for (int i = cpt; i < cpt + TAILLE; i++) {
			bufferReponse[i - cpt] = reponseServeur.at(i);
		}
		cpt += TAILLE;
		nb = send(sd, bufferReponse, TAILLE, 0);
	}
	// Envoi de la fin de la réponse
	for (int j = cpt; j <= reponseServeur.length() - 1; j++) {
		bufferReponse[j - cpt] = reponseServeur.at(j);
	}
	// Vider le reste du buffer et envoyer la fin de la réponse
	for (int k = reponseServeur.length() - cpt; k <= TAILLE - 1; k++) {
		bufferReponse[k] = ' ';
	}
	nb = send(sd, bufferReponse, TAILLE, 0);

	/*
	
	bool requeteOk; // Si la requête abouti
	int cpt = 0; // Compteur temporaire

	// On attend la fin de la requête client et on stock
	while (requeteClient.find("\r\n\r\n") == -1) {
		nb = recv(sd, buffer, TAILLE, 0);
		requeteClient.append(buffer);
	}
	*/
	/* Etudier la requête client pour stocker les paramètres éventuels */
	/*
	requeteOk = true;
	
	// Préparation de la réponse 

	if(requeteOk) SetHeader(reponseServeur, (char*) "200", "text/html"); // Dans le cas ou il n'y a pas d'erreur
	else SetHeader(reponseServeur, (char*) "404", "text/html"); // Dans le cas ou il y a une erreur de fichier manquant

	// Dans le cas ou l'on accède à la racine
	// Lecture de l'index
	std::ifstream ifs("index.htm");
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	// Ajout du corps de la réponse
	reponseServeur.append(content);

	// Parcours de la réponse pour la bufferiser et l'envoyer au client
	while (reponseServeur.length() - cpt > TAILLE) {
		for (int i = cpt; i < cpt+TAILLE; i++) {
			bufferReponse[i-cpt] = reponseServeur.at(i);
		}
		cpt += TAILLE;
		nb = send(sd, bufferReponse, TAILLE, 0);
	}

	// Envoi de la fin de la réponse
	for (int j = cpt; j <= reponseServeur.length()-1; j++) {
		bufferReponse[j-cpt] = reponseServeur.at(j);
	}
	// Vider le reste du buffer et envoyer la fin de la réponse
	for (int k = reponseServeur.length() - cpt; k <= TAILLE-1; k++) {
		bufferReponse[k] = ' ';
	}
	nb = send(sd, bufferReponse, TAILLE, 0);
	
	// Récupérer l'ensemble de la requete. "http:// /moteur?q=val+
	// Récupérer la partie moteur dans 
	// Derrière le ? on a tous les paramètres qui sont de la forme : param = contenu
	// le contenu devant être parsé pour 

	// générer entete ici avec une fonction
	// Faire un code html dans le corps

	*/
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
