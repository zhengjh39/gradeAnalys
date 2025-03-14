#include <OpenXLSX.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
using namespace std;
using namespace OpenXLSX;

class Student {
public:
    int klassID;
    int seatID;
    string name;
    map<string, float> score;
    
    // 当键不存在时，初始化为 -1
    float& operator[](const string &key) {
        if(score.find(key) == score.end())
            score[key] = -1;
        return score[key];
    }
    
    bool hasKey(const string &key) const {
        return score.count(key) > 0;
    }
    
    // 对给定的多个科目求和，并保存到 newKey 对应的成绩中
    void unionScore(const vector<string>& subjects, const string& newKey) {
        float s = 0;
        for (const auto &subject : subjects) {
            if (!hasKey(subject)) return;
            s += score.at(subject);
        }
        score[newKey] = s;
    }
};

class Klass {
public:
    vector<Student*> students;
    
    void push_back(Student* s) {
        students.push_back(s);
    }
    
    void show(const string &key) {
        for (auto s : students)
            cout << (s->hasKey(key) ? (*s)[key] : -1) << ' ';
    }
    
    void unionScore(const vector<string>& subjects, const string& newKey) {
        for (auto s : students)
            s->unionScore(subjects, newKey);
    }
};

class School {
public:
    map<int, Klass> klasses;
    vector<Student*> students;

    void load(XLWorksheet& wks) {
        int ncol = wks.columnCount(), nrow = wks.rowCount();
        vector<string> header{" "};
        for (int j = 1; j <= ncol; j++)
            header.push_back(wks.cell(1, j).value());
        for (int i = 2; i <= nrow; i++) {
            Student* s = new Student;
            for (int j = 1; j <= ncol; j++) {
                auto cell = wks.cell(i, j);
                if (cell.value().type() == XLValueType::Empty) continue;
                const string& colName = header[j];
                if (colName == "班级")
                    s->klassID = cell.value();
                else if (colName == "姓名")
                    s->name = cell.value().getString();
                else if (colName == "座号")
                    s->seatID = cell.value();
                else if (cell.value().type() == XLValueType::Float || cell.value().type() == XLValueType::Integer)
                    (*s)[colName] = cell.value();
            }
            students.push_back(s);
            klasses[s->klassID].push_back(s);
        }
    }
    
    Klass& operator[](int id) {
        return klasses[id];
    }
};

void scoreRank(vector<Student*>& students, const string& key, const string& rankKey) {
    vector<Student*> keyStudents;
    for (auto s : students)
        if (s->hasKey(key))
            keyStudents.push_back(s);
    stable_sort(keyStudents.begin(), keyStudents.end(), [&](Student* a, Student* b) {
        return (*a)[key] > (*b)[key];
    });
    for (size_t i = 0; i < keyStudents.size(); i++)
        keyStudents[i]->score[rankKey] = i + 1;
}

void saveSheetStudent(XLWorksheet& wks, const vector<string>& keys, const vector<Student*>& students) {
    for (size_t i = 0; i < students.size(); i++)
        for (size_t j = 0; j < keys.size(); j++)
            if (students[i]->hasKey(keys[j]))
                wks.cell(i + 1, j + 1) = (*students[i])[keys[j]];
}

vector<Student*> sortStudent(const vector<Student*>& students, const string& key) {
    vector<Student*> keyStudents;
    for (auto s : students)
        if (s->hasKey(key))
            keyStudents.push_back(s);
    stable_sort(keyStudents.begin(), keyStudents.end(), [&](Student* a, Student* b) {
        return (*a)[key] < (*b)[key];
    });
    return keyStudents;
}
#include <sstream>
string formatFloat(float f) {
    std::ostringstream oss;
    oss << f;  // 默认格式输出，不会固定填充小数位
    return oss.str();
}
struct GroupInfo {
    string key;
    string field;
    vector<float> pivot;
    int type;
    
    vector<string> tags;
    
};

struct Group {
    vector<Student*> students;
    vector<string> tags;
    vector<string> field;
};

vector<Group> groupByScore(Group g, const GroupInfo& info) {
    auto sortedStudents = sortStudent(g.students, info.key);
    vector<Group> groups(info.pivot.size() - 1);
    for (auto s : sortedStudents) {
        float value = (*s)[info.key];
        for (size_t j = 0; j < info.pivot.size() - 1; j++) {
            if (value >= info.pivot[j] && value < info.pivot[j + 1]) {
                groups[j].students.push_back(s);
                break;
            }
        }
    }
    for (size_t i = 0; i < info.pivot.size() - 1; i++){
        groups[i].tags =  g.tags;
        groups[i].field = g.field;
        groups[i].tags.push_back(formatFloat(info.pivot[i]) + " - " + formatFloat(info.pivot[i + 1]));
        groups[i].field.push_back(info.field);
    }

    return groups;
}

vector<Group> groupByProp(Group g, const GroupInfo& info) {
    auto sortedStudents = sortStudent(g.students, info.key);
    int total = sortedStudents.size();
    float totalWeight = 0;
    auto weights = info.pivot;
    for (auto w : weights)
        totalWeight += w;
    vector<int> counts;
    for (auto w : weights)
        counts.push_back(static_cast<int>((w / totalWeight) * total));
    int assigned = 0;
    for (auto c : counts)
        assigned += c;
    int remainder = total - assigned;
    for (auto& c : counts)
        if (remainder-- > 0)
            c++;
    vector<Group> groups(weights.size());
    int idx = 0;
    for (size_t i = 0; i < counts.size(); i++) {
        for (int j = 0; j < counts[i] && idx < total; j++, idx++)
            groups[i].students.push_back(sortedStudents[idx]);
    }
    float s = 0;
    for(int i = 0; i < groups.size(); i ++){
        groups[i].tags =  g.tags;
        groups[i].field = g.field;
        groups[i].field.push_back(info.field);
        groups[i].tags.push_back(formatFloat(s) + " - " + formatFloat(s + info.pivot[i]));
        s += info.pivot[i];
    }

    return groups;
}
vector<vector<Student*>> groupByNum(vector<Student*>& students, const string& key){
    auto sortedStudents = sortStudent(students, key);
    set<float> values;
    vector<vector<Student*>> groups = {};
    int i = -1;
    for(auto &s: sortedStudents){
       if(values.find((*s)[key]) == values.end()){
          groups.push_back(vector<Student*>());
          i += 1;
          values.insert((*s)[key]);
       } 
       groups[i].push_back(s);
    }
    return groups;
}

vector<Student*> filterStudents(const vector<Student*>& students, bool (*filterFunc)(Student*)) {
    vector<Student*> filtered;
    for (auto s : students)
        if (filterFunc(s))
            filtered.push_back(s);
    return filtered;
}



vector<Group> groupBy(const Group& group, const GroupInfo& info) {

    // 根据分组类型选择不同的分组方法
    if (info.type == 1)
        return groupByScore(group, info);
    else if (info.type == 2)
        return groupByProp(group, info);

}

struct OpInfo {
    float (*op)(vector<Student*>&, const string&);
    string key;
    string field;
};

void genSheet(vector<Student*> students,
              bool (*filterFunc)(Student*),
              const vector<GroupInfo>& groupInfos,
              const vector<OpInfo>& opInfos,
              XLWorkbook& workbook,
              const string& sheetName) {
    // 初始过滤，形成一个总的分组
    if(filterFunc) students = filterStudents(students, filterFunc);

    vector<Group> studentGroups = { { students, {} } };
    
    // 按照每个 GroupInfo 细分分组
    for (const auto& info : groupInfos) {
        vector<Group> newGroups;
        for (const auto& grp : studentGroups) {
            auto subdivided = groupBy(grp, info);
            newGroups.insert(newGroups.end(), subdivided.begin(), subdivided.end());
        }
        studentGroups = newGroups;
    }
    
    // 如果工作表不存在则创建
    if (!workbook.sheetExists(sheetName))
        workbook.addWorksheet(sheetName);
    auto wks = workbook.worksheet(sheetName);
    
    int row = 1, col = 1;
    if(studentGroups.size() > 0){
        for(auto filed: studentGroups[0].field){
            wks.cell(row, col++) = filed;
        }
    }
    // 写入表头：第一列为分组信息，其余列为各领域的名称
    for (const auto& op : opInfos) {
        wks.cell(row, col++) = op.field;
    }
    row++;
    
    // 对每个分组，写入分组标签及各统计操作结果
    for (const auto& grp : studentGroups) {
        col = 1;
        // 拼接分组标签，如果标签不为空则用逗号分隔
        string groupInfoStr;
        if (!grp.tags.empty()) {
            for (size_t i = 0; i < grp.tags.size(); i++) {
                wks.cell(row, col++) = grp.tags[i];
            }
        }
        // 针对每个 opInfo 计算对应值并写入
        for (const auto& op : opInfos) {
            float value = op.op(const_cast<vector<Student*>&>(grp.students), op.key);
            wks.cell(row, col++) = value;
        }
        row++;
    }
}


bool filterMath(Student* s) {
    return s->hasKey("数学");
}

float Count(vector<Student*>& students, const string& key) {
    return static_cast<float>(students.size());
}

int main() {
    XLDocument doc;
    doc.open("tables/grade2.xlsx");
    auto workbook = doc.workbook();
    auto wks = workbook.worksheet("原始分");
    
    School school;
    school.load(wks);
    
    GroupInfo groupInfo1({"数学","数学成绩", {0, 90, 120, 150}, 1});
    GroupInfo groupInfo2({"英语", "英语分段", {20, 30, 20}, 2});
    vector<GroupInfo> groupInfos { groupInfo1, groupInfo2 };
    
    OpInfo opInfo { Count, "数学", "人数" };
    vector<OpInfo> opInfos { opInfo };
    
    genSheet(school.students, nullptr, groupInfos, opInfos, workbook, "TestSheet");
    doc.save();
    doc.close();
    return 0;
}
