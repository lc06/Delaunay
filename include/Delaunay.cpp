//
// Created by lc06 on 4/14/2023.
//


#include "Delaunay.h"

void Delaunay::readFile(std::string path) {
    std::fstream fin;
    fin.open(path, std::ios::in);

    if(fin.fail()) {
        std::cout << "read file failed!" << std::endl;
        return;
    }

    std::string line;
    int i = 1;
    while(std::getline(fin, line, '\n')) {

        std::stringstream word(line);
        float p[4];
        word >> p[0] >> p[1] >> p[2] >> p[3];
        p[0] = i++;

        p_z.emplace_back(p[3]);
        p[3] = 1.0f;

        points.emplace_back(p);
    }
    fin.close();

    bound.x_min = std::min_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.x < p2.x;})->x;
    bound.x_max = std::max_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.x < p2.x;})->x;
    bound.y_min = std::min_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.y < p2.y;})->y;
    bound.y_max = std::max_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.y < p2.y;})->y;
    bound.z_min = std::min_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.z < p2.z;})->z;
    bound.z_max = std::max_element(points.begin(), points.end(), [](auto& p1, auto& p2) {return p1.z < p2.z;})->z;

}

int Delaunay::getIndex(int i, int j, int size) {

    if(i > j)
        std::swap(i, j);
    /*
     * [0] 1 2 3 4 5 6
     *   [1] 2 3 4 5 6
     *     [2] 3 4 5 6
     *       [3] 4 5 6
     *         [4] 5 6
     *           [5] 6
     */
    // (1, 2) => 1 * (7 - 1) + (2 - 1) = 7
    // (4, 5) => ((7 - 1) + (7 - 4)) * 4 / 2 + (5 - 4) = 19
    return (2 * size - 1 - i) * i / 2 + (j - i);
}

void Delaunay::getIJ(int index, int& i, int& j, int size) {
    // (0, 1) => ((7 - 1) + (7 - 0)) * 0 / 2 + (1 - 0) = 1
    // (5, 6) => ((7 - 1) + (7 - 5)) * 5 / 2 + (6 - 5) = 21
    index += 1;
    i = index / size * 2;
    j = index - getIndex(i, 0, size);
    if(j >= size ) {
        i += 1;
        j = index - getIndex(i, 0, size);
    }
}

void Delaunay::generateTIN(std::vector<unsigned int>& triIDs) {
    std::stack<Edge> edges;
    std::vector<double> dists;
    std::vector<std::pair<int, int>> indices;

    std::unordered_map<long, bool> existed {};

    int pointsSize = points.size();

    for(int i=0; i < pointsSize; ++i) {
        for(int j=i+1; j < pointsSize; j++) {
            Edge e(points[i], points[j]);
            double dist = e.length();
            dists.push_back(dist);
            indices.emplace_back(i, j);
        }
    }

    int index_ij = std::min_element(dists.begin(), dists.end()) - dists.begin();
    auto [i, j] = indices[index_ij];

    edges.emplace(points[i], points[j]);

    int count = 0;
    while(!edges.empty()) {
        auto edge = edges.top();
        edges.pop();

        auto p1 = edge.p1;
        auto p2 = edge.p2;

        double maxMinAngle = 0.0;
        Point _p;

        int a, b, c;
        long index;
        for(auto&p : points) {
            if(p == p1 || p == p2) continue;
            a = p1.id, b = p2.id, c = p.id;

            if(a > c) std::swap(a, c);
            if(b > c) std::swap(b, c);
            if(a > b) std::swap(a, b);

            index = a * 100000000 + b * 10000 + c;
            if(existed[index]) continue;

            Triangle tri(edge, Edge(p2, p), Edge(p, p1));
            if(!tri.isReseaonable()) continue;

            bool flag = false;
            for(auto& p_: points)
                if(p_ != p1 && p_ != p2 && p_ != p && tri.inside(p_)) {
                    flag = true;
                    break;
                }
            if(flag) continue;

            if(tri.minAngle() > maxMinAngle){
                maxMinAngle = tri.minAngle();
                _p = p;
            }
        }

        a = p1.id, b = p2.id, c = _p.id;

        if(c == 0) continue;

        if(a > c) std::swap(a, c);
        if(b > c) std::swap(b, c);
        if(a > b) std::swap(a, b);

        index = a * 100000000 + b * 10000 + c;
        if(existed[index]) continue;

        existed[index] = true;

        triIDs.push_back(a - 1);
        triIDs.push_back(b - 1);
        triIDs.push_back(c - 1);

        Triangle tri(Point(points[a-1].x, points[a-1].y, p_z[a-1]),
                     Point(points[b-1].x, points[b-1].y, p_z[b-1]),
                     Point(points[c-1].x, points[c-1].y, p_z[c-1]));
        points[a-1].normal += tri.getNormal();
        points[a-1].normal += tri.getNormal();
        points[a-1].normal += tri.getNormal();

        edges.emplace(p1, _p);
        edges.emplace(p2, _p);

        std::cout << count++ << ", " << a << ", " << b << ", " << c << std::endl;
    }
}

void Delaunay::getVertices(std::vector<float>& vertices) {

    for(auto& p : points) {
        vertices.push_back(p.id);
        vertices.push_back((p.x - bound.x_min) / (bound.x_max - bound.x_min) * 2.f - 1.f);
        vertices.push_back((p.y - bound.y_min) / (bound.y_max - bound.y_min) * 2.f - 1.f);
//        vertices.push_back((p.z - bound.z_min) / (bound.z_max - bound.z_min) * 2.f - 1.f);
        vertices.push_back(p.z);

        glm::vec3 pNormal = glm::normalize(p.normal);
        vertices.push_back(pNormal.x);
        vertices.push_back(pNormal.y);
        vertices.push_back(pNormal.z);

    }

}