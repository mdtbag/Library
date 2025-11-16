#include <bits/stdc++.h>
#include <unistd.h>
#include <random>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
using namespace std;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> dis(1,100);

// ==================== 工具函数 ====================
wstring utf8_to_wstring(const string &str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void showMessage(const string &text, const string &title = "提示") {
    wstring wText = utf8_to_wstring(text);
    wstring wTitle = utf8_to_wstring(title);
    MessageBoxW(NULL, wText.c_str(), wTitle.c_str(), MB_OK | MB_ICONINFORMATION);
}

void red(const string &s) { cout << "\033[31m" << s << "\033[0m"; }

void blue(const string &s) { cout << "\033[44m"<< s << "\033[0m\n"; }

void green(const string &s) { cout << "\033[32m" << s << "\033[0m"; }

void js(int n){
    for(int i = n; i >= 1; i--){
        cout << flush;
        red("\r数据上传中，");
        cout << i;
        red("秒后清屏。");
        sleep(1);
    }
    system("cls");
}

void yzm(){
    red("请输入验证码，确保你不是机器人\n");
    int a, b, ans;
    a = dis(gen);
    b = dis(gen);
    cout << a << "+" << b << "=";
    ans = a + b;
    int x;
    cin >> x;
    if(x != ans){
        red("再给你一次机会\\-_-/\n");
        yzm();
    } else {
        green("已验证您是真人\n");
        green("审核中...\n");
        js(3);
    }
}

string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

vector<string> split(const string &s, char delim) {
    vector<string> res;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) res.push_back(trim(item));
    return res;
}

// ==================== 类定义 ====================
struct Book {
    int id;
    string isbn, title, author, publisher, pubDate, intro;
    double price;
    int pages;
    int borrowCount;

    Book() {}
    Book(int id, string isbn, string title, string author, string publisher,
         string pubDate, double price, int pages, string intro)
        : id(id), isbn(isbn), title(title), author(author), publisher(publisher),
          pubDate(pubDate), price(price), pages(pages), intro(intro), borrowCount(0) {}
};

struct BorrowRecord {
    string readerAccount;
    string bookTitle;
    string borrowDate;
    string returnDate;
    bool returned;

    BorrowRecord(string reader, string book, string bdate)
        : readerAccount(reader), bookTitle(book), borrowDate(bdate),
          returned(false) {}
};

class User {
public:
    string account, password, role;
    User() {}
    User(string acc, string pwd, string r) : account(acc), password(pwd), role(r) {}
    virtual void menu() = 0;
};

class Reader : public User {
public:
    vector<BorrowRecord> myRecords;
    Reader(string acc, string pwd) : User(acc, pwd, "reader") {}
    void menu() override;
};

class Admin : public User {
public:
    Admin(string acc, string pwd) : User(acc, pwd, "admin") {}
    void menu() override;
};

// ==================== 主系统类 ====================
class Library {
public:
    vector<Book> books;
    vector<Admin> admins;
    vector<Reader> readers;
    vector<BorrowRecord> records;

    const string bookFile = "books.txt";
    const string adminFile = "admins.txt";
    const string readerFile = "readers.txt";
    const string recordFile = "records.txt";

    Library() { loadAll(); }

    void loadAll() {
        loadBooks(); loadAdmins(); loadReaders(); loadRecords();
    }

    void saveAll() {
        saveBooks(); saveAdmins(); saveReaders(); saveRecords();
    }

    void loadBooks() {
        books.clear();
        ifstream fin(bookFile);
        if (!fin) return;
        string line;
        int lineNum = 0;

        auto safe_stoi = [](const string &s, int def = 0) {
            try { return stoi(s); } catch (...) { return def; }
        };
        auto safe_stod = [](const string &s, double def = 0.0) {
            try { return stod(s); } catch (...) { return def; }
        };

        while (getline(fin, line)) {
            lineNum++;
            if (line.empty()) continue;
            auto v = split(line, '|');
            if (v.size() < 9) {
                red("[警告] 第 " + to_string(lineNum) + " 行字段不足，已跳过。\n");
                continue;
            }
            try {
                Book b(
                    safe_stoi(v[0]),
                    v[1], v[2], v[3], v[4], v[5],
                    safe_stod(v[6]),
                    safe_stoi(v[7]),
                    v[8]
                );
                b.borrowCount = (v.size() > 9 ? safe_stoi(v[9]) : 0);
                books.push_back(b);
            } catch (...) {
                red("[警告] 第 " + to_string(lineNum) + " 行数据格式错误，已跳过。\n");
            }
        }
        fin.close();
    }

    void saveBooks() {
        ofstream fout(bookFile);
        for (auto &b : books)
            fout << b.id << "|" << b.isbn << "|" << b.title << "|" << b.author << "|"
                 << b.publisher << "|" << b.pubDate << "|" << b.price << "|"
                 << b.pages << "|" << b.intro << "|" << b.borrowCount << "\n";
        fout.close();
    }

    void loadAdmins() {
        admins.clear();
        ifstream fin(adminFile);
        if (!fin) return;
        string acc, pwd;
        while (fin >> acc >> pwd)
            admins.emplace_back(acc, pwd);
        fin.close();
    }

    void saveAdmins() {
        ofstream fout(adminFile);
        for (auto &a : admins)
            fout << a.account << " " << a.password << "\n";
        fout.close();
    }

    void loadReaders() {
        readers.clear();
        ifstream fin(readerFile);
        if (!fin) return;
        string acc, pwd;
        while (fin >> acc >> pwd)
            readers.emplace_back(acc, pwd);
        fin.close();
    }

    void saveReaders() {
        ofstream fout(readerFile);
        for (auto &r : readers)
            fout << r.account << " " << r.password << "\n";
        fout.close();
    }

    void loadRecords() {
        records.clear();
        ifstream fin(recordFile);
        if (!fin) return;
        string line;
        while (getline(fin, line)) {
            auto v = split(line, '|');
            if (v.size() < 5) continue;
            BorrowRecord r(v[0], v[1], v[2]);
            r.returnDate = v[3];
            r.returned = (v[4] == "1");
            records.push_back(r);
        }
        fin.close();
    }

    void saveRecords() {
        ofstream fout(recordFile);
        for (auto &r : records)
            fout << r.readerAccount << "|" << r.bookTitle << "|" << r.borrowDate
                 << "|" << r.returnDate << "|" << (r.returned ? "1" : "0") << "\n";
        fout.close();
    }

    Book* findBookByTitle(const string &title) {
        for (auto &b : books)
            if (b.title == title) return &b;
        return nullptr;
    }

    Book* findBookById(int id) {
        for (auto &b : books)
            if (b.id == id) return &b;
        return nullptr;
    }

    Reader* findReaderByAccount(const string &acc) {
        for (auto &r : readers)
            if (r.account == acc) return &r;
        return nullptr;
    }

    bool readerExists(const string &acc) {
        return findReaderByAccount(acc) != nullptr;
    }

    // 重新整理图书ID，保持连续
    void reorderBookIds() {
        sort(books.begin(), books.end(), [](const Book &a, const Book &b) {
            return a.id < b.id;
        });
        for (int i = 0; i < books.size(); i++) {
            books[i].id = i + 1;
        }
    }

    // 获取下一个可用的图书ID
    int getNextBookId() {
        if (books.empty()) return 1;
        int maxId = 0;
        for (auto &b : books) {
            maxId = max(maxId, b.id);
        }
        return maxId + 1;
    }

    void showRankings() {
        blue("\n==== 排行榜 ====\n");
        
        blue("\n借阅次数最多的图书前10名：\n");
        vector<Book> tmp = books;
        sort(tmp.begin(), tmp.end(), [](Book &a, Book &b) {
            return a.borrowCount > b.borrowCount;
        });
        for (int i = 0; i < min(10, (int)tmp.size()); i++)
            cout << i+1 << ". " << tmp[i].title << "（" << tmp[i].borrowCount << "次）\n";

        blue("\n最新出版图书前10名：\n");
        sort(tmp.begin(), tmp.end(), [](Book &a, Book &b) {
            return a.pubDate > b.pubDate;
        });
        for (int i = 0; i < min(10, (int)tmp.size()); i++)
            cout << i+1 << ". " << tmp[i].title << "（出版时间：" << tmp[i].pubDate << "）\n";

        blue("\n借阅次数最多的读者前10名：\n");
        map<string, int> cnt;
        for (auto &r : records)
            if (!r.readerAccount.empty()) cnt[r.readerAccount]++;
        vector<pair<string,int>> rank(cnt.begin(), cnt.end());
        sort(rank.begin(), rank.end(), [](auto &a, auto &b){ return a.second > b.second; });
        for (int i = 0; i < min(10, (int)rank.size()); i++)
            cout << i+1 << ". " << rank[i].first << "（" << rank[i].second << "次）\n";
    }
};

Library lib;

// ==================== 管理员功能 ====================
void Admin::menu() {
    int choice;
    while (true) {
        blue("\n===== 管理员菜单 =====\n");
        cout << "1. 添加图书\n2. 删除图书\n3. 修改图书\n4. 显示所有图书\n5. 管理读者账号\n6. 查看排行榜\n0. 返回\n";
        blue("请选择：");
        cin >> choice;
        
        if (choice == 0) {
            js(30);
            break;
        }
        
        if (choice == 1) {
            Book b;
            b.id = lib.getNextBookId();
            green("系统自动分配ID：" + to_string(b.id) + "\n");
            
            cout << "ISBN："; cin >> b.isbn;
            cin.ignore();
            cout << "书名："; getline(cin, b.title);
            
            // 检查书名是否已存在
            if (lib.findBookByTitle(b.title) != nullptr) {
                red("该书名已存在，添加失败！\n");
                continue;
            }
            
            cout << "作者："; getline(cin, b.author);
            cout << "出版社："; getline(cin, b.publisher);
            cout << "出版时间："; getline(cin, b.pubDate);
            cout << "价格："; cin >> b.price;
            cout << "页数："; cin >> b.pages;
            cin.ignore();
            cout << "简介："; getline(cin, b.intro);
            b.borrowCount = 0;
            
            lib.books.push_back(b);
            lib.saveBooks();
            green("图书《" + b.title + "》添加成功！ID为：" + to_string(b.id) + "\n");
            
        } else if (choice == 2) {
            if (lib.books.empty()) {
                red("图书库为空，无法删除！\n");
                continue;
            }
            
            blue("请选择删除方式：\n");
            cout << "1. 按书名删除\n2. 按ID删除\n";
            blue("请选择：");
            int delChoice;
            cin >> delChoice;
            
            bool found = false;
            if (delChoice == 1) {
                string title;
                cout << "输入要删除的书名："; 
                cin.ignore(); 
                getline(cin, title);
                
                for (auto it = lib.books.begin(); it != lib.books.end(); ++it) {
                    if (it->title == title) {
                        lib.books.erase(it);
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    lib.reorderBookIds();
                    lib.saveBooks();
                    green("图书《" + title + "》删除成功！ID已自动调整。\n");
                } else {
                    red("未找到书名为《" + title + "》的图书，删除失败！\n");
                }
                
            } else if (delChoice == 2) {
                int id;
                cout << "输入要删除的图书ID：";
                cin >> id;
                
                for (auto it = lib.books.begin(); it != lib.books.end(); ++it) {
                    if (it->id == id) {
                        string title = it->title;
                        lib.books.erase(it);
                        found = true;
                        lib.reorderBookIds();
                        lib.saveBooks();
                        green("图书《" + title + "》（ID:" + to_string(id) + "）删除成功！ID已自动调整。\n");
                        break;
                    }
                }
                
                if (!found) {
                    red("未找到ID为 " + to_string(id) + " 的图书，删除失败！\n");
                }
            } else {
                red("无效选择！\n");
            }
            
        } else if (choice == 3) {
            if (lib.books.empty()) {
                red("图书库为空，无法修改！\n");
                continue;
            }
            
            blue("请选择查找方式：\n");
            cout << "1. 按书名查找\n2. 按ID查找\n";
            blue("请选择：");
            int findChoice;
            cin >> findChoice;
            
            Book *b = nullptr;
            if (findChoice == 1) {
                string title;
                cout << "输入要修改的书名："; 
                cin.ignore(); 
                getline(cin, title);
                b = lib.findBookByTitle(title);
            } else if (findChoice == 2) {
                int id;
                cout << "输入要修改的图书ID：";
                cin >> id;
                b = lib.findBookById(id);
            } else {
                red("无效选择！\n");
                continue;
            }
            
            if (!b) {
                red("未找到该图书，修改失败！\n");
                continue;
            }
            
            cout << "当前信息：《" << b->title << "》 价格：" << b->price << " 简介：" << b->intro << "\n";
            cout << "新价格："; 
            cin >> b->price;
            cout << "新简介："; 
            cin.ignore(); 
            getline(cin, b->intro);
            lib.saveBooks();
            green("图书《" + b->title + "》修改成功！\n");
            
        } else if (choice == 4) {
            if (lib.books.empty()) {
                red("图书库为空！\n");
                continue;
            }
            
            blue("\n========== 所有图书列表 ==========\n");
            for (auto &b : lib.books) {
                cout << "[ID:" << b.id << "] 《" << b.title << "》\n";
                cout << "  作者：" << b.author << " | 出版社：" << b.publisher << "\n";
                cout << "  价格：" << b.price << "元 | 页数：" << b.pages << "页\n";
                cout << "  借阅次数：" << b.borrowCount << "次\n";
                cout << "  简介：" << b.intro << "\n";
                cout << "------------------------------------\n";
            }
            green("共 " + to_string(lib.books.size()) + " 本图书\n");
            
        } else if (choice == 5) {
            blue("\n===== 读者账号管理 =====\n");
            cout << "1. 显示所有读者\n2. 添加读者\n3. 删除读者\n4. 重置密码\n";
            blue("请选择：");
            int c; 
            cin >> c;
            
            if (c == 1) {
                if (lib.readers.empty()) {
                    red("当前没有读者账号！\n");
                } else {
                    blue("\n========== 所有读者账号 ==========\n");
                    for (int i = 0; i < lib.readers.size(); i++) {
                        cout << i+1 << ". 账号：" << lib.readers[i].account << "\n";
                    }
                    green("共 " + to_string(lib.readers.size()) + " 个读者账号\n");
                }
                
            } else if (c == 2) {
                string a, p;
                cout << "账号："; 
                cin >> a;
                
                if (lib.readerExists(a)) {
                    red("账号 " + a + " 已存在，添加失败！\n");
                    continue;
                }
                
                cout << "密码："; 
                cin >> p;
                lib.readers.emplace_back(a, p);
                lib.saveReaders();
                green("读者账号 " + a + " 添加成功！\n");
                
            } else if (c == 3) {
                if (lib.readers.empty()) {
                    red("当前没有读者账号，无法删除！\n");
                    continue;
                }
                
                string a;
                cout << "输入要删除的账号：";
                cin >> a;
                
                auto it = remove_if(lib.readers.begin(), lib.readers.end(),
                                    [&](Reader &r){ return r.account == a; });
                
                if (it != lib.readers.end()) {
                    lib.readers.erase(it, lib.readers.end());
                    lib.saveReaders();
                    green("账号 " + a + " 删除成功！\n");
                } else {
                    red("未找到账号 " + a + "，删除失败！\n");
                }
                
            } else if (c == 4) {
                if (lib.readers.empty()) {
                    red("当前没有读者账号，无法重置！\n");
                    continue;
                }
                
                string a;
                cout << "输入要重置密码的账号：";
                cin >> a;
                
                bool found = false;
                for (auto &r : lib.readers) {
                    if (r.account == a) {
                        r.password = "1234";
                        found = true;
                        break;
                    }
                }
                
                if (found) {
                    lib.saveReaders();
                    green("账号 " + a + " 的密码已重置为 1234\n");
                } else {
                    red("未找到账号 " + a + "，重置失败！\n");
                }
            } else {
                red("无效选择！\n");
            }
            
        } else if (choice == 6) {
            lib.showRankings();
        } else {
            red("无效选择，请重新输入！\n");
            system('cls');
        }
    }
}

// ==================== 读者功能 ====================
void Reader::menu() {
    int choice;
    while (true) {
        blue("\n===== 读者菜单 =====\n");
        cout << "1. 搜索图书\n2. 借书\n3. 还书\n4. 查看借阅记录\n5. 修改密码\n0. 返回\n";
        blue("请选择：");
        cin >> choice;
        
        if (choice == 0) {
            js(3);
            break;
        }
        
        if (choice == 1) {
            if (lib.books.empty()) {
                red("图书库为空！\n");
                continue;
            }
            
            blue("请选择搜索方式：\n");
            cout << "1. 按书名搜索\n2. 按作者搜索\n3. 按ID搜索\n";
            blue("请选择：");
            int searchChoice;
            cin >> searchChoice;
            
            bool found = false;
            if (searchChoice == 1) {
                string key;
                cout << "输入书名：";
                cin.ignore();
                getline(cin, key);
                
                for (auto &b : lib.books) {
                    if (b.title.find(key) != string::npos) {
                        cout << "\n[ID:" << b.id << "] 《" << b.title << "》\n";
                        cout << "作者：" << b.author << " | 出版社：" << b.publisher << "\n";
                        cout << "价格：" << b.price << "元 | 页数：" << b.pages << "页\n";
                        cout << "简介：" << b.intro << "\n";
                        cout << "------------------------------------\n";
                        found = true;
                    }
                }
                
            } else if (searchChoice == 2) {
                string key;
                cout << "输入作者：";
                cin.ignore();
                getline(cin, key);
                
                for (auto &b : lib.books) {
                    if (b.author.find(key) != string::npos) {
                        cout << "\n[ID:" << b.id << "] 《" << b.title << "》\n";
                        cout << "作者：" << b.author << " | 出版社：" << b.publisher << "\n";
                        cout << "价格：" << b.price << "元 | 页数：" << b.pages << "页\n";
                        cout << "简介：" << b.intro << "\n";
                        cout << "------------------------------------\n";
                        found = true;
                    }
                }
                
            } else if (searchChoice == 3) {
                int id;
                cout << "输入图书ID：";
                cin >> id;
                
                Book *b = lib.findBookById(id);
                if (b) {
                    cout << "\n[ID:" << b->id << "] 《" << b->title << "》\n";
                    cout << "作者：" << b->author << " | 出版社：" << b->publisher << "\n";
                    cout << "价格：" << b->price << "元 | 页数：" << b->pages << "页\n";
                    cout << "简介：" << b->intro << "\n";
                    cout << "------------------------------------\n";
                    found = true;
                }
                
            } else {
                red("无效选择！\n");
                continue;
            }
            
            if (!found) {
                red("未找到匹配的图书！\n");
            }
            
        } else if (choice == 2) {
            if (lib.books.empty()) {
                red("图书库为空，无法借书！\n");
                continue;
            }
            
            string t;
            cout << "输入要借的书名：";
            cin.ignore();
            getline(cin, t);
            
            Book *b = lib.findBookByTitle(t);
            if (!b) {
                red("未找到书名为《" + t + "》的图书，借阅失败！\n");
                continue;
            }
            
            // 检查是否已借阅未归还
            bool alreadyBorrowed = false;
            for (auto &r : lib.records) {
                if (r.readerAccount == account && r.bookTitle == t && !r.returned) {
                    alreadyBorrowed = true;
                    break;
                }
            }
            
            if (alreadyBorrowed) {
                red("您已借阅该书且未归还，无法重复借阅！\n");
                continue;
            }
            
            b->borrowCount++;
            lib.records.emplace_back(account, t, "2025-10-04");
            lib.saveBooks();
            lib.saveRecords();
            green("图书《" + t + "》借阅成功！请按时归还。\n");
            
        } else if (choice == 3) {
            string t;
            cout << "输入要还的书名：";
            cin.ignore();
            getline(cin, t);
            
            bool found = false;
            for (auto &r : lib.records) {
                if (r.readerAccount == account && r.bookTitle == t && !r.returned) {
                    r.returned = true;
                    r.returnDate = "2025-10-04";
                    lib.saveRecords();
                    green("图书《" + t + "》归还成功！\n");
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                red("未找到您借阅的《" + t + "》或已归还，操作失败！\n");
            }
            
        } else if (choice == 4) {
            blue("\n========== 我的借阅记录 ==========\n");
            bool hasRecord = false;
            for (auto &r : lib.records) {
                if (r.readerAccount == account) {
                    cout << "《" << r.bookTitle << "》\n";
                    cout << "  借阅日期：" << r.borrowDate << "\n";
                    cout << "  归还日期：" << (r.returned ? r.returnDate : "未还") << "\n";
                    cout << "  状态：";
                    if (r.returned) {
                        green("已归还\n");
                    } else {
                        red("未归还\n");
                    }
                    cout << "------------------------------------\n";
                    hasRecord = true;
                }
            }
            
            if (!hasRecord) {
                red("您还没有借阅记录！\n");
            }
            
        } else if (choice == 5) {
            string newPwd, confirmPwd;
            cout << "输入新密码：";
            cin >> newPwd;
            cout << "确认新密码：";
            cin >> confirmPwd;
            
            if (newPwd != confirmPwd) {
                red("两次密码输入不一致，修改失败！\n");
                continue;
            }
            
            password = newPwd;
            lib.saveReaders();
            green("密码修改成功！\n");
        } else {
            red("无效选择，请重新输入！\n");
        }
    }
}

// ==================== 主菜单 ====================
void mainMenu() {
    while (true) {
        blue("\n===== 欢迎进入湖南大学VIP图书馆管理系统 \\^_^/=====\n");
        cout << "1. 管理员登录\n2. 读者登录\n3. 注册账号\n0. 退出\n";
        blue("请选择：");
        string c; 
        cin >> c;
        
        if (c == "0") {
            lib.saveAll();
            for(int i = 5; i >= 1; i--){
                cout << flush;
                green("\r同步中...");
                cout << i;
                green("秒后系统自动退出。");
                sleep(1);
            }
            blue("\n退出成功！再见！\n");
            sleep(2);
            break;
        }
        
        if (c == "1") {
            string a, p;
            cout << "账号："; 
            cin >> a;
            cout << "密码："; 
            cin >> p;
            yzm();
            
            bool ok = false;
            for (auto &ad : lib.admins) {
                if (ad.account == a && ad.password == p) {
                    green("登录成功！欢迎管理员 " + a + "\n");
                    ad.menu();
                    ok = true;
                    break;
                }
            }
            
            if (!ok) {
                red("账户或密码错误！\n");
                showMessage("账户或密码错误。你不会是CSU的吧", "来自HNU的提示");
            }
            
        } else if (c == "2") {
            string a, p;
            cout << "账号："; 
            cin >> a;
            cout << "密码："; 
            cin >> p;
            yzm();
            
            bool ok = false;
            for (auto &r : lib.readers) {
                if (r.account == a && r.password == p) {
                    green("登录成功！欢迎读者 " + a + "\n");
                    r.menu();
                    ok = true;
                    break;
                }
            }
            
            if (!ok) {
                red("账户或密码错误！\n");
                showMessage("账户或密码错误。你不会是CSU的吧", "来自HNU的提示");
            }
            
        } else if (c == "3") {
            string a, p;
            cout << "输入账号："; 
            cin >> a;
            
            // 检查账号是否已存在
            if (lib.readerExists(a)) {
                red("账号 " + a + " 已存在！\n");
                red("请重新取一个账户名。\n");
                continue;
            }
            
            cout << "密码："; 
            cin >> p;
            cout << "请输入湖大校训：";
            string hnuxx;
            cin >> hnuxx;
            green("干得好，你是懂湖大的\n");
            yzm();
            
            lib.readers.emplace_back(a, p);
            lib.saveReaders();
            green("注册成功！欢迎来自HNU的新用户 " + a + "\n");
            showMessage("注册成功！欢迎来自HNU的新用户", "来自HNU的提示");
            
        } else {
            red("无效选择！你不会是CSU的吧，认真点\n");
        }
    }
}

// ==================== 程序入口 ====================
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_TEXT);
    _setmode(_fileno(stdin), _O_TEXT);
    
    showMessage("只有 HNU 的 VIP 才能使用哦 \\^_^/ ", "来自 HNU 的提示");
    mainMenu();
    return 0;
}