#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
using namespace std;

// ---- Pair ----
template<typename T1, typename T2>
ostream& operator<<(ostream& os, const pair<T1, T2>& p) {
    return os << "(" << p.first << ", " << p.second << ")";
}

// ---- Tuple ----
template<class Tuple, size_t... Is>
void dbg_tuple(const Tuple& t, index_sequence<Is...>) {
    int unused[] = {0, ((cout << (Is == 0 ? "" : ", ") << get<Is>(t)), 0)...};
    (void)unused;
}
template<typename... Args>
void dbg(const tuple<Args...>& t) {
    cout << "(";
    dbg_tuple(t, index_sequence_for<Args...>{});
    cout << ")";
}

// ---- Special cases ----
inline void dbg(const string& s) { cout << "\"" << s << "\""; }
inline void dbg(const char& c) { cout << "'" << c << "'"; }

// ---- Type traits ----
template<typename T>
class is_iterable {
 private:
  template<typename U>
  static auto test(int) -> decltype(begin(declval<U>()), end(declval<U>()), true_type());

  template<typename>
  static false_type test(...);

 public:
  static const bool value = is_same<decltype(test<T>(0)), true_type>::value;
};


template<typename T>
typename enable_if<!is_iterable<T>::value && !is_same<T, string>::value &&
                                     !is_same<T, char>::value, void>::type
dbg(const T& val);

// ---- Containers ----

template<typename K, typename V>
void dbg(const std::pair<K, V>& p) {
    cout << "(";
    dbg(p.first);
    cout << ": ";
    dbg(p.second);
    cout << ")";
}

template<typename T>
typename enable_if<is_iterable<T>::value && !is_same<T, string>::value, void>::type
dbg(const T& container) {
    cout << "{";
    for (auto it = begin(container); it != end(container); ++it) {
        if (it != begin(container)) cout << ", ";
        dbg(*it);
    }
    cout << "}";
}

// ---- Scalar types ----
template<typename T>
typename enable_if<!is_iterable<T>::value && !is_same<T, string>::value &&
                   !is_same<T, char>::value, void>::type
dbg(const T& val) {
    cout << val;
}

// ---- Variadic print ----
inline void print() {
    cout << endl;
}

template<typename T>
void print(const T& single) {
    dbg(single);
    cout << endl;
}

template<typename T, typename... Rest>
void print(const T& first, const Rest&... rest) {
    dbg(first);
    cout << " | ";
    print(rest...);
}

// ---- Aliases and constants ----
#define ll long long
#define ld long double
#define pii pair<int, int>
#define tii tuple<int, int>
#define tll tuple<ll, ll>
#define vi vector<int>
#define vb vector<bool>
#define vvi vector<vector<int>>
#define sza(x) ((int)x.size())
#define all(a) (a).begin(), (a).end()

const int MAX_N = 1e5 + 5;
const ll MOD = 1e9 + 7;
const ll INF = 1e9;
const ld EPS = 1e-9;
