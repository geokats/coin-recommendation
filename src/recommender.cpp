#include "recommender.hpp"
#include "util.hpp"
#include "searcher.hpp"
#include "point.hpp"
#include "clusterCreator.hpp"

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

using namespace std;

float getTweetScore(tweet t, unordered_map<string, float> lexicon){
  float totalScore = 0;
  float alpha = 15;

  for(auto word : t.words){
    auto val = lexicon.find(word);
    if(val != lexicon.end()){
      //If the current word of the tweet is in the lexicon add its value
      totalScore += val->second;
    }
  }

  float score = totalScore / sqrt(totalScore * totalScore + alpha);

  return score;
}

point getCoinScore(tweet t, unordered_map<string, float> lexicon, vector<string> coins, unordered_map<string, int> coinLexicon){
  //Initialise vector values to zeros
  vector<double> vals;
  for(int i = 0; i < coins.size(); i++){
    vals.push_back(0);
  }

  //Check which coins are mentioned in the tweet
  unordered_set<int> coinIndices;
  for(auto word : t.words){
    //Check if word is related to a coin
    auto coinIndex = coinLexicon.find(word);
    if(coinIndex != coinLexicon.end()){
      coinIndices.insert(coinIndex->second);
    }
  }

  //Add score to indices of mentioned coins
  float score = getTweetScore(t, lexicon);
  for(auto index : coinIndices){
    vals.at(index) = score;
  }

  //Add values to existing score vector
  string name = to_string(t.userId) + "_coin_scores";
  point p(name, vals);
  return p;
}

unordered_map<int, point> getUsersScore(unordered_map<int, tweet> tweets, unordered_map<string, float> lexicon, vector<string> coins, unordered_map<string, int> coinLexicon){
  point zeros("zeros", coins.size());

  unordered_map<int, point> u;
  for(auto t : tweets){
    point score = getCoinScore(t.second, lexicon, coins, coinLexicon);
    if(score.equal(zeros)){
      //If tweet doesn't give any information, don't add it
      continue;
    }

    auto entry = u.find(t.second.userId);
    if(entry == u.end()){
      //If user vector hasn't been created yet, add a new entry
      u.emplace(t.second.userId, score);
    }
    else{
      //If user already has a vector, add score to the vector
      entry->second.add(score);
    }
  }
  return u;
}

vector<point> getClustersScore(vector<vector<int>> clusters, unordered_map<int, tweet> tweets, unordered_map<string, float> lexicon, vector<string> coins, unordered_map<string, int> coinLexicon){
  vector<point> c;
  for(auto cluster : clusters){
    //Create vector for cluster
    point score("coin_scores", coins.size());

    for(auto tweetId : cluster){
      //For each tweet in cluster add score vector to cluster vector
      point tweetScore = getCoinScore(tweets[tweetId], lexicon, coins, coinLexicon);
      score.add(tweetScore);
    }
    c.emplace_back(score);
  }
  return c;
}

point getAverageScore(unordered_map<int, point> u, int coinCount){
  point average("average_score", coinCount);
  for(auto entry : u){
    average.add(entry.second);
  }
  average.div((float) coinCount);
  return average;
}

unordered_map<int,point> normaliseScores(unordered_map<int,point> &scores){
  unordered_map<int,point> newScores;
  for(auto entry : scores){
    double mean = entry.second.sumVals() / (double) entry.second.nonZeroVals();
    entry.second.setZeroVals(mean);

    newScores.emplace(entry.first, entry.second);
  }

  return newScores;
}

point getScorePrediction(point user, unordered_set<point*> neighbors){
  point sum("sum", user.dim());
  float simSum = 0;

  for(auto n : neighbors){
    if(n->equal(user)){
      //If the user is in the neighbors, then skip him
      continue;
    }

    float sim = user.cosDist(*n);
    simSum += abs(sim);

    point temp(user);
    temp.diff(*n);
    temp.mult(sim);
    sum.add(temp);
  }
  sum.div(simSum);
  point prediction = user.sum(sum);

  return prediction;
}

vector<string> getCoinRecommendations(point userNotNormalized, point prediction, vector<string> coins, int k){
  userNotNormalized.print();
  prediction.print();

  multimap<double, string> coinScores;

  for(int i = 0; i < prediction.dim(); i++){
    if(userNotNormalized.get(i) == 0.0){
      //If user hadn't talked about the coin
      coinScores.emplace(prediction.get(i), coins[i]);
    }
  }

  int recommendationCount = k > coinScores.size() ? coinScores.size() : k;
  vector<string> recommendations;

  auto entry = coinScores.begin();
  for(int i = 0; i < recommendationCount; i++){
    cout << entry->first << " : " << entry->second << "\n";
    recommendations.emplace_back(entry->second);
    entry++;
  }

  return recommendations;
}

unordered_map<int, point> getLSHPredictions(int k, int L, unordered_map<int, point> u, int coinsCount){
  unordered_map<int, point> predictions;

  //Get user scores to be used as points for LSH
  vector<point> scores;
  for(auto entry : u){
    scores.emplace_back(entry.second);
  }
  //Initialise LSH
  lsh srch = lsh(k, L, coinsCount, "euclidean", &scores);

  for(auto entry : u){
    unordered_set<point*> neighbors;
    int tries = 0;
    float radius = 0.01;
    do{
      neighbors.clear();
      tries++;
      neighbors = srch.rnn(entry.second, radius);
      radius *= 2;
    }while(neighbors.size() < 50 && tries < 50);

    for(auto i1 : neighbors){
      for(auto i2 : neighbors){
        if(i1 != i2){
          if(i1->getName().compare(i2->getName()) == 0){
            cout << "!!\n";
          }
        }
      }
    }

    // cout << "User #" << entry.first << " has " << neighbors.size() << " neighbors after " << tries << " tries\n";

    //Make prediction based on neighbors
    point prediction = getScorePrediction(entry.second, neighbors);
    predictions.emplace(entry.first, prediction);
  }

  return predictions;
}

unordered_map<int, point> getClusteringPredictions(configuration conf, unordered_map<int, point> u, int coinsCount){
  //Get user scores to be used as points for clustering
  vector<point> scores;
  for(auto entry : u){
    scores.emplace_back(entry.second);
  }

  //Initialise clustering
  clusterCreator cl = clusterCreator(&scores, conf);
}