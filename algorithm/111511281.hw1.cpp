#pragma GCC optimize("Ofast","inline","-ffast-math")
#pragma GCC target("avx,sse2,sse3,sse4,mmx")

#include <vector>
#include <iostream>

using namespace std;

vector<int> tokens;
int left, right, power, token = 0;

int Partition(vector<int>& v, int low, int end){
    int pivot = v[end];
    int i = low - 1;
    for(int j = low; j<= end-1; j++){
        if(v[j] <= pivot){
            i++;
            swap(v[i],v[j]);
        }
    }
    swap(v[i+1], v[end]);

    return (i+1);
}

void quicksort(vector<int>& vec, int left, int right){
    if(left < right){
        int pivot  = Partition(vec, left, right);
        quicksort(vec, left, pivot-1);
        quicksort(vec, pivot+1, right);
    }

}

int maxscore(vector<int>& t, int p){

    int score = 0;
    int maxcount = 0;
    int i=0;
    int j = t.size() - 1;

    while(i<=j){
        if(p >= t[i]){
            p -= t[i];
            score++;
            maxcount = max(maxcount,score);
            i++;
        }
        else if(score >= 0){
            p += t[j];
            score--;
            j--;
        }
        else
            break;
    }
    return maxcount;
}

int main(){
    cin >> power;
    while(cin >> token){
        if(token < 0){
            break;
        }
        else{
            tokens.push_back(token);
        }
    }

    int n = tokens.size();
    quicksort(tokens , 0 , n-1);

    cout << maxscore(tokens, power) << endl;

    return 0;

}