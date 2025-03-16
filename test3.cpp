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

class Klass{
public:
    int id;
    vector<Student*> students;
    map<string, map<string, float> > info;
    void getNum(const string course, string field = "与考人数"){
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        int c = 0;
        for(auto &s: students){
            if(s->has(course)) c+=1;
        }
        info[course][field] = c;
    }
    void getMax(string course, string field = "最高分"){
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        float c = 0;
        for(auto &s: students){
            if(s->has(course) && s->get(course) > c) c = s->get(course);
        }
        if(c!=0) info[course][field] = c;
    }
    void getMin(string course, string field = "最低分"){
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        float c = 1000;
        for(auto &s: students){
            if(s->has(course) && s->get(course) < c) c = s->get(course);
        }
        if(c != 1000) info[course][field] = c;
    }
    void rangeCount(string course, int a, int b){
        string field = to_string(a) +"-" + to_string(b);
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        int c = 0;
        for(auto &s: students){
            if(s->has(course)){
                float score = s->get(course);
                if(score >= a && score <b) c += 1;
            }
        }
        info[course][field] = c;
    }
    void getAvg(string course, string field = "平均分"){
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        int c = 0;
        float _sum = 0;
        for(auto &s: students){
            if(s->has(course)){
                c += 1;
                _sum += s->get(course);
            }
        }
        info[course][field] = _sum/c;
    }
};

void getRank(vector<Student>& students, string course, string key){
    vector<Student *> _students;
    for(auto &s : students){
        if(s.has(course)) _students.push_back(&s);
    }
    stable_sort(_students.begin(), _students.end(), [&](Student* a, Student* b) {
        return a->get(course) > b->get(course);
    });
    for(int i = 0; i < _students.size(); i++){
        if(i > 0 && _students[i]->info[course] == _students[i - 1]->info[course]){
            _students[i]->info[key] = _students[i -1]->info[key];
        }
        else
            _students[i]->info[key] = i + 1;
    }
}
void load(OpenXLSX::XLWorksheet wks, vector<Student> & students, vector<Klass> &klass){
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
    map<int, int> klassIds;
    for(auto &s: students){
        s.info["语数英"] = s.get("语文") +s.get("数学") + s.get("英语");
        s.info["副科"] = s.get("化学") + s.get("地理") + s.get("政治")+ s.get("生物");
        if(s.has("历史")){
            s.info["历史总分"] = s.get("语数英") + s.get("副科") + s.get("历史");
        }
        if(s.has("物理")){
            s.info["物理总分"] = s.get("语数英") + s.get("副科") + s.get("物理");
        }
        int id = s.get("班级");
        if(klassIds.find(id) == klassIds.end()){
            klassIds[id] = klassIds.size();
            klass.push_back({id});
        }
        klass[klassIds[id]].students.push_back(&s);
    }
    for(auto& k: klass){
        cout << k.id << " " << k.students.size() << endl;
    }
    vector<string> courses = {"语文","数学","英语",
                            "物理","历史",
                            "化学","地理","政治", "生物",
                            "物理总分", "历史总分"};
    vector<string> keys = {"语排","数排","英排",
                            "物排","历排",
                            "化排","地排","政排", "生排",
                            "物类排", "历类排"};
    for(int i = 0 ; i< courses.size(); i++){
        getRank(students, courses[i], keys[i]);
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

void Sheet1(OpenXLSX::XLWorksheet wks, vector<Student>& studens, int startRow = 1){
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
    
    int row = startRow;
    int col = 1;
    wks.cell(startRow, 2) = "班级";
    for(int j = 0; j < m.size(); j++){
        wks.cell(startRow, j + 3) = _idx[j];
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
OpenXLSX::XLWorksheet getSheet(XLDocument &doc, string name){
    auto workbook = doc.workbook();
    if(!workbook.sheetExists(name)){
        workbook.addWorksheet(name);
    }
    auto wks = doc.workbook().worksheet(name);
    return wks;
}

                
void Sheet2(OpenXLSX::XLWorksheet wks, vector<Student>& _students, int startRow = 1){
    vector<Student> students;
    for(auto &s: _students) {
        if(s.has("物理")){
            students.push_back(s);
        }
    }
    vector<string> prefix = {"序号","班级","座号","班座","姓名"};
    vector<string> courses = {"语文","数学","英语",
                            "物理",
                            "化学","地理","政治", "生物",
                            "物理总分"};
    vector<string> keys = {"语排","数排","英排",
                            "物排",
                            "化排","地排","政排", "生排",
                            "物类排"};
    for(int j = 0; j < prefix.size(); j ++){
        wks.cell(startRow, j + 1) = prefix[j];
    }
    for(int j = 0; j < courses.size(); j ++){
        wks.cell(startRow, 2 * j + 1 + prefix.size()) = courses[j];
        wks.cell(startRow, 2 * j + 2 + prefix.size()) = keys[j];
    }
    for(int i = 0 ; i<courses.size(); i++){
        getRank(students, courses[i], keys[i]);
    }
    stable_sort(students.begin(), students.end(), [&](Student a, Student b) {
        return a.get("物理总分") > b.get("物理总分");
    });
    for(int i = 0; i < students.size(); i++){
        auto & s = students[i];
        for(int j = 0; j < prefix.size(); j ++){
            if(prefix[j] == "姓名" || prefix[j] == "班座"){
                wks.cell(i + 1 + startRow, j + 1) = s.getStr(prefix[j]);
            }
            else
                wks.cell(i + 1 + startRow, j + 1) = s.get(prefix[j]);
        }
        for(int j = 0; j < courses.size(); j ++){
            if(s.has(courses[j])){
                wks.cell(i + 1 + startRow, 2 * j + 1 + prefix.size()) = s.get(courses[j]);
                wks.cell(i + 1 + startRow, 2 * j + 2 + prefix.size()) = s.get(keys[j]);
            }
        }
    }
}
void Sheet3(OpenXLSX::XLWorksheet wks, vector<Student>& _students, int startRow = 1){
    vector<Student> students;
    for(auto &s: _students) {
        if(s.has("历史")){
            students.push_back(s);
        }
    }
    vector<string> prefix = {"序号","班级","座号","班座","姓名"};
    vector<string> courses = {"语文","数学","英语",
                            "历史",
                            "化学","地理","政治", "生物",
                            "历史总分"};
    vector<string> keys = {"语排","数排","英排",
                            "历排",
                            "化排","地排","政排", "生排",
                            "历类排"};
    for(int j = 0; j < prefix.size(); j ++){
        wks.cell(startRow, j + 1) = prefix[j];
    }
    for(int j = 0; j < courses.size(); j ++){
        wks.cell(startRow, 2 * j + 1 + prefix.size()) = courses[j];
        wks.cell(startRow, 2 * j + 2 + prefix.size()) = keys[j];
    }
    for(int i = 0 ; i<courses.size(); i++){
        getRank(students, courses[i], keys[i]);
    }
    stable_sort(students.begin(), students.end(), [&](Student a, Student b) {
        return a.get("历史总分") > b.get("历史总分");
    });
    for(int i = 0; i < students.size(); i++){
        auto & s = students[i];
        for(int j = 0; j < prefix.size(); j ++){
            if(prefix[j] == "姓名" || prefix[j] == "班座"){
                wks.cell(i + 1 + startRow, j + 1) = s.getStr(prefix[j]);
            }
            else
                wks.cell(i + 1 + startRow, j + 1) = s.get(prefix[j]);
        }
        for(int j = 0; j < courses.size(); j ++){
            if(s.has(courses[j])){
                wks.cell(i + 1 + startRow, 2 * j + 1 + prefix.size()) = s.get(courses[j]);
                wks.cell(i + 1 + startRow, 2 * j + 2 + prefix.size()) = s.get(keys[j]);
            }
        }
    }
}

void _Sheet4(OpenXLSX::XLWorksheet wks, vector<Student>& _students, int& startRow,
    vector<int>&pivot, string key, set<int> &klass){
    vector<Student> students;
    for(auto&s : _students){
        if(klass.find(int(s.get("班级")))!= klass.end()){
            students.push_back(s);
        }
    }

    map<int, int> m;
    vector<int> _idx;
    for(auto &s: students){
        if(s.has("班级")){
            int id = s.get("班级");
            if(m.find(id) == m.end()){
                _idx.push_back(id);
                m[id] = m.size();
            }
        }
    }
    for(int i = 0; i < m.size(); i ++){
        int classID = _idx[i];
        vector<int> counts(pivot.size() + 1, 0);
        for(auto &s: students){
            if(int(s.get("班级")) != classID || !s.has(key)) continue;
            float score = s.get(key);
            for(int j = 0; j < pivot.size(); j ++){
                if(j == 0 && score >= pivot[0]) counts[0] += 1;
                else if(j == pivot.size()-1 && score < pivot[pivot.size() - 1]) counts[pivot.size()] += 1;
                else if(score < pivot[j] && score >= pivot[j + 1]){
                    counts[j + 1] += 1;
                }
            }
        }
        for(int j = 0; j < counts.size(); j ++ ){
            wks.cell(i + startRow, j + 2) = counts[j];
        }
    }
    for(int i = 0; i < m.size(); i ++){
        wks.cell(i + startRow, 1) = _idx[i];
    }
    startRow += m.size();
}
void Sheet4(OpenXLSX::XLWorksheet wks, vector<Student>& students, int& startRow){
    vector<int> pivot1 = {650, 640};
    for(int i = 1; ;i++){
        if(pivot1[i] > 400){
            pivot1.push_back(pivot1[i] - 20);
        }
        else break;
    }
    wks.cell(startRow, 1) = "班级";
    for(int i = 0; i < pivot1.size(); i++){
        string field;
        if(i == 0) field = to_string(pivot1[i]) + "以上";
        else{
            field = to_string(pivot1[i]) + "-" + to_string(pivot1[i - 1] - 1);
        }
        wks.cell(startRow, i + 2) = field;
    }
    wks.cell(startRow, pivot1.size() + 2) = to_string(pivot1[pivot1.size() - 1]) + "以下";
    string key1 = "物理总分";
    set<int> klass1;
    for(int i = 0; i<=13; i ++) klass1.insert(i);
    startRow += 1;
    _Sheet4(wks, students, startRow, pivot1, "物理总分", klass1);
    set<int> klass2 = {14,15,16,17,18};
    _Sheet4(wks, students, startRow, pivot1, "历史总分", klass2);
    startRow += 1;
}
void _Sheet5(OpenXLSX::XLWorksheet wks, vector<Student>& _students, int& startRow,
    vector<int>&pivot, string key, set<int> &klass){
    vector<Student> students;
    for(auto&s : _students){
        if(klass.find(int(s.get("班级")))!= klass.end()){
            students.push_back(s);
        }
    }

    map<int, int> m;
    vector<int> _idx;
    for(auto &s: students){
        if(s.has("班级")){
            int id = s.get("班级");
            if(m.find(id) == m.end()){
                _idx.push_back(id);
                m[id] = m.size();
            }
        }
    }
    for(int i = 0; i < m.size(); i ++){
        int classID = _idx[i];
        vector<int> counts(pivot.size() - 1, 0);
        for(auto &s: students){
            if(int(s.get("班级")) != classID || !s.has(key)) continue;
            float score = s.get(key);
            for(int j = 0; j < pivot.size() - 1; j ++){
                if(score > pivot[j] && score <= pivot[j + 1]){
                    counts[j] += 1;
                }
            }
        }
        for(int j = 0; j < counts.size(); j ++ ){
            wks.cell(i + startRow, j + 1) = counts[j];
        }
    }
    for(int i = 0; i < m.size(); i ++){
        wks.cell(i + startRow, 1) = _idx[i];
    }
    startRow += m.size();
}

void Sheet5(OpenXLSX::XLWorksheet wks, vector<Student>& students, int& startRow){
    vector<int> pivot1 = {0, 20, 50, 100, 150, 200, 250, 300, 350, 400, 500, 600, 700};
    vector<int> pivot2 = {0, 20, 50, 100, 150, 200, 250};
    set<int> klass1;
    for(int i = 0; i<=13; i ++) klass1.insert(i);
    set<int> klass2 = {14,15,16,17,18};

    wks.cell(startRow, 1) = "班级";
    for(int i = 0; i < pivot1.size() - 1; i++){
        string field;
        field = to_string(pivot1[i] + 1) + "-" + to_string(pivot1[i + 1] );
        wks.cell(startRow, i + 2) = field;
    }
    startRow += 1;
    _Sheet5(wks, students, startRow, pivot1, "物类排", klass1);
    _Sheet5(wks, students, startRow, pivot2, "历类排", klass2);
}

void Sheet6(OpenXLSX::XLWorksheet wks, vector<Student>& students, vector<Klass> &klass, int& startRow){
    string course = "语文";
    int col = 1;
    for(auto& k: klass){
        k.getNum(course);
        k.getAvg(course);
        k.getMax(course);
        k.getMin(course);
        vector<int> pivot = {151, 135, 120, 105, 90, 75, 0};
        for(int i =0; i< pivot.size() - 1; i ++){
            k.rangeCount(course, pivot[i + 1], pivot[i]);
        }
        auto r = startRow;
        wks.cell(r++, col) = k.info[course]["与考人数"];
        wks.cell(r++, col) = formatFloat(k.info[course]["最高分"]);
        wks.cell(r++, col) = formatFloat(k.info[course]["最低分"]);
        wks.cell(r++, col) = formatFloat(k.info[course]["平均分"]);
        for(int i =0; i< pivot.size() - 1; i ++){
            string t = to_string(pivot[i+1]) + "-" + to_string(pivot[i]);
            wks.cell(r++, col) = k.info[course][t];
        }
        col += 1;
        
    }
}
int main(){
    XLDocument doc;
    doc.open("tables/grade2.xlsx");
    auto workbook = doc.workbook();
    auto wks = workbook.worksheet("原始分");
    
    XLDocument doc2;
    doc2.open("tables/result.xlsx");
    auto workbook2 = doc2.workbook();
    if(!workbook2.sheetExists("Sheet1")){
        workbook2.addWorksheet("Sheet1");
    }
    vector<Student> students;
    vector<Klass> klass;
    load(wks, students, klass);
    // for(auto s: students){
    //     cout << s.get("班级") << " ";
    // }
    //cout << students.size();
    // auto wks2 = getSheet(doc2, "Sheet1");
    // Sheet1(wks2, students);
    // auto wks3 = getSheet(doc2, "Sheet2");
    // Sheet2(wks3, students, 1);
    // auto wks4 = getSheet(doc2, "Sheet3");
    // Sheet3(wks4, students, 1);

    //  auto wks5 = getSheet(doc2, "Sheet4");
    //  int row = 1;
    //  Sheet4(wks5, students, row);
    //  Sheet5(wks5, students, row);
    
    auto wks6 = getSheet(doc2, "Sheet6");
    int row = 1;
    Sheet6(wks6, students, klass, row);


    
    doc2.save();
    doc2.close();
}