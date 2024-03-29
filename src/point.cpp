#include "point.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h> //atoi
#include <cmath>
#include <random>
#include <chrono>

using namespace std;

point::point(string name, int dim, int mean, int dev){
  //Initialise random real number generator
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  normal_distribution<double> norm(mean, dev);
  default_random_engine re(seed);

  this->name = name;
  for(int i = 0; i < dim; i++){
    vals.push_back(norm(re));
  }
}

point::point(std::string line){
  string str;
  int i = 0;
  for(int j = 0; j < line.length(); j++){
    if(line[j] == '\t' || line[j] == ','){
      if(i == 0){
        name = line.substr(i, j - i);
      }
      else{
        str = line.substr(i, j - i);
        if(str.find_first_not_of(" \t\r\n") != str.npos){
          vals.push_back(atof(str.c_str()));
        }
      }
      i = j + 1;
    }
  }
  if(i < line.length()){
    str = line.substr(i);
    if(str.find_first_not_of(" \t\r\n") != str.npos){
      vals.push_back(atof(str.c_str()));
    }
  }
}

point::point(string name, vector<double> vals){
  this->name = name;
  this->vals = vals;
}

point::point(string name, int dim){
  this->name = name;
  for(int i = 0; i < dim; i++){
    this->vals.push_back(0);
  }
}

point::point(const point &p){
  this->name = p.name;
  this->vals = p.vals;
}

void point::print(){
  cout << name << ": ";
  for(vector<double>::iterator it = vals.begin(); it != vals.end(); it++){
    cout << *it << ' ';
  }
  cout << '\n';
}

void point::printVals(ostream &stream){
  stream << "(";
  for(int i = 0; i < vals.size(); i++){
    stream << vals[i];
    if(i + 1 < vals.size()){
      stream << ", ";
    }
  }
  stream << ")";
}

double point::get(int i){
  return vals[i];
}

void point::set(int i, double val){
  vals[i] = val;
}

int point::dim(){
  return vals.size();
}

string point::getName(){
  return name;
}

void point::rename(string newName){
  name = newName;
}

double point::euclDist(point p){
  if(this->vals.size() != p.vals.size()){
    cerr << "Error: Dimensions don't match";
    return -1.0;
  }
  double dist = 0.0;
  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    dist += pow(*it1 - *it2, 2);

    it1++;
    it2++;
  }

  dist = sqrt(dist);
  return dist;
}

double point::cosDist(point p){
  if(this->vals.size() != p.vals.size()){
    cerr << "Error: Dimensions don't match";
    return -1.0;
  }

  float prod = 0;
  float sumA = 0;
  float sumB = 0;

  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();
  for(int i = 0; i < this->vals.size(); i++){
    prod += *it1 * *it2;
    sumA += *it1 * *it1;
    sumB += *it2 * *it2;

    it1++;
    it2++;
  }

  double dist = prod/(sqrt(sumA) * sqrt(sumB));

  return dist;
}

double point::product(point p){
  double prod = 0.0;
  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    prod += *it1 * *it2;

    it1++;
    it2++;
  }

  return prod;
}

point point::sum(point p){
  vector<double> newVals;

  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    newVals.push_back(*it1 + *it2);

    it1++;
    it2++;
  }

  return point("", newVals);
}

point point::diff(point p){
  vector<double> newVals;

  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    newVals.push_back(*it1 - *it2);

    it1++;
    it2++;
  }

  return point("", newVals);
}

bool point::equal(point p){
  if(this->vals.size() != p.vals.size()){
    return false;
  }
  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    if(*it1 != *it2){
      return false;
    }
  }

  return true;
}

void point::add(point p){
  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    *it1 += *it2;

    it1++;
    it2++;
  }
}

void point::sub(point p){
  vector<double>::iterator it1 = this->vals.begin();
  vector<double>::iterator it2 = p.vals.begin();

  for(int i = 0; i < this->vals.size(); i++){
    *it1 -= *it2;

    it1++;
    it2++;
  }
}

void point::div(float f){
  for(int i = 0; i < this->vals.size(); i++){
    vals[i] = vals[i] / f;
  }
}

void point::mult(float f){
  for(int i = 0; i < this->vals.size(); i++){
    vals[i] = vals[i] * f;
  }
}

int point::nonZeroVals(){
  int count = 0;
  for(int i = 0; i < this->vals.size(); i++){
    if(vals[i] != 0){
      count++;
    }
  }
  return count;
}

double point::sumVals(){
  double sum = 0;
  for(int i = 0; i < this->vals.size(); i++){
    sum += vals[i];
  }
  return sum;
}

void point::setZeroVals(double val){
  for(int i = 0; i < this->vals.size(); i++){
    if(vals[i] == 0){
      vals[i] = val;
    }
  }
}
