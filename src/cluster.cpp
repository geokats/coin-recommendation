#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <string>
#include <cmath>
#include <bitset>
#include <random>
#include <chrono>
#include <time.h>
#include <unistd.h> //getopt
#include <stdlib.h> //atoi

#include "util.hpp"
#include "point.hpp"
#include "clusterCreator.hpp"

using namespace std;

string usageStr = "./cluster -i <input file> -c <configuration file> -o <output file> -d <metric>\n";

int main(int argc, char* const *argv) {
  //Command line arguments
  char *inputFileName = NULL;
  char *configFileName = NULL;
  char *outputFileName = NULL;
  string metric;

  char c;

  //Read command line arguments
  while((c = getopt(argc, argv, "i:c:o:d:")) != -1){
    switch(c){
      case 'i':
        inputFileName = optarg;
        break;
      case 'c':
        configFileName = optarg;
        break;
      case 'o':
        outputFileName = optarg;
        break;
      case 'd':
        metric = string(optarg);
        break;
      default:
        cerr << "Usage: " << usageStr;
        return 1;
    }
  }

  //Check command line arguments
  if(inputFileName == NULL || configFileName == NULL || outputFileName == NULL || metric.size() == 0){
    cerr << "Usage: " << usageStr;
    return 1;
  }
  if(metric != "euclidean" && metric != "cosine"){
    cerr << "Error: Unknown metric \"" << metric << "\"\n";
    return 1;
  }

  //Read configuration file
  configuration conf;
  readConfigFile(configFileName, conf);
  //Check if configuration is correct
  if(!conf.clusterConf()){
    cerr << "Error: Bad configuration file given\n";
    return 1;
  }
  else{
    cout << "Clustering parameters:\n";
    cout << "\t- Clusters = " << conf.getClusterCount() << "\n";
    cout << "\t- Hash functions = " << conf.getHashFuncCount() << "\n";
    cout << "\t- Hash tables = " << conf.getHashTableCount() << "\n";
    cout << "\n";
  }

  //Read input file
  vector<point> points;
  int inputDim;
  // string metric;
  readInputFile(inputFileName, points, inputDim, metric);
  //Check input dimensions
  if(inputDim == -1){
    cerr << "Error: Inconsistent input dataset dimension\n";
    return 1;
  }

  //Open output file
  fstream outputFile(outputFileName, ios_base::out);
  //Timer variables
  clock_t start, end;
  //Initialise cluster creator
  clusterCreator cl(&points, conf);

  cout << "Starting cluster creation...\n";
  //Get start time
  start = clock();
  cl.makeClusters();
  //Get end time
  end = clock();
  //Get duration
  double clusteringTime = (double) (end - start)/CLOCKS_PER_SEC;
  std::vector<point*> *clusters = cl.getClusters();
  std::vector<point> *centroids = cl.getCentroids();

  for(int i = 0; i < conf.getClusterCount(); i++){
    outputFile << "CLUSTER-" << i << " {size: " << clusters[i].size();
    outputFile << ", centroid: ";
    if(conf.getUpdate() == "kmeans"){
      centroids->at(i).printVals(outputFile);
    }
    else{
      outputFile << centroids->at(i).getName();
    }
    outputFile << "}\n";
  }
  outputFile << "Clustering time: " << clusteringTime << "\n";

  // cout << "Calculating Silhouette scores...\n";
  // vector<float> s = cl.silhouette();
  // float avgS = 0;
  // outputFile << "Silhouette: [";
  // for(int i = 0; i < s.size(); i++){
  //   outputFile << s[i] << ", ";
  //   avgS += s[i];
  // }
  // outputFile << avgS << "]\n";


  outputFile.close();
  return 0;
}
