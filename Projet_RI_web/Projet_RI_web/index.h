#pragma once
#include<map>

void IndexData();
std::string SearchPages(SOCKET sd, std::string requete);
std::map<int, float> computePageRank(std::string linksFile);
std::string getSelectWordID(std::string mot);