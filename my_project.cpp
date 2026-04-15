#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <cassert>
#include <chrono>

#include "cpp_template.hpp"

#ifdef SINGLE
#define REAL float
#else
#define REAL double
#endif

extern "C" {
void exactinit();
REAL orient2d(REAL *pa, REAL *pb, REAL *pc);
REAL incircle(REAL *pa, REAL *pb, REAL *pc, REAL *pd);
}

struct PairHash {
    std::size_t operator()(const std::pair<REAL, REAL>& p) const {
        std::size_t hash1 = std::hash<REAL>{}(p.first);
        std::size_t hash2 = std::hash<REAL>{}(p.second);
        return hash1 ^ (hash2 << 1);
    }
};

struct QuadEdge; 

struct Edge {
    QuadEdge *quad; // pointer to the parent quad-edge
    pair<REAL, REAL> data; // contains coordinate of e.Org
    Edge *next; // takes you to the next CCW quad-edge about e.Org
};

struct QuadEdge {
    Edge edges[4]; 
};

Edge *Rot(Edge *e) {
    return &e->quad->edges[(e - e->quad->edges + 1) % 4];
}

Edge *Sym(Edge *e) {
    return &e->quad->edges[(e - e->quad->edges + 2) % 4];
}

Edge *InvRot(Edge *e) {
    return &e->quad->edges[(e - e->quad->edges + 3) % 4];
}

Edge *Onext(Edge *e) {
    return e->next;
}

Edge *Oprev(Edge *e) {
    return Rot(Onext(Rot(e)));
}

Edge *Dnext(Edge *e) {
    return Sym(Onext(Sym(e)));
}

Edge *Dprev(Edge *e) {
    return InvRot(Onext(InvRot(e)));
}

// These are the important ones for YOUR convention:
Edge *Lnext(Edge *e) {
    return Rot(Onext(InvRot(e)));
}

Edge *Lprev(Edge *e) {
    return Sym(Onext(e));
}

Edge *Rnext(Edge *e) {
    return InvRot(Onext(Rot(e)));
}

Edge *Rprev(Edge *e) {
    return Onext(Sym(e));
}

pair<REAL, REAL> Org(Edge *e) {
    return e->data;
}

pair<REAL, REAL> Dest(Edge *e) {
    return Sym(e)->data;
}

Edge *MakeEdge(pair<REAL, REAL> org, pair<REAL, REAL> dest) {
    QuadEdge *quad = new QuadEdge();

    Edge *e0 = &quad->edges[0];
    Edge *e1 = &quad->edges[1];
    Edge *e2 = &quad->edges[2];
    Edge *e3 = &quad->edges[3];

    e0->quad = e1->quad = e2->quad = e3->quad = quad;

    e0->next = e0;
    e1->next = e3;
    e2->next = e2;
    e3->next = e1;

    e0->data = org;
    e2->data = dest;

    return e0;
}

void Splice(Edge *a, Edge *b) {
    Edge *alpha = Rot(Onext(a));
    Edge *beta = Rot(Onext(b)); 

    // swap(a->next, b->next);
    Edge *temp = a->next;
    a->next = b->next;
    b->next = temp;

    // swap(alpha->next, beta->next);
    temp = alpha->next;
    alpha->next = beta->next;
    beta->next = temp;
}

Edge* Connect(Edge *a, Edge *b) {
    Edge *e = MakeEdge(Dest(a), Org(b));
    Splice(e, Lnext(a));
    Splice(Sym(e), b);
    return e; 
}

void DeleteEdge(Edge *e) {
    Splice(e, Oprev(e));
    Splice(Sym(e), Oprev(Sym(e)));
}

int CCW(pair<REAL, REAL> a, pair<REAL, REAL> b, pair<REAL, REAL> c) {
    REAL res = orient2d(&a.first, &b.first, &c.first);
    if (res > 0) return 1; // ccw
    if (res < 0) return -1; // cw
    return 0; // collinear
}

int RightOf(pair<REAL, REAL> X, Edge *e) {
    return CCW(X, Dest(e), Org(e));
}

bool Valid(Edge *e, Edge *base) {
    return RightOf(Dest(e), base) > 0;
}

int LeftOf(pair<REAL, REAL> X, Edge *e) {
    return CCW(X, Org(e), Dest(e));
}

int InCircle(pair<REAL, REAL> a, pair<REAL, REAL> b, pair<REAL, REAL> c, pair<REAL, REAL> d) {
    REAL res = incircle(&a.first, &b.first, &c.first, &d.first);
    if (res > 0) return 1; // inside
    if (res < 0) return -1; // outside
    return 0; // on the circle
}

bool comp (pair<REAL, REAL> a, pair<REAL, REAL> b) {
    // sort by x-coordinate, and if tie, sort by y-coordinate
    return a.first < b.first || (a.first == b.first && a.second < b.second); 
}

pair<std::vector<pair<REAL, REAL>>, std::vector<pair<REAL, REAL>>> vertical_cut(const std::vector<pair<REAL, REAL>>& points) {
    int n = static_cast<int>(points.size());
    int half = n / 2;
    std::vector<pair<REAL, REAL>> sorted_points = points;
    std::sort(sorted_points.begin(), sorted_points.end(), comp);
    std::vector<pair<REAL, REAL>> L_points(sorted_points.begin(), sorted_points.begin() + half);
    std::vector<pair<REAL, REAL>> R_points(sorted_points.begin() + half, sorted_points.end());

    return {L_points, R_points};
}

pair<Edge*, Edge*> dt(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts) {
    int n = static_cast<int>(points.size());

    if (n == 2) {
        std::sort(points.begin(), points.end(), comp);
        Edge *a = MakeEdge(points[0], points[1]);
        return {a, Sym(a)};
    }
    if (n == 3) {
        std::sort(points.begin(), points.end(), comp);
        Edge *a = MakeEdge(points[0], points[1]);
        Edge *b = MakeEdge(points[1], points[2]);
        Splice(Sym(a), b);
        
        if (CCW(points[0], points[1], points[2]) > 0) {
            Edge *c = Connect(b, a);

            return {a, Sym(b)};
        } else if (CCW(points[0], points[2], points[1]) > 0) {
            Edge *c = Connect(b, a);
            

            assert(Org(a) == points[0] && Dest(a) == points[1]);
            assert(Org(b) == points[1] && Dest(b) == points[2]);
            assert(Org(c) == points[2] && Dest(c) == points[0]);
            
            Edge *u = Sym(a);
            Edge *v = Lnext(u);
            Edge *w = Lnext(v);

            return {Sym(c), c};
        } else {
            return {a, Sym(b)};
        }
    }

    vector<pair<REAL, REAL>> L_points, R_points;

    if (!alternating_cuts) {
        auto cuts = vertical_cut(points);
        L_points = cuts.first;
        R_points = cuts.second;
    }  else {
        auto cuts = vertical_cut(points);
        L_points = cuts.first;
        R_points = cuts.second;
    }

    auto [ldo, ldi] = dt(L_points, alternating_cuts);
    auto [rdi, rdo] = dt(R_points, alternating_cuts);

    // compute the lower common tangent of L and R
    do {
        if (LeftOf(Org(rdi), ldi) > 0) {
            ldi = Lnext(ldi);
        } else if (RightOf(Org(ldi), rdi) > 0) {
            rdi = Rprev(rdi);
        } else {
            break;
        }
    } while (1);

    // create first cross edge base1 from rdi.Org to ldi.Org
    Edge *base = Connect(Sym(rdi), ldi);
    if (Org(ldi) == Org(ldo)) {
        ldo = Sym(base);
    }
    if (Org(rdi) == Org(rdo)) {
        rdo = base;
    }
    do { // this is the merge loop
        // locate the first L point (lcand) to be encountered by the rising bubble
        // and delete L edges out of base.Dest that fail the circle test
        Edge *lcand = Onext(Sym(base));
        if (Valid(lcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(lcand), Dest(Onext(lcand))) > 0) {
                Edge *t = Onext(lcand);
                DeleteEdge(lcand);
                lcand = t;
            }
        }
        // symmetrically, locate the first R point to be hit, and delete R edges
        Edge *rcand = Oprev(base);
        if (Valid(rcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(rcand), Dest(Oprev(rcand))) > 0) {
                Edge *t = Oprev(rcand);
                DeleteEdge(rcand);
                rcand = t;
            }
        }
        
        // if both lcand and rcand are invalid, then base is the upper common tangent
        if (!Valid(lcand, base) && !Valid(rcand, base)){
            break;
        }

        if (!Valid(lcand, base) || (Valid(rcand, base) && InCircle(Dest(lcand), Org(lcand), Org(rcand), Dest(rcand)) > 0)) {
            base = Connect(rcand, Sym(base));
        } else {
            base = Connect(Sym(base), Sym(lcand));
        }
    } while (true);

    return {ldo, rdo}; 
}

int main(int argc, char **argv) {

    std::string node_path, out_path;
    if (argc <= 1) {
        node_path = "tests/spiral.node";
        out_path = "tests/spiral.ele";
        print("No input path provided. Using: ", node_path);
        print("Using output path: ", out_path);
    } else {
        node_path = argv[1];
        out_path = node_path.substr(0, node_path.find_last_of('.')) + ".ele";
        print("Using input path: ", node_path);
        print("Using output path: ", out_path);
    }
    print();

    bool alternating_cuts = false; 
    print("0: vertical cuts only (default), 1: alternating cuts");
    if (argc <= 2) {
        print("No alternating cuts flag provided. Using: ", alternating_cuts);
    } else {
        alternating_cuts = (std::string(argv[2])[0] == 'a' || std::string(argv[2])[0] == 'A');
        print("Using alternating cuts flag: ", alternating_cuts);
    }
    print();

    exactinit(); 

    std::ifstream node_file(node_path);
    if (!node_file) {
        std::cerr << "Error opening path: " << node_path << "\n";
        return 1;
    }

    int n = 0;
    int trash; 
    if (!(node_file >> n >> trash >> trash >> trash)) {
        std::cerr << "Error \n";
        return 1;
    }

    std::vector<pair<REAL, REAL>> points;
    std::unordered_map<pair<REAL, REAL>, int, PairHash> point_to_index; // for outputting indices in the end
    points.reserve(n);

    for (int i = 0; i < n; i++) {
        int index;
        REAL x, y; 

        if (!(node_file >> index >> x >> y)) {
            std::cerr << "Error \n";
            return 1;
        }
        points.push_back({x, y});
        point_to_index[{x, y}] = index;
    }

    // compute DT
    auto start = std::chrono::steady_clock::now();
    auto [left, _] = dt(points, alternating_cuts);
    auto end = std::chrono::steady_clock::now();
    print("DT computed in ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), " ms");

    // write output
    std::ofstream out_file(out_path);
    if (!out_file) {
        std::cerr << "Error \n";
        return 1;
    }

    // print(point_to_index);

    queue<Edge*> q;
    set<Edge*> visited;
    vector<tuple<int, int, int>> triangles; 
    q.push(left);

    while (!q.empty()) {
        Edge *e = q.front();
        q.pop();

        assert(e != nullptr);
        assert(Org(e) != Dest(e)); 
        assert(e == Sym(Sym(e)));
        assert(e == Onext(Oprev(e)));
        assert(e == Oprev(Onext(e)));

        if (visited.count(e)) {
            continue;
        }

        // extract triangle
        Edge *e2 = Lnext(e);
        Edge *e3 = Lnext(e2);
        if (Lnext(e3) == e) {
            triangles.push_back({
                point_to_index[Org(e)],
                point_to_index[Org(e2)],
                point_to_index[Org(e3)]
            }); 
        }

        // print("e ", Org(e), Dest(e), point_to_index[Org(e)], point_to_index[Dest(e)]);
        // print("e2", Org(e2), Dest(e2), point_to_index[Org(e2)], point_to_index[Dest(e2)]);
        // print("e3", Org(e3), Dest(e3), point_to_index[Org(e3)], point_to_index[Dest(e3)]);
        // print(Org(Lnext(e3)), Org(e));
        // print(point_to_index[Org(Lnext(e3))], point_to_index[Org(e)]);
        // print("same edge?", Lnext(e3) == e);

        visited.insert(e);
        
        q.push(Onext(e));
        q.push(Sym(e));
        q.push(Lnext(e));
    }

    for (auto &[a, b, c] : triangles) {
        if (a > b) swap(a, b);
        if (b > c) swap(b, c);
        if (a > b) swap(a, b);
    }
    sort(triangles.begin(), triangles.end());
    triangles.erase(unique(triangles.begin(), triangles.end()), triangles.end());
    
    out_file << triangles.size() << " 3 0\n";
    int index = 0; 
    for (const auto& [a, b, c] : triangles) {
        out_file << ++index << " " << a << " " << b << " " << c << "\n";
        // print(index, a, b, c);
    }

    return 0;
}
