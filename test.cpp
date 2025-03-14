#include <OpenXLSX.hpp>
#include <iostream>
#include <variant>
#include <map>
#include <vector>

using namespace std;
using namespace OpenXLSX;
#include <iostream>
#include <map>
#include <any>
using Student = map<string, any>;
std::any toAny(XLCellAssignable v) {
    XLCellValue cellVal = v.value();
    switch (cellVal.type()) {
        case XLValueType::Boolean:
            return cellVal.get<bool>();
        case XLValueType::Integer:
            return static_cast<float>(cellVal.get<int64_t>());
        case XLValueType::Float:
            return static_cast<float>(cellVal.get<double>());
        case XLValueType::String:
            return cellVal.get<std::string>();
        case XLValueType::Empty:
            return {}; // 返回空any
        default:
            return {}; // 处理未知类型
    }
}

bool validStudent(Student& s){
    if(s["序号"].has_value() && s["姓名"].has_value()) return true;
    else return false;
}
float getGrade(any& s){
    float r = 0;
    if(s.type() == typeid(int)) r = any_cast<int>(s);
    else if(s.type() == typeid(float)) r = any_cast<float>(s);
    return r;

}

class Class{
public:
    vector<Student> students;
    void push_back(Student s){
        students.push_back(s);
    }
    template <typename T>
    void show(string key){
        for(auto s: students){
            if(s.find(key)!=s.end() && s[key].type() == typeid(T)){
                cout << any_cast<T>(s[key]) << ' ';
            }
        }
        cout << endl;
    }
};

class School{
public:
    map<int, Class> classes;
    vector<string> header;
    int numStudent = 0;
    int numClass = 0;

    School(OpenXLSX::XLWorksheet& wks){
        int ncol = wks.columnCount();
        int nrow = wks.rowCount();
        header.push_back(" ");
        for(int j =1; j <= ncol; j ++){
            header.push_back(wks.cell(1, j).value());
        }
        for(int i = 2; i <= nrow; i ++){
            Student s;
            for(int j = 1; j <= ncol; j ++){
                s[header[j]] = toAny(wks.cell(i, j));
            }
            int classID = 0;
            if(s["班级"].type() == typeid(float)) classID = any_cast<float>(s["班级"]);

            if(classes.find(classID)== classes.end()){
                numClass += 1;
                classes[classID] = Class();
                
            }

            if(validStudent(s)){
                numStudent += 1;
                classes[classID].push_back(s);
            }
        }
    }
};
int main() {
    // 打开现有的 Excel 文件
    XLDocument doc;
    doc.open("tables/grade2.xlsx");
    Student student;
    int a = 0;
    
    // 获取工作表
    auto wks = doc.workbook().worksheet("原始分");
    School Putian4(wks);
    cout << Putian4.numClass << ' ' << Putian4.numStudent << endl;
    Putian4.classes[1].show<float>("物理");
    
    return 0;
}
