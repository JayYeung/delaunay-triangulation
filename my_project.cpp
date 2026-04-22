#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>
#include <cassert>
#include <chrono>
#include <algorithm>

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

Edge *Rot(Edge *e){ return &e->quad->edges[(e - e->quad->edges + 1) % 4]; }
Edge *Sym(Edge *e){ return &e->quad->edges[(e - e->quad->edges + 2) % 4]; }
Edge *InvRot(Edge *e){ return &e->quad->edges[(e - e->quad->edges + 3) % 4]; }

Edge *Onext(Edge *e){ return e->next; }
Edge *Oprev(Edge *e){ return Rot(Onext(Rot(e))); }
Edge *Dnext(Edge *e){ return Sym(Onext(Sym(e))); }
Edge *Dprev(Edge *e){ return InvRot(Onext(InvRot(e))); }
Edge *Lnext(Edge *e){ return Rot(Onext(InvRot(e))); }
Edge *Lprev(Edge *e){ return Sym(Onext(e)); }
Edge *Rnext(Edge *e){ return InvRot(Onext(Rot(e))); }
Edge *Rprev(Edge *e){ return Onext(Sym(e)); }

pair<REAL, REAL> Org(Edge *e)  { return e->data; }
pair<REAL, REAL> Dest(Edge *e) { return Sym(e)->data; }

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
    Edge *beta  = Rot(Onext(b));
    Edge *temp = a->next; a->next = b->next; b->next = temp;
    temp = alpha->next; alpha->next = beta->next; beta->next = temp;
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

int RightOf(pair<REAL, REAL> X, Edge *e) { return CCW(X, Dest(e),Org(e)); }
int LeftOf (pair<REAL, REAL> X, Edge *e) { return CCW(X, Org(e),Dest(e)); }

bool Valid(Edge *e, Edge *base) { return RightOf(Dest(e), base) > 0; }

int InCircle(pair<REAL, REAL> a, pair<REAL, REAL> b, pair<REAL, REAL> c, pair<REAL, REAL> d) {
    REAL res = incircle(&a.first, &b.first, &c.first, &d.first);
    if (res > 0) return 1; // inside
    if (res < 0) return -1; // outside
    return 0; // on the circle
}

bool comp(pair<REAL, REAL> a, pair<REAL, REAL> b) {
    return a.first < b.first || (a.first == b.first && a.second < b.second);
}

bool comp_y(pair<REAL, REAL> a, pair<REAL, REAL> b) {
    return a.second < b.second || (a.second == b.second && a.first < b.first);
}

struct Hull {
    Edge *ldo;
    Edge *rdo;
    Edge *bbo;
    Edge *tbo;
};

Hull hull_from_seed(Edge *seed) {
    std::vector<Edge*> ring;
    Edge *cur = seed;
    do {
        ring.push_back(cur);
        cur = Lnext(cur);
    } while (cur != seed);

    int m = static_cast<int>(ring.size());
    assert(m > 0);

    int iL = 0, iR = 0, iB = 0, iT = 0;
    for (int i = 1; i < m; ++i) {
        pair<REAL, REAL> p  = Org(ring[i]);
        pair<REAL, REAL> lp = Org(ring[iL]);
        pair<REAL, REAL> rp = Org(ring[iR]);
        pair<REAL, REAL> bp = Org(ring[iB]);
        pair<REAL, REAL> tp = Org(ring[iT]);
        if (p.first  < lp.first  || (p.first  == lp.first  && p.second < lp.second))  iL = i;
        if (p.first  > rp.first  || (p.first  == rp.first  && p.second > rp.second))  iR = i;
        if (p.second < bp.second || (p.second == bp.second && p.first  < bp.first))   iB = i;
        if (p.second > tp.second || (p.second == tp.second && p.first  > tp.first))   iT = i;
    }

    Hull h;
    h.ldo = Sym(ring[(iL - 1 + m) % m]);
    h.rdo = ring[iR];
    h.bbo = Sym(ring[(iB - 1 + m) % m]);
    h.tbo = ring[iT];
    return h;
}

pair<std::vector<pair<REAL, REAL>>, std::vector<pair<REAL, REAL>>>vertical_cut(const std::vector<pair<REAL, REAL>>& points) {
    int n = (int)points.size();
    int half = n / 2;
    std::vector<pair<REAL, REAL>> s = points;
    std::nth_element(s.begin(), s.begin() + half, s.end(), comp);
    return {{s.begin(), s.begin() + half}, {s.begin() + half, s.end()}};
}

pair<std::vector<pair<REAL, REAL>>, std::vector<pair<REAL, REAL>>>
horizontal_cut(const std::vector<pair<REAL, REAL>>& points) {
    int n = (int)points.size();
    int half = n / 2;
    std::vector<pair<REAL, REAL>> s = points;
    std::nth_element(s.begin(), s.begin() + half, s.end(), comp_y);
    return {{s.begin(), s.begin() + half}, {s.begin() + half, s.end()}};
}

Hull dt(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth = 0);
Hull dt_vertical(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth = 0);
Hull dt_horizontal(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth = 0);

Hull dt_vertical(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth) {
    int n = (int)points.size();

    if (n == 2) {
        std::sort(points.begin(), points.end(), comp);
        Edge *a = MakeEdge(points[0], points[1]);
        return hull_from_seed(Sym(a));
    }
    if (n == 3) {
        std::sort(points.begin(), points.end(), comp);
        Edge *a = MakeEdge(points[0], points[1]);
        Edge *b = MakeEdge(points[1], points[2]);
        Splice(Sym(a), b);
        if (CCW(points[0], points[1], points[2]) > 0) {
            Connect(b, a);
            return hull_from_seed(Sym(b));
        }
        if (CCW(points[0], points[2], points[1]) > 0) {
            Edge *c = Connect(b, a);
            return hull_from_seed(c);
        }
        return hull_from_seed(Sym(b));
    }

    auto cuts = vertical_cut(points);
    std::vector<pair<REAL, REAL>> L_points = cuts.first;
    std::vector<pair<REAL, REAL>> R_points = cuts.second;
    Hull lh = dt(L_points, alternating_cuts, depth + 1);
    Hull rh = dt(R_points, alternating_cuts, depth + 1);

    Edge *ldo = lh.ldo;
    Edge *ldi = lh.rdo;
    Edge *rdi = rh.ldo;
    Edge *rdo = rh.rdo;

    while (true) {
        if (LeftOf (Org(rdi), ldi) > 0) { 
            ldi = Lnext(ldi);
        } else if (RightOf(Org(ldi), rdi) > 0) {
            rdi = Rprev(rdi);
        } else {
            break;
        }
    }

    Edge *base = Connect(Sym(rdi), ldi);
    Edge *first_base = base;

    do {
        Edge *lcand = Onext(Sym(base));
        if (Valid(lcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(lcand), Dest(Onext(lcand))) > 0) {
                Edge *t = Onext(lcand); DeleteEdge(lcand); lcand = t;
            }
        }
        Edge *rcand = Oprev(base);
        if (Valid(rcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(rcand), Dest(Oprev(rcand))) > 0) {
                Edge *t = Oprev(rcand);
                DeleteEdge(rcand);
                rcand = t;
            }
        }
        
        if (!Valid(lcand, base) && !Valid(rcand, base)){
            break;
        }

        if (!Valid(lcand, base) || (Valid(rcand, base) && InCircle(Dest(lcand), Org(lcand), Org(rcand), Dest(rcand)) > 0)) {
            base = Connect(rcand, Sym(base));
        } else {
            base = Connect(Sym(base), Sym(lcand));
        }
    } while (true);

    return hull_from_seed(first_base);
}

Hull dt_horizontal(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth) {
    int n = (int)points.size();

    if (n == 2) {
        std::sort(points.begin(), points.end(), comp_y);
        Edge *a = MakeEdge(points[0], points[1]);
        return hull_from_seed(a);
    }
    if (n == 3) {
        std::sort(points.begin(), points.end(), comp_y);
        Edge *a = MakeEdge(points[0], points[1]);
        Edge *b = MakeEdge(points[1], points[2]);
        Splice(Sym(a), b);
        if (CCW(points[0], points[1], points[2]) > 0) {
            Connect(b, a);
            return hull_from_seed(Sym(a));
        }
        if (CCW(points[0], points[2], points[1]) > 0) {
            Connect(b, a);
            return hull_from_seed(a);
        }
        return hull_from_seed(a);
    }

    auto cuts = horizontal_cut(points);
    std::vector<pair<REAL, REAL>> B_points = cuts.first;
    std::vector<pair<REAL, REAL>> T_points = cuts.second;
    Hull bh = dt(B_points, alternating_cuts, depth + 1);
    Hull th = dt(T_points, alternating_cuts, depth + 1);

    Edge *bti = Onext(bh.tbo);
    Edge *tbi = Oprev(th.bbo);

    while (true) {
        if (LeftOf (Org(bti), tbi) > 0) {
            tbi = Lnext(tbi);
        } else if (RightOf(Org(tbi), bti) > 0) {
            bti = Rprev(bti);
        } else {
            break;
        }
    }

    Edge *base = Connect(Sym(bti), tbi);
    Edge *first_base = base;

    do {
        Edge *tcand = Onext(Sym(base));
        if (Valid(tcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(tcand), Dest(Onext(tcand))) > 0) {
                Edge *t = Onext(tcand); 
                DeleteEdge(tcand); 
                tcand = t;
            }
        }
        Edge *bcand = Oprev(base); 
        if (Valid(bcand, base)) {
            while (InCircle(Dest(base), Org(base), Dest(bcand), Dest(Oprev(bcand))) > 0) {
                Edge *t = Oprev(bcand); DeleteEdge(bcand); bcand = t;
            }
        }
        if (!Valid(tcand, base) && !Valid(bcand, base)) { 
            break;
        }
        if (!Valid(tcand, base) || (Valid(bcand, base) && InCircle(Dest(tcand), Org(tcand), Org(bcand), Dest(bcand)) > 0)) {
            base = Connect(bcand, Sym(base));
        } else {
            base = Connect(Sym(base), Sym(tcand));
        }
    } while (true);

    return hull_from_seed(first_base);
}

Hull dt(std::vector<pair<REAL, REAL>>& points, bool alternating_cuts, int depth) {
    if (!alternating_cuts) {
        return dt_vertical(points, alternating_cuts, depth);
    }
    if ((depth % 2) == 0) {
        return dt_vertical(points, alternating_cuts, depth);
    }
    return dt_horizontal(points, alternating_cuts, depth);
}


int main(int argc, char **argv) {
    auto program_start = std::chrono::steady_clock::now();

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
        int index; REAL x, y;
        if (!(node_file >> index >> x >> y)) { 
            std::cerr << "Error\n"; 
            return 1; 
        }
        points.push_back({x, y});
        point_to_index[{x, y}] = index;
    }

    auto start = std::chrono::steady_clock::now();
    Hull h = dt(points, alternating_cuts);
    Edge *start_edge = h.ldo;
    auto end = std::chrono::steady_clock::now();
    print("DT computed in ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), " ms");

    std::ofstream out_file(out_path);
    if (!out_file) {
        std::cerr << "Error \n";
        return 1;
    }

    // print(point_to_index);

    queue<Edge*> q;
    set<Edge*> visited;
    vector<tuple<int, int, int>> triangles;
    q.push(start_edge);

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
    }

    auto program_end = std::chrono::steady_clock::now();
    print("Total program time: ", std::chrono::duration_cast<std::chrono::milliseconds>(program_end - program_start).count(), " ms");
    print("Total triangles in triangulation: ", triangles.size()); 
    return 0;
}
