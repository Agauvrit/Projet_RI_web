#include <ctime>
#include <sstream>
#include <fstream>
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
	char buffer[TAILLE];
	char bufferReponse[TAILLE];
	int nb;
	std::string requeteClient;
	std::string reponseServeur;

	while (requeteClient.find("\r\n\r\n") == -1) {
		nb = recv(sd, buffer, TAILLE, 0);
		requeteClient.append(buffer);
	}
	
	SetHeader(reponseServeur, (char*) "200", "text/html");
	
	std::ifstream ifs("index.htm");
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	reponseServeur.append(content);

	int i;
	int cpt = 0;
	while (reponseServeur.length() - cpt > TAILLE) {
		for (i = cpt; i <= cpt+TAILLE; i++) {
			bufferReponse[i] = reponseServeur.at(i);
		}
		cpt += TAILLE;
		printf(bufferReponse);
		nb = send(sd, bufferReponse, TAILLE, 0);
	}

	for (int j = cpt; j <= reponseServeur.length()-1; j++) {
		bufferReponse[j-cpt] = reponseServeur.at(j);
	}
	// Maintenant il faut vider le reste du buffer.
	for (int k = reponseServeur.length() - cpt; k <= TAILLE; k++) {
		bufferReponse[k] = ' ';
	}
	nb = send(sd, bufferReponse, TAILLE, 0);
	
	// Récupérer l'ensemble de la requete. "http:// /moteur?q=val+
	// Récupérer la partie moteur dans 
	// Derrière le ? on a tous les paramètres qui sont de la forme : param = contenu
	// le contenu devant être parsé pour 

	// générer entete ici avec une fonction
	// Faire un code html dans le corps

	
}

void HttpRequest::SetHeader(std::string &AnswerBuf, char *httpCode, const char *mime)
{
	std::stringstream header;

	// GetTime
	auto t=std::time(nullptr);

	// Set HTTP header
	header << "HTTP/1.0 " << httpCode << "\r\n";
	header << "Date: " << std::put_time(std::gmtime(&t), "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
	header << "Server: WebSearchServer/1.0" << "\r\n";
	header << "Content-Type: " << mime << "\r\n";
	header << "\r\n";

	AnswerBuf=header.str();
}
