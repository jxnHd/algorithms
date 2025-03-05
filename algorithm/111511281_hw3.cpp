#pragma GCC optimize("Ofast", "inline", "-ffast-math")
#pragma GCC target("avx,sse2,sse3,sse4,mmx")

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;


int mincost(vector<int>& cost) {
    int n = cost.size();
    
    vector<int> dp(n);
    
    dp[n] = 0;
    dp[n + 1] = 0;
    dp[n + 2] = 0;
    
    for(int i = n - 1; i >= 0; i--) {
        dp[i] = cost[i] + min(dp[i + 1], dp[i + 3]);
    }
    
    return min(dp[0], dp[1]);
}

int main(){
    string num;
    vector<int> cost;
    if(getline(cin,num)){
        istringstream iss(num);
        int n;
        while(iss >> n){
            cost.push_back(n);
        }
    }
    cout << mincost(cost) << endl;

    // for(int i=0; i<cost.size();i++){
    //     cout << cost[i] << " ";
    // }
    // cout << endl;


    return 0;

}
