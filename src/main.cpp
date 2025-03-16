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

class Lecture{
public:
    string name;
    vector<float> pivots = {1000};
    float pass;
    float avg = 0;
    float _max = 0;
    float _min = 150;
    float num = 0;
    void getPivots(vector<Student>& students){
        vector<float> scores;
        float _sum = 0;
        for(auto&s:students){
            if(s.has(name)){
                float score = s.get(name);
                if(score > _max) _max = score;
                if(score < _min) _min = score;
                _sum += score;
                scores.push_back(score);
            }
        }
        int c = scores.size();
        num = c;
        if(c > 0) avg = _sum/c;
        sort(scores.begin(), scores.end(), std::greater<int>());
        for(int i = 1; i < 10; i ++){
            pivots.push_back(scores[int(i * 0.1 * c)]);
        }
        pivots.push_back(0);

        set<string> s1 = {"语文", "数学", "英语"};
        if(s1.find(name) != s1.end()) pass = 90;
        else if(name == "历史总分" || name == "物理总分") pass = 450;
        else if(name == "语数英") pass = 270;
        else pass = 60;
    }
};
class Klass{
public:
    int id;
    vector<Student*> students;
    map<string, map<string, float> > info;
    bool has(string course){
        return students.size() > 0 &&
               students[0]->has(course);
    }
    bool has(string course, string field){
        return info.find(course) != info.end() 
        && info[course].find(field)!= info[course].end();
    }
    void setInfo(string course, string field, float s){
        if(info.find(course) == info.end()){
            info[course] = map<string, float>();
        }
        info[course][field] = s;
    }
    void getNum(const string course, string field = "与考人数"){
        int c = 0;
        for(auto &s: students){
            if(s->has(course)) c+=1;
        }
        if(c!=0) setInfo(course, field, c);
    }
    void getMax(string course, string field = "最高分"){
        float c = 0;
        for(auto &s: students){
            if(s->has(course) && s->get(course) > c) c = s->get(course);
        }
        if(c!=0) setInfo(course, field, c);
    }
    void getMin(string course, string field = "最低分"){
        float c = 1000;
        for(auto &s: students){
            if(s->has(course) && s->get(course) < c) c = s->get(course);
        }
        if(c!=1000) setInfo(course, field, c);
    }
    void rangeCount(string course, int a, int b, string field = "", bool rate = false){
        if(field.size() == 0) field = to_string(a) +"-" + to_string(b);
        int c = 0;
        float c2 = 0;
        for(auto &s: students){
            if(s->has(course)){
                c2 += 1;
                float score = s->get(course);
                if(score >= a && score <b) c += 1;
            }
        }
        if(!rate) setInfo(course, field, c);
        else setInfo(course, field, c/c2);
        //cout << field<< ' ' << c << endl;
    }
    void getAvg(string course, string field = "平均分"){
        int c = 0;
        float _sum = 0;
        for(auto &s: students){
            if(s->has(course)){
                c += 1;
                _sum += s->get(course);
            }
        }
        if(c > 0) setInfo(course, field, _sum/c);
    }

    void prepare(map<string, Lecture> &lecture){
        vector<string> courses = {"语文","数学","英语",
                            "物理","历史",
                            "化学","地理","政治", "生物","语数英",
                            "历史总分", "物理总分"};
        for(auto &course: courses){
            getNum(course);
            getMax(course);
            getMin(course);
            getAvg(course);
            float t = info[course]["平均分"] - lecture[course].avg;
            setInfo(course, "均分差", t);
            auto pivot = lecture[course].pivots;
            rangeCount(course, pivot[1], 1000, "前10%");
            for(int i = 1; i <= 5; i++){
                string filed = to_string(i * 10) + "% - "+ to_string(i* 10 + 10) + "%";
                rangeCount(course, pivot[i + 1], pivot[i], filed);
            }
            rangeCount(course, 0, pivot[6], "后60%");
            rangeCount(course, lecture[course].pass, 1000, "及格率", true);
            rangeCount(course, pivot[1], 1000, "优秀率", true);
        }
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
void assignScore(vector<Student>& _students, string course){
    vector<Student*> students;
    for(auto &s: _students){
        if(s.has(course)) students.push_back(&s);
    }
    vector<vector<float> > area= {{100, 86}, {85, 71}, {70,56}, {55, 41}, {40, 30}};
    vector<vector<float> > origin(area.size(), {-1, 1000});
    vector<float> p = {0, 0.15, 0.5, 0.85, 0.98, 1.1};
    stable_sort(students.begin(), students.end(), 
        [&](Student *a, Student *b){
            return a->get(course) > b->get(course);
        });
    int i = 0, n = students.size();
    int j = 0;
    for(auto &s: students){
        float t = float(i)/n;
        while(!(t>= p[j] && t < p[j + 1])){j += 1;}

        float score = s->get(course);
        if(score > origin[j][0]) origin[j][0] = score;
        if(score < origin[j][1]) origin[j][1] = score;
        i += 1;
    }
    for(auto &s: students){
        float score = s->get(course);
        for(int i = 0; i < origin.size(); i ++){
            if(score >= origin[i][1] && score <= origin[i][0]){
                float a = origin[i][0];
                float b = origin[i][1];
                float m = area[i][0];
                float n = area[i][1];
                s->info[course] = n + (score - b)/(a - b) * (m - n);
                break;
            }
        }
    }
}
void load(OpenXLSX::XLWorksheet wks, vector<Student> & students, 
        map<string, Lecture>& lecture, vector<Klass> &klass, int type = 1){
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
    if(type == 2){
        vector<string> biC = {"化学", "地理", "政治", "生物"};
        for(auto &course: biC) assignScore(students, course);
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
    vector<string> courses = {"语文","数学","英语",
                            "物理","历史",
                            "化学","地理","政治", "生物",
                            "语数英",
                            "物理总分", "历史总分"};
    vector<string> keys = {"语排","数排","英排",
                            "物排","历排",
                            "化排","地排","政排", "生排",
                            "语数英排",
                            "物类排", "历类排"};
    for(int i = 0 ; i< courses.size(); i++){
        getRank(students, courses[i], keys[i]);
    }

    for(auto &course:courses){
        Lecture l;
        l.name = course;
        l.getPivots(students);
        lecture[course] = l;
    }
    for(auto &k: klass){
        k.prepare(lecture);
    }
    for(auto &course: courses){
        vector<vector<float>> rank ;
        for(int i = 0 ;i< klass.size(); i ++){
            if(klass[i].has(course)) rank.push_back({klass[i].info[course]["平均分"], float(i)});
            else{
                rank.push_back({0, float(i)});
            }
        }
        stable_sort(rank.begin(), rank.end(), 
        [](vector<float> a, vector<float> b){
            return a > b;
        });
        int i = 1;
        for(auto r: rank){
            klass[int(r[1])].setInfo(course, "名次", i);
            i += 1;
        }
    }
    
}

float format(float a){
    return round(a * 100)/100;
}
double formatFloat(float f) {
    double a = f;
    return round(a * 100)/100;
}
#include <sstream>
string formatStr(float f) {
    std::ostringstream oss;
    oss << f;  // 默认格式输出，不会固定填充小数位
    return oss.str();
}
OpenXLSX::XLWorksheet getSheet(XLDocument &doc, string name){
    auto workbook = doc.workbook();
    if(!workbook.sheetExists(name)){
        workbook.addWorksheet(name);
    }
    auto wks = doc.workbook().worksheet(name);
    return wks;
}
void Sheet2(OpenXLSX::XLWorksheet wks, vector<Student>& _students, int startRow = 1, int type = 1){
    
    vector<string> prefix = {"序号","班级","座号","班座","姓名"};
    vector<string> courses = {"语文","数学","英语",
                            "物理",
                            "化学","地理","政治", "生物", 
                            "语数英"};
    vector<string> keys = {"语排","数排","英排",
                            "物排",
                            "化排","地排","政排", "生排", 
                            "语数英排"};
    string s1, s2;
    vector<Student> students;
    if(type == 1){
        s1 = "物理总分";
        s2 = "物类排";
        for(auto &s: _students) {
            if(s.has("物理")){
                students.push_back(s);
            }
        }
    }
    else{
        s1 = "历史总分";
        s2 = "历类排";
        courses[3] = "历史";
        keys[3] = "历排";
        for(auto &s: _students) {
            if(s.has("历史")){
                students.push_back(s);
            }
        }
    }

    courses.push_back(s1);
    keys.push_back(s2);

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
        return a.get(s1) > b.get(s1);
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
                wks.cell(i + 1 + startRow, 2 * j + 1 + prefix.size()) = formatFloat(s.get(courses[j]));
                wks.cell(i + 1 + startRow, 2 * j + 2 + prefix.size()) = s.get(keys[j]);
            }
        }
    }
}
void Sheet4(OpenXLSX::XLWorksheet wks, string course, vector<Student>& students, 
        map<string, Lecture>& lecture, vector<Klass> &klass, int& startRow){
    vector<string> field = {"与考人数", "平均分", "均分差","名次",
            "最高分", "最低分", "优秀率", "及格率","前10%"};
    auto pivots = lecture[course].pivots;
    for(int i = 1; i <= 5; i++){
        field.push_back(to_string(i * 10) + "% - "+ to_string(i* 10 + 10) + "%");
    }
    field.push_back("后60%");
    int row = startRow;
    int col = 1;
    wks.cell(row, col++) = "班级";
    for(auto &k: klass){
        if(k.has(course))
            wks.cell(row, col++) = k.id;
    }
    row += 1;
    for(auto &f: field){
        int col = 1;
        wks.cell(row, col++) = f;
        for(auto &k : klass){
            if(k.has(course))
                if(f == "及格率" || f == "优秀率"){
                    wks.cell(row, col++) = formatStr(formatFloat(k.info[course][f] * 100)) + "%";
                }
                else
                    wks.cell(row, col++) = formatFloat(k.info[course][f]);
                
        }
        row += 1;
    }
}
void Sheet3(OpenXLSX::XLWorksheet wks, map<string, Lecture>& lecture){
    vector<string> courses = {"语文","数学","英语",
                            "物理","历史",
                            "化学","地理","政治", "生物","语数英",
                            "物理总分", "历史总分"};
    int col = 1;
    vector<string> field = {"课程", "与考人数", "平均分", "最高分", "最低分", "及格线"};
    for(int i = 1; i <=9; i++) field.push_back(to_string(i*10));
    int row = 1;
    for(auto &f: field) wks.cell(row++, col) = f;
    col += 1;
    for(auto &course: courses){
        int row = 1;
        wks.cell(row ++, col) = course;
        auto& l = lecture[course];
        vector<float> rs = {l.num, l.avg, l._max, l._min, l.pass};
        for(int i = 1; i <= 9; i ++) rs.push_back(l.pivots[i]);
        for(auto &r: rs) wks.cell(row++, col) = formatFloat(r);
        col += 1;
    }

}
void Sheet1(OpenXLSX::XLWorksheet wks, vector<Student>& students){
    vector<string> prefix = {"序号","班级","座号","班座","姓名","语文","数学","英语",
                            "物理",
                            "化学","地理","政治", "生物", 
                            "语数英", "总分"};
    int startRow = 1;
    for(int j = 0; j < prefix.size(); j ++){
        wks.cell(startRow, j + 1) = prefix[j];
    }

    for(int i = 0; i < students.size(); i++){
        auto & s = students[i];
        for(int j = 0; j < prefix.size(); j ++){
            if(prefix[j] == "姓名" || prefix[j] == "班座"){
                wks.cell(i + 1 + startRow, j + 1) = s.getStr(prefix[j]);
            }
            else if(students[i].has(prefix[j]))
                wks.cell(i + 1 + startRow, j + 1) = formatFloat(s.get(prefix[j]));
        }
    }
}

void Document(string srcPath, string sheet,string targetPath, int type = 1){
    XLDocument doc;

    doc.open(srcPath);
    auto workbook = doc.workbook();
    auto wks = workbook.worksheet(sheet);
    
    XLDocument doc2;
    doc2.create(targetPath);
    doc2.open(targetPath);
    auto workbook2 = doc2.workbook();

    vector<Student> students;
    vector<Klass> klass;
    map<string, Lecture> lecture;
    
    load(wks, students, lecture, klass, type);
    vector<string> courses = {"语文","数学","英语",
                            "物理","历史",
                            "化学","地理","政治", "生物","语数英",
                            "物理总分", "历史总分"};
    auto wks1 = getSheet(doc2, "Sheet1");
    Sheet1(wks1, students);
    auto wks2 = getSheet(doc2, "物理类排名");
    Sheet2(wks2, students, 1, 1);
    auto wks3 = getSheet(doc2, "历史类排名");
    Sheet2(wks3, students, 1, 2);
    auto wks4 = getSheet(doc2, "各科总揽");
    Sheet3(wks4, lecture);
    for(auto &course: courses){
        auto wks5 = getSheet(doc2, course);
        int row = 1;
        Sheet4(wks5, course, students, lecture,  klass, row);
    }
    doc2.save();
    doc2.close();
}
int main(){
    Document("grade.xlsx", "原始分", "result1.xlsx", 1);
    Document("grade.xlsx", "原始分", "result2.xlsx", 2);
    
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

    //auto wks7 = getSheet(doc2, "sheet7");
    //Sheet7(wks7, lecture);
}