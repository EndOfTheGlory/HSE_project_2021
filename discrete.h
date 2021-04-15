#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include "help_functions.h"
#include "path_algo.h"
#include "maze.h"

extern RandomGenerator gen;

std::vector<int> subseq(std::vector<int>& v, size_t a, size_t b) {
    std::vector<int> subseq;
    subseq.reserve(6);
    for (size_t i = a; i < b; ++i) {
        subseq.push_back(v[i]);
    }

    return subseq;
}

class DiscreteOperator {
protected:
    double probability = .0;

public:
    DiscreteOperator() {
        probability = .0;
    }

    DiscreteOperator(double prob) {
        probability = prob;
    }

    const double GetProb() const { return probability; }
    // void SetProb(double prob) { probability = prob; }

    virtual void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const {}
    virtual void name() const {}
};

class Inverse : public DiscreteOperator {
public:
    Inverse(double prob) : DiscreteOperator(prob) {};

    void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const override {
        if (probability == .0) return;
        if (subseq_size > v.size()) { return; }

        size_t fixed_size = v.size();
        for (size_t i = 1; i < subseq_size; ++i) {
            v.push_back(v[fixed_size - i - 1]);
        }
    }

    void name() const override { std::cout << "I'm an Inverse!\n"; }
};

class Symmetry : public DiscreteOperator {
public:

    Symmetry(double prob) : DiscreteOperator(prob) {}

        /*
        1. ��������� ������������������ ���������� � ���� "LRUP"
        2. ������������ ������������ ������ �����
        3. ������ �������, ������������ ���������
        4. �������� ����� ����������������� :
            ���� ������ - ������������, L �������� �� R � ��������
            ���� ������ - ��������������, U �������� �� D � ��������
        5. �� ���������� ������������������ ��������������� ������������ ����,
            ���� ������� �� ������ 3.
        */
    void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const override {
        if (probability == .0) return;
        // std::vector<int> subv = subseq(v, v.size() - 1 - subseq_size, v.size());
        std::vector<int> subv = v;
        Directions DIRS = mz.GetDirections();
        auto str_v = DIRS.convert_to_dirs(subv);  // ����� 1
        int first_vert = subv.front();

        // ������������� ������ ������������ �������
        // �� ���� ������� ��������� ���������� ��� �������������,
        // ��� ��� ��� ������������� ����� ����������� ��������� �����
        // ����� ������������ � �������������� ��������
        std::string type = "vertical";

        // ���� ������������ �������
        // ������� ������� �����, ��������� �� ����� ������
        // � �� ����������� ���������� �����
        auto gph = mz.GetGraphList();

        // ���� ������������� �������������� ������,
        // �� LR ������ �� UD
        int tmp = first_vert;
        int count_left = 0;
        while (true) {
            size_t i = 0;
            while (i < gph[tmp].size() && 
                   DIRS.DirectionBetween(tmp, gph[tmp][i]) != "L") {
                ++i;
            }
            if (i < gph[tmp].size()) {
                ++count_left;
                tmp = gph[tmp][i];
            } else { break; }
        }

        tmp = first_vert;
        while (true) {
            size_t i = 0;
            while (i < gph[tmp].size() &&
                DIRS.DirectionBetween(tmp, gph[tmp][i]) != "R") {
                ++i;
            }
            if (i < gph[tmp].size()) {
                //std::cout << mz.GetBjn()[gph[tmp][i]] << '\n';
                tmp = gph[tmp][i];
            }
            else { break; }
        }

        int new_vert = tmp; // ���������� ������� �� 2-3
        for (size_t j = 0; j < count_left; ++j) {
            for (size_t i = 0; i < gph[new_vert].size(); ++i) {
                if (DIRS.DirectionBetween(new_vert, gph[new_vert][i]) == "L") {
                    new_vert = gph[new_vert][i];
                }
            }
        }

        // �� ����������� ���� ������� � new_vert
        std::vector<int> tmp_v = mz.GetTensor()(subv.back(), new_vert);
        for (size_t i = 0; i < tmp_v.size() - 1; ++i) { v.push_back(tmp_v[i]); }
        inverse_dirs(str_v, type);  // ����� 4
        subv = DIRS.convert_to_v(str_v, new_vert); // ����� 5
        for (size_t i = 0; i < subv.size(); ++i) { v.push_back(subv[i]); }
    }

    void name() const override { std::cout << "I'm a Symmetry!\n"; }
};

class Grip : public DiscreteOperator {
public:

    Grip(double prob) : DiscreteOperator(prob) {};

    void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const override {
        if (probability == .0) return;
        if (subseq_size > v.size()) { return; }
        Symmetric_Tensor3D ST = mz.GetTensor();

        size_t from = v[v.size() - subseq_size];
        size_t to = *(--v.end());

        for (size_t i = 0; i < subseq_size - 1; ++i) {
            v.pop_back();
        }


        std::vector<int> u = ST(from, to);
        for (size_t i = 0; i < u.size(); ++i) {
            v.push_back(u[i]);
        }
    }

    void name() const override { std::cout << "I'm a Grip!\n"; }
};

class Ring : public DiscreteOperator {
public:

    Ring(double prob) : DiscreteOperator(prob) {};

    void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const override {
        if (probability == .0) return;

        auto backup = v;
        std::vector<Vertex> VS = mz.GetVertexes();
        int current_v = *(--v.end());
        // if (!VS[current_v].bio_value) { return; }

        graph_int gph = mz.GetGraphList();
        auto fixed_v = current_v;
        // ������� ���� �� ������-������ ��������� ��������
        while (true) {
            int u = random_choice(gph[current_v]);
            current_v = gph[current_v][u];
            v.push_back(current_v);

            if (VS[current_v].bio_value) { 
                break;
            }
        }

        // ���� ������� ���������� �����
        // ��� ��� ����� ��������, �� ���� ������ ����� �� ��������
        auto path = mz.GetTensor()(current_v, fixed_v);
        for (size_t i = 1; i < path.size(); ++i) {
            v.push_back(path[i]);
        }

        // ���� ���� ������� �������
        if (v.size() > 3 * gph.size() / 2) {
            v = backup;
            backup.clear();
            path.clear();
            gph.clear();
            VS.clear();
            method(mz, v, subseq_size);
        }
    }

    void name() const override { std::cout << "I'm a Ring!\n"; }
};

class Empty : public DiscreteOperator {
public:

    Empty(double prob) : DiscreteOperator(prob) {};
    void method(const Maze& mz, std::vector<int>& v, size_t subseq_size) const override {
        return;
    }

    void name() const override { std::cout << "I'm an Empty!\n"; }
};

class discrete_vector {
private:
    std::vector<std::unique_ptr<DiscreteOperator>> DOs;
    double enable_prob; // ����������� ����, ��� �� ���� �� �� ����� ��������

public:
    discrete_vector() {
        DOs.reserve(5);
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Inverse(.0)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Symmetry(.0)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Grip(.0)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Ring(.0)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Empty(1.)));
    }
    discrete_vector(double p1, double p2, double p3, double p4) {
        try {
            if ((p1 + p2 + p3 + p4) > 1) {
                throw std::invalid_argument("�������� ����������� �� ������������� �����������");
            }
        }
        catch (std::invalid_argument& e) {
            std::cerr << e.what() << '\n';
            std::cerr << "����� �������� ���� ������������ �� ������ ��������� �������\n";
            exit(1);
        }

        DOs.reserve(5);
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Inverse(p1)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Symmetry(p2)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Grip(p3)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Ring(p4)));
        DOs.push_back(std::unique_ptr<DiscreteOperator>(new Empty(1. - (p1 + p2 + p3 + p4))));

        std::sort(DOs.begin(), DOs.end(),
            [](const std::unique_ptr<DiscreteOperator>& one,
                const std::unique_ptr<DiscreteOperator>& two) -> bool {
                return (one->GetProb() < two->GetProb());
            }
        );
    }

    const std::unique_ptr<DiscreteOperator>& operator[] (size_t j) const { return DOs[j]; }
    
    // https://coderoad.ru/17250568/���������-�����-��-������-��-�����������-�������������
    size_t rand_choice() const {
        double rnd = gen.d_udist(0, 1);
        size_t i = 0;
        for (size_t i = 0; i < 5; ++i) {
            rnd -= DOs[i]->GetProb();
            if (rnd <= .0) return i;
        }

        return 4;
    }
};

std::vector<char> gen_discrete_path(const Maze& mz,
        const discrete_vector& DV,
            const std::vector<int>& seq) {

    std::vector<int> new_seq;
    new_seq.reserve(seq.size());
    for (size_t i = 0; i < seq.size() - 1; ++i) {
        new_seq.push_back(seq[i]);
        if (new_seq.size() < 3) { continue; }
        size_t k = DV.rand_choice();
        // ���� ��������, ��� ����� ��������������������� ����� ������������
        // ���� �� ��������������, ����� ������� ���:
        size_t LENGHT = std::min(int(new_seq.size()), gen.int_udist(2, 6));

        DV[k]->method(mz, new_seq, LENGHT);

        // ��������� �� ��������� ���������� �������
        // ���� ���� ����� � i+1-��, �� ��
        // ����� ��� ���� ������ � i+1, 
        // ���������� �� ����������� ����
        // � �����, ����� ��� � ������� � ��� �������� ������������� �����
        std::vector<int> bridge = mz.GetTensor()(new_seq.back(), seq[i + 1]);
        
        if (bridge.size() == 0) { continue; }
        for (size_t j = 0; j < bridge.size() - 1; ++j) {
            new_seq.push_back(bridge[j]);
        }
    }

    new_seq.push_back(seq.back());
    return int_to_char(mz.GetBjn(), new_seq);
}

void gen_paths_file(const std::string& filename, const Maze& mz,
    const discrete_vector& DOs, size_t trials, char entry) {

    std::ofstream file(filename, std::ios_base::trunc | std::ios_base::out);
    try {
        if (!file.is_open())
            throw std::runtime_error("�� ������� ������� ����");
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }

    std::vector<int> seq = mz.gen_trivial_path(entry);
    auto tmp = int_to_char(mz.GetBjn(), seq);
    for (auto it = tmp.begin(); it != tmp.end(); ++it) { file << *it; }
    file << '\n';

    for (size_t i = 0; i < trials; ++i) {
        auto v = gen_discrete_path(mz, DOs, seq);
        for (auto it = v.begin(); it != v.end(); ++it) { file << *it; }
        file << '\n';
    }

    file.close();
}