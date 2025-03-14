#include <OpenXLSX.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <cmath>
#include <iomanip> // 用于格式化输出
using namespace std;
using namespace OpenXLSX;

class Student{
public:
    map<string, string> strInfo;
    map<string, float> info;

    bool has(string k){
        return info.find(k)!= info.end() || strInfo.find(k)!=strInfo.end();
    }
    float get(string k){
        if(info.find(k) != info.end()) return info[k];
        return 0;
    }
    string getStr(string k){
        if(strInfo.find(k)!=strInfo.end()) return strInfo[k];
        return "";
    }
};

void load(OpenXLSX::XLWorksheet wks, vector<Student> & students){
    int ncol = wks.columnCount(), nrow = wks.rowCount();
    vector<string> header;
    for (int j = 1; j <= ncol; j++)
        header.push_back(wks.cell(1, j).value());
    for (int i = 2; i <= nrow; i++) {
        Student s;
        for(int j = 1; j <= ncol; j++){  // Fixed the condition here
            auto cell = wks.cell(i, j);
            if (cell.value().type() == XLValueType::Empty) continue;
            auto& k = header[j - 1];

            if(cell.value().type() == XLValueType::String){
                s.strInfo[k] = cell.value().getString();
            }
            else if (cell.value().type() == XLValueType::Float 
                    || cell.value().type() == XLValueType::Integer){
                s.info[k] = cell.value();
            }
        }
        if(s.strInfo.size() > 0) students.push_back(s);
    }
}

float format(float a){
    return round(a * 100)/100;
}
#include <sstream>
string formatFloat(float f) {
    std::ostringstream oss;
    oss << format(f);  // 默认格式输出，不会固定填充小数位
    return oss.str();
}
void Sheet1(OpenXLSX::XLWorksheet wks, vector<Student>& studens){
    vector<string> courses = {"历史总分","物理总分", "语数英", "语文","数学",
                "英语","物理","历史","化学","地理","政治", "生物"};
    map<string, vector<float> > lines;
    lines["语文"] = {90, 120};
    lines["数学"] = {90, 120};
    lines["英语"] = {90, 120};
    lines["英语"] = {90, 120};
    lines["物理"] = {60, 80};
    lines["历史"] = {60, 80};
    lines["化学"] = {40, 60};
    lines["地理"] = {40, 60};
    lines["政治"] = {40, 60};
    lines["生物"] = {40, 60};
    map<int, int> m;
    vector<int> _idx;
    for(auto &s: studens){
        if(s.has("班级")){
            int id = s.get("班级");
            if(m.find(id) == m.end()){
                _idx.push_back(id);
                m[id] = m.size();
            }
        }
    }
    
    int row = 1;
    int col = 1;
    wks.cell(1, 2) = "班级";
    for(int j = 0; j < m.size(); j++){
        wks.cell(1, j + 3) = _idx[j];
    }
    row += 2;
    for(auto &course: courses){
        map<string, float> statis;
        float _min = 200, _max = 0;
        float sum = 0, count = 0, avg = 0;
        for(auto &s: studens){
            if(s.has(course)){
                float score = s.get(course);
                if(score < _min) _min = score;
                if(score > _max) _max = score;
                sum += score;
                count += 1;
            }
        }
        if(count > 0) avg = sum / count;
        vector<vector<float>> klass(m.size(), {0, 0, 0, 0});  // Fixed initialization
        for(auto &s: studens){
            if(s.has(course) && s.has("班级")){
                int id = m[s.get("班级")];
                float score = s.get(course);
                klass[id][0] += score;
                klass[id][1] += 1;
                if(lines.find(course) != lines.end()){
                    if(score > lines[course][0]) klass[id][2] += 1;
                    if(score > lines[course][1]) klass[id][3] += 1;
                }
            }
        }
        vector<vector<float>> klassAvg;
        for(int i = 0; i < m.size(); i++){  // Fixed the condition here
            if(klass[i][1] > 0){
                klassAvg.push_back({klass[i][0] / klass[i][1], float(i)});
            }
            else{
                klassAvg.push_back({0, float(i)});
            }
        }
        stable_sort(klassAvg.begin(), klassAvg.end(), 
            [](const vector<float> &d1, const vector<float> &d2){return d1[0] > d2[0]; });

        wks.cell(row, 1) = course;
        wks.cell(row, 2) = "平均分";
        wks.cell(row + 1, 2) = "名次";
        wks.cell(row + 2, 2) = "均分差";
        
        if(lines.find(course) != lines.end()){
            wks.cell(row + 3, 2) = "及格率";
            wks.cell(row + 4, 2) = "优秀率";
        }
        for(int j = 0; j < m.size(); j++){
            if(klass[j][1] > 0){
                wks.cell(row , j + 3) = formatFloat(klass[j][0] / klass[j][1]);
                wks.cell(row + 1, j + 3) = klass[j][2];  // Fixed out-of-bounds error
                wks.cell(row + 2, j + 3) = formatFloat(klass[j][0] / klass[j][1] - avg);

                if(lines.find(course) != lines.end()){
                    wks.cell(row + 3, j + 3) = formatFloat(klass[j][2] * 100 / klass[j][1]) + "%";
                    wks.cell(row + 4, j + 3) = formatFloat(klass[j][3] * 100 / klass[j][1]) + "%";
                }
            }
        }
        row += 4;
        if(lines.find(course) != lines.end()){
            row += 2;
        }
    }
}

int main(){
    XLDocument doc;
    doc.open("tables/grade2.xlsx");
    auto workbook = doc.workbook();
    auto wks = workbook.worksheet("原始分");
    
    XLDocument doc2;
    doc2.open("tables/result.xlsx");
    auto wks2 = doc2.workbook().worksheet("Sheet1");
    vector<Student> students;
    load(wks, students);
    // for(auto s: students){
    //     cout << s.get("班级") << " ";
    // }
    //cout << students.size();
    for(auto &s: students){
        if(s.has("历史")){
            s.info["历史总分"] = s.get("总分");
        }
        if(s.has("物理")){
            s.info["物理总分"] = s.get("总分");
        }
    }
    Sheet1(wks2, students);

    doc2.save();
    doc2.close();
}