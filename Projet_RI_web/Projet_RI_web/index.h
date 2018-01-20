#pragma once
#include<map>

void IndexData();
void SearchPages(SOCKET sd, std::string requete);
std::map<int, int> computePageRank(std::string linksFile);