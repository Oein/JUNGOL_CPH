#include <iostream>

#define PROBLEM 1000
#define TIMEOUT 1000LL

using namespace std;

int solution() {
    int a, b;
    cin >> a >> b;
    cout << a + b;
    return 0;
}

#ifdef ONLINE_JUDGE
int main() {
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
    solution();
    return 0;
}
#endif
