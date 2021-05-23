#include <iostream>
#include <vector>
#include <memory>
#include "bijection.h"
#include "randomgenerator.h"
#include "tensor.h"
#include "vertex.h"
#include "help_functions.h"
#include "discrete.h"
#include "maze.h"
#include "levenshtein.h"

using graph_int = std::vector<std::vector<int>>;
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");
    string mazename = "nikolskaya.txt";
    //cout << "������� ����� � ����������: ";
    //getline(cin, mazename);

    Maze mz = Construct_Maze("graphs/" + mazename);
    std::cout << "�������� ��������\n";

    // alert_vertexes(mz.GetBjn(), mz.GetGraphList(), mz.GetVertexes());
    // mz.GetTensor().PrintPaths(mz.GetBjn());
    discrete_vector DV;
    vector<string> U = read_file("temp.txt", 5);
    string U_1 = "���������������������������������������������������������";

    // pair<vector<double>, int> answer = learn(mz, 16, U, DV, U_1, 0.05, 20);

    //std::vector<std::vector<double>> grid = gen_grid(
    //    { 0.1, 0.1, 0.1, 0.1 }, { 0.9, 0.9, 0.9, 0.9 }, 0.1);
    //alert(grid);
    // we found out that {0.2 , 0.05, 0.65, 0.1} is the most optimal vector(= answer)
    // but it takes long to search, so let' compare atirifcally
    vector<double> best{ 0.2, 0.05, 0.65, 0.1 };
    compare_real_learned(mz, 16, U, DV, U_1, 0.05, 20, make_pair(best, 6));
    return 0;
}

// ��� ������������������ �� ������ �������� �� ������� ���������
// ������� ������ ��� ����� �� ������� � ������ - ��� ���������� �� � ��
// � ���� ������ ����� �������������� ����� SymTensor
// ����: ��������� ����� ������������������ ��� ��������� ������
