#pragma GCC optimize("Ofast", "inline", "-ffast-math")
#pragma GCC target("avx,sse2,sse3,sse4,mmx")

#include <iostream>
#include <vector>
#include <sstream>

using namespace std;


class RBTree {
private:
    struct Node {
        static const bool RED = true, BLACK = false;
        long long num;
        int cnt = 1, size = 1;
        Node* left = nullptr;
        Node* right = nullptr;
        bool color = RED;
        Node(long long n) : num(n) {}
    };

    Node* root = nullptr;

    int getSize(Node* n){
        if( n!= nullptr){
            return n->size;
        }
        else{
            return 0;
        }
    }

    Node* rotateLeft(Node* n) {
        Node* tmp = n->right;
        n->right = tmp->left;
        tmp->left = n;
        tmp->color = n->color;
        n->color = Node::RED;
        tmp->size = n->size;
        n->size = getSize(n->left) + getSize(n->right) + n->cnt;
        return tmp;
    }

    Node* rotateRight(Node* n) {
        Node* tmp = n->left;
        n->left = tmp->right;
        tmp->right = n;
        tmp->color = n->color;
        n->color = Node::RED;
        tmp->size = n->size;
        n->size = getSize(n->left) + getSize(n->right) + n->cnt;
        return tmp;
    }

    void flipColors(Node* n) {
        n->color = Node::RED;
        n->left->color = Node::BLACK;
        n->right->color = Node::BLACK;
    }

    bool isRed(Node* n) { return n && n->color == Node::RED; }


    Node* insert(Node* n, long long num) {
        if (n == nullptr) return new Node(num);
        n->size++;
        if (num == n->num) n->cnt++;
        else if (num < n->num) n->left = insert(n->left, num);
        else n->right = insert(n->right, num);
        if (isRed(n->right) && !isRed(n->left)) n = rotateLeft(n);
        if (isRed(n->left) && isRed(n->left->left)) n = rotateRight(n);
        if (isRed(n->left) && isRed(n->right)) flipColors(n);
        return n;
    }

    int numSmaller(Node* n, long long num, bool self) {
        if(n == nullptr){
            return 0;
        }
        else if(num == n->num){
            return getSize(n->left) + (self ? n->cnt : 0);
        }
        else if(num < n->num){
            return numSmaller(n->left,num,self);
        }
        else{
            return getSize(n->left) + n->cnt + numSmaller(n->right,num,self);
        }
    }

public:
    void insert(long long num) {
        root = insert(root, num);
        root->color = Node::BLACK;
    }

    int numSmaller(long long num, bool self) {
        return numSmaller(root, num, self);
    }
};

int subcounter(const vector<int>& nums, long long p, long long q) {
    RBTree rbt;
    int ans = 0;
    rbt.insert(0);
    for (long long i = 0, sum = 0; i < nums.size(); i++) {
        sum += nums[i];
        ans += rbt.numSmaller(sum - p, true) - rbt.numSmaller(sum - q, false);
        rbt.insert(sum);
    }
    return ans;
}

int main() {
    string line;
    vector<int> nums;
    long long lower, upper;
    if (getline(cin, line)) {
        istringstream iss(line);
        int num;
        while (iss >> num){
            nums.push_back(num);
        }
    }
    if (getline(cin, line)) {
        istringstream iss(line);
        iss >> lower >> upper;
    }
    cout << subcounter(nums, lower, upper) << endl;
    return 0;
}
