//
// Created by lc06 on 4/14/2023.
//

#ifndef DELAUNAY_DELAUNAY_H
#define DELAUNAY_DELAUNAY_H

#include <string>
#include <math.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>
#include <glm/glm.hpp>

struct Point {
    float id{};
    float x{};
    float y{};
    float z{};

    glm::vec3 normal = {0.f, 0.f, 0.f};

    Point() = default;
    Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Point(float _id, float _x, float _y, float _z) : id(_id), x(_x), y(_y), z(_z) {}
    explicit Point(float* p) : id(p[0]), x(p[1]), y(p[2]), z(p[3]) {}

    Point& operator = (Point& p) = default;

    bool operator == (Point& p) const {
        if(id == p.id) return true;
        else return false;
    }

    bool operator != (Point& p) const {
        return !(*this == p);
    }
};

struct Edge {
    Point p1;
    Point p2;

    Edge() = default;
    Edge(Point _p1, Point _p2) : p1(_p1), p2(_p2) {}

    double length() {
        return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    }

    glm::vec3 getEdgeVector() const {
        return {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    }
};

struct Triangle {
    Edge l1;
    Edge l2;
    Edge l3;

    double a;
    double b;
    double c;

    Triangle() = default;
    Triangle(Edge _l1, Edge _l2, Edge _l3) : l1(_l1), l2(_l2), l3(_l3) {
        a = l1.length();
        b = l2.length();
        c = l3.length();
    }
    Triangle(Point _p1, Point _p2, Point _p3) : l1(Edge(_p1, _p2)), l2(Edge(_p2, _p3)), l3(Edge(_p3, _p1)) {
        a = l1.length();
        b = l2.length();
        c = l3.length();
    }

    bool isReseaonable() const {
        if(a + b <= c || a + c <= b || b + c <= a) return false;
        else return true;
    }

    double area() const {

        double p = (a + b + c) / 2.;
        return sqrt(p * (p-a) * (p-b) * (p-c));
    }

    double outerRadius() const {

        double r = (a * b * c) / (4. * area());
        return r;
    }

    double minAngle() const {

        double angleA = acos((b * b + c * c - a * a) / (2. * b * c));
        double angleB = acos((a * a + c * c - b * b) / (2. * a * c));
        double angleC = acos((a * a + b * b - c * c) / (2. * a * b));

        return angleA < angleB ? (angleA < angleC ? angleA : angleC) : (angleB < angleC ? angleB : angleC);
    }

    glm::vec3 getNormal() {
        glm::vec3 normal = glm::normalize(glm::cross(l1.getEdgeVector(), l2.getEdgeVector()));
        if(normal.z < 0) normal = glm::normalize(glm::cross(l2.getEdgeVector(), l1.getEdgeVector()));

        return normal;
    }

    bool inside(Point p) {

//        double r = outerRadius();
//        double sinA = a / (2. * r);
//        double sinB = b / (2. * r);
//        double sinC = c / (2. * r);
//
//        double cosA = (b * b + c * c - a * a) / (2. * b * c);
//        double cosB = (a * a + c * c - b * b) / (2. * a * c);
//        double cosC = (a * a + b * b - c * c) / (2. * a * b);
//
//        float sin2A = 2. * sinA * cosA;
//        float sin2B = 2. * sinB * cosB;
//        float sin2C = 2. * sinC * cosC;
//
//        glm::vec3 c_o = (sin2A * glm::vec3(l3.p1.x, l3.p1.y, l3.p1.z) + sin2B * glm::vec3(l1.p1.x, l1.p1.y, l1.p1.z) + sin2C * glm::vec3(l2.p1.x, l2.p1.y, l2.p1.z)) / (sin2A + sin2B + sin2C);

        double a[3] = {l1.p1.x, l1.p1.y, l1.p1.z}, b[3] = {l2.p1.x, l2.p1.y, l2.p1.z} , c[3] = {l3.p1.x, l3.p1.y, l3.p1.z}, cumcen[3] = {0., 0., 0.};
        tricircumcenter3d(a, b, c, cumcen, nullptr, nullptr);

        cumcen[0] += a[0];
        cumcen[1] += a[1];
        cumcen[2] += a[2];

        Edge temp(Point(cumcen[0], cumcen[1], cumcen[2]), l3.p1);
        double r_ = temp.length();

        Edge e(Point(cumcen[0], cumcen[1], cumcen[2]), p);
//        if(e.length() < outerRadius()) return true;
        if(e.length() < r_) return true;
        else return false;
    }

    void tricircumcenter3d(double a[3], double b[3], double c[3], double* circumcenter,
                           double *xi, double *eta)
    {
        double xba, yba, zba, xca, yca, zca;
        double balength, calength;
        double xcrossbc, ycrossbc, zcrossbc;
        double denominator;
        double xcirca, ycirca, zcirca;

        /* Use coordinates relative to point `a' of the triangle. */
        xba = b[0] - a[0];
        yba = b[1] - a[1];
        zba = b[2] - a[2];
        xca = c[0] - a[0];
        yca = c[1] - a[1];
        zca = c[2] - a[2];
        /* Squares of lengths of the edges incident to `a'. */
        balength = xba * xba + yba * yba + zba * zba;
        calength = xca * xca + yca * yca + zca * zca;

        /* Cross product of these edges. */
#ifdef EXACT
        /* Use orient2d() from http://www.cs.cmu.edu/~quake/robust.html     */
  /*   to ensure a correctly signed (and reasonably accurate) result, */
  /*   avoiding any possibility of division by zero.                  */

  A[0] = b[1]; A[1] = b[2];
  B[0] = c[1]; B[1] = c[2];
  C[0] = a[1]; C[1] = a[2];
  xcrossbc = orient2d(A, B, C);

  A[0] = c[0]; A[1] = c[2];
  B[0] = b[0]; B[1] = b[2];
  C[0] = a[0]; C[1] = a[2];
  ycrossbc = orient2d(A, B, C);

  A[0] = b[0]; A[1] = b[1];
  B[0] = c[0]; B[1] = c[1];
  C[0] = a[0]; C[1] = a[1];
  zcrossbc = orient2d(A, B, C);

  /*
  xcrossbc = orient2d(b[1], b[2], c[1], c[2], a[1], a[2]);
  ycrossbc = orient2d(b[2], b[0], c[2], c[0], a[2], a[0]);
  zcrossbc = orient2d(b[0], b[1], c[0], c[1], a[0], a[1]);
  */
#else
//        printf( " Warning: IEEE floating points used: Define -DEXACT in makefile \n");
        /* Take your chances with floating-point roundoff. */
        xcrossbc = yba * zca - yca * zba;
        ycrossbc = zba * xca - zca * xba;
        zcrossbc = xba * yca - xca * yba;
#endif

        /* Calculate the denominator of the formulae. */
        denominator = 0.5 / (xcrossbc * xcrossbc + ycrossbc * ycrossbc +
                             zcrossbc * zcrossbc);

        /* Calculate offset (from `a') of circumcenter. */
        xcirca = ((balength * yca - calength * yba) * zcrossbc -
                  (balength * zca - calength * zba) * ycrossbc) * denominator;
        ycirca = ((balength * zca - calength * zba) * xcrossbc -
                  (balength * xca - calength * xba) * zcrossbc) * denominator;
        zcirca = ((balength * xca - calength * xba) * ycrossbc -
                  (balength * yca - calength * yba) * xcrossbc) * denominator;
        circumcenter[0] = xcirca;
        circumcenter[1] = ycirca;
        circumcenter[2] = zcirca;

        if (xi != (double *) nullptr) {
            /* To interpolate a linear function at the circumcenter, define a     */
            /*   coordinate system with a xi-axis directed from `a' to `b' and    */
            /*   an eta-axis directed from `a' to `c'.  The values for xi and eta */
            /*   are computed by Cramer's Rule for solving systems of linear      */
            /*   equations.                                                       */

            /* There are three ways to do this calculation - using xcrossbc, */
            /*   ycrossbc, or zcrossbc.  Choose whichever has the largest    */
            /*   magnitude, to improve stability and avoid division by zero. */
            if (((xcrossbc >= ycrossbc) ^ (-xcrossbc > ycrossbc)) &&
                ((xcrossbc >= zcrossbc) ^ (-xcrossbc > zcrossbc))) {
                *xi = (ycirca * zca - zcirca * yca) / xcrossbc;
                *eta = (zcirca * yba - ycirca * zba) / xcrossbc;
            } else if ((ycrossbc >= zcrossbc) ^ (-ycrossbc > zcrossbc)) {
                *xi = (zcirca * xca - xcirca * zca) / ycrossbc;
                *eta = (xcirca * zba - zcirca * xba) / ycrossbc;
            } else {
                *xi = (xcirca * yca - ycirca * xca) / zcrossbc;
                *eta = (ycirca * xba - xcirca * yba) / zcrossbc;
            }
        }
    }
};

struct Bound {
    float x_min{std::numeric_limits<float>::max()}, x_max{std::numeric_limits<float>::lowest()};
    float y_min{std::numeric_limits<float>::max()}, y_max{std::numeric_limits<float>::lowest()};
    float z_min{std::numeric_limits<float>::max()}, z_max{std::numeric_limits<float>::lowest()};
};

class Delaunay {
private:
    std::vector<Point> points;
    std::vector<float> p_z;
    Bound bound;
    int dim {4};
    void readFile(std::string path);

    int getIndex(int i, int j, int size);
    void getIJ(int index, int& i, int& j, int size);

public:
    Delaunay(): points{} {}
    Delaunay(std::string path): points {} {
        readFile(std::move(path));
    }

    ~Delaunay() { }

    void generateTIN(std::vector<unsigned int>& triIDs);
    void getVertices(std::vector<float>& vertices);
    int getSize() { return points.size() * dim * sizeof(float); };
    int getPointSize() { return points.size(); };
};


#endif //DELAUNAY_DELAUNAY_H
