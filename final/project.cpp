#include <bits/stdc++.h>
using namespace std;

// ---------------- Utilities ----------------
string now_iso() {
    time_t t = time(nullptr);
    tm bt = *localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &bt);
    return string(buf);
}

vector<string> split_csv(const string &line){
    vector<string> res; string tmp;
    for(char c: line){ if(c==','){res.push_back(tmp); tmp.clear();} else tmp+=c;}
    res.push_back(tmp); return res;
}

// ---------------- Password Hash ----------------
string hash_password(const string &password){
    size_t h = hash<string>{}(password);
    stringstream ss; ss << h; return ss.str();
}

// ---------------- Account ----------------
struct Account {
    int accNo;
    string name;
    double balance;
    string password; // hashed
};

const string ACC_FILE = "accounts.csv";

vector<Account> load_accounts(){
    vector<Account> v; ifstream in(ACC_FILE); string line;
    while(getline(in,line)){
        if(line.empty()) continue;
        auto p = split_csv(line); if(p.size()<4) continue;
        Account a; a.accNo=stoi(p[0]); a.name=p[1]; a.balance=stod(p[2]); a.password=p[3];
        v.push_back(a);
    }
    return v;
}

void save_accounts(const vector<Account> &v){
    ofstream out(ACC_FILE, ios::trunc);
    for(auto &a:v) out<<a.accNo<<","<<a.name<<","<<fixed<<setprecision(2)<<a.balance<<","<<a.password<<"\n";
}

int find_account_index(const vector<Account> &v,int accNo){
    for(int i=0;i<(int)v.size();++i) if(v[i].accNo==accNo) return i;
    return -1;
}

// ---------------- Transaction ----------------
string hist_file(int accNo){ return "history_" + to_string(accNo) + ".csv"; }

void log_txn(int accNo,const string &type,double amt,double bal,const string &note=""){
    ofstream out(hist_file(accNo), ios::app);
    out<<now_iso()<<","<<type<<","<<fixed<<setprecision(2)<<amt<<","<<bal<<","<<note<<"\n";
}

void show_history(int accNo){
    ifstream in(hist_file(accNo)); if(!in){ cout<<"No history.\n"; return;}
    struct Row{ string ts,type,note; double amt=0,bal=0; };
    vector<Row> rows; string line;
    while(getline(in,line)){
        auto p = split_csv(line); if(p.size()<5) continue;
        Row r; r.ts=p[0]; r.type=p[1]; r.amt=stod(p[2]); r.bal=stod(p[3]); r.note=p[4];
        rows.push_back(r);
    }
    if(rows.empty()){cout<<"No transactions.\n"; return;}
    cout<<"\nTimestamp           | Type        | Amount      | Balance     | Note\n";
    cout<<string(72,'-')<<"\n";
    for(auto &r: rows)
        cout<<left<<setw(20)<<r.ts<<setw(12)<<r.type<<setw(12)<<fixed<<setprecision(2)<<r.amt<<setw(12)<<r.bal<<setw(20)<<r.note<<"\n";
}

void download_full_history(int accNo){
    ifstream in(hist_file(accNo)); 
    if(!in){ cout<<"No history for account "<<accNo<<"\n"; return;}
    string outname="history_"+to_string(accNo)+"_full.csv";
    ofstream out(outname, ios::trunc);
    out<<"Timestamp,Type,Amount,Balance,Note\n";
    string line; while(getline(in,line)) out<<line<<"\n";
    cout<<"Full transaction history saved to "<<outname<<"\n";
}

// ---------------- Statement ----------------
void generate_statement(int accNo, string date) {
    ifstream in(hist_file(accNo)); 
    if(!in){ cout<<"No history.\n"; return;}
    
    struct Row{ string ts,type,note; double amt=0,bal=0; };
    vector<Row> rows; string line;

    while(getline(in,line)){
        auto p = split_csv(line); 
        if(p.size()<5) continue;
        // Filter by date (YYYY-MM-DD) or month (YYYY-MM)
        if(date.length()==10 && p[0].substr(0,10)!=date) continue;
        if(date.length()==7 && p[0].substr(0,7)!=date) continue;
        Row r; r.ts=p[0]; r.type=p[1]; r.amt=stod(p[2]); r.bal=stod(p[3]); r.note=p[4]; 
        rows.push_back(r);
    }

    if(rows.empty()){ cout<<"No transactions for "<<date<<"\n"; return;}
    
    sort(rows.begin(),rows.end(),[](Row &a, Row &b){ return a.ts<b.ts; });
    
    double opening=rows.front().bal; 
    if(rows.front().type=="Deposit") opening-=rows.front().amt;
    else if(rows.front().type=="Withdraw") opening+=rows.front().amt;
    
    double totalDep=0,totalWdr=0;
    for(auto &r:rows){ if(r.type=="Deposit") totalDep+=r.amt; else if(r.type=="Withdraw") totalWdr+=r.amt; }
    double closing=rows.back().bal;
    
    string outname="statement_"+to_string(accNo)+"_"+date+".txt";
    ofstream out(outname,ios::trunc);
    out<<"===============================================\n";
    out<<"               BANK STATEMENT                  \n";
    out<<"Account: "<<accNo<<"    Date/Month: "<<date<<"\n";
    out<<"===============================================\n";
    out<<"Opening Balance: "<<fixed<<setprecision(2)<<opening<<"\n";
    out<<"Total Deposits : "<<totalDep<<"\n";
    out<<"Total Withdraws: "<<totalWdr<<"\n";
    out<<"Closing Balance: "<<closing<<"\n\n";
    out<<left<<setw(20)<<"Timestamp"<<setw(12)<<"Type"<<setw(12)<<"Amount"<<setw(12)<<"Balance"<<setw(20)<<"Note"<<"\n";
    out<<string(76,'-')<<"\n";
    for(auto &r:rows) out<<left<<setw(20)<<r.ts<<setw(12)<<r.type<<setw(12)<<r.amt<<setw(12)<<r.bal<<setw(20)<<r.note<<"\n";

    cout<<"Statement generated: "<<outname<<"\n";
}

// ---------------- Admin ----------------
const string ADMIN_USER="admin",ADMIN_PASS="admin123";

void create_account(){
    vector<Account> v=load_accounts(); Account a;
    cout<<"Account No: "; cin>>a.accNo; if(find_account_index(v,a.accNo)!=-1){cout<<"Exists\n"; return;}
    cin.ignore(); cout<<"Name: "; getline(cin,a.name); cout<<"Initial Balance: "; cin>>a.balance;
    string raw_pass; cout<<"Password: "; cin>>raw_pass;
    a.password = hash_password(raw_pass);
    v.push_back(a); save_accounts(v); log_txn(a.accNo,"Open",a.balance,a.balance,"Account created"); cout<<"Account created.\n";
}

void display_accounts(){ 
    vector<Account> v=load_accounts(); 
    if(v.empty()){cout<<"No accounts\n"; return;} 
    for(auto &a:v) cout<<"AccNo: "<<a.accNo<<" | Name: "<<a.name<<" | Balance: "<<fixed<<setprecision(2)<<a.balance<<"\n"; 
}

void delete_account() {
    vector<Account> v = load_accounts();
    int accNo;
    cout << "Enter account number to delete: ";
    cin >> accNo;
    int idx = find_account_index(v, accNo);
    if (idx == -1) { cout << "Account not found.\n"; return; }

    char confirm;
    cout << "Are you sure you want to delete account " << accNo << "? (y/n): ";
    cin >> confirm;
    if (confirm != 'y' && confirm != 'Y') { cout << "Deletion canceled.\n"; return; }

    v.erase(v.begin() + idx);
    save_accounts(v);

    string hist = hist_file(accNo);
    if (remove(hist.c_str()) == 0) cout << "Transaction history deleted.\n";

    cout << "Account deleted successfully.\n";
}

// ---------------- User ----------------
void user_session(int accNo){
    vector<Account> v=load_accounts(); int idx=find_account_index(v,accNo);
    if(idx==-1){cout<<"Not found\n"; return;}
    string pass; cout<<"Password: "; cin>>pass; 
    if(hash_password(pass)!=v[idx].password){cout<<"Wrong\n"; return;}
    while(true){
        cout<<"\n1.View 2.Deposit 3.Withdraw 4.Transfer 5.History 6.Statement 7.Download History 8.Logout\nChoice: "; 
        int ch; cin>>ch;
        if(ch==1) cout<<"Name: "<<v[idx].name<<" | Balance: "<<v[idx].balance<<"\n";
        else if(ch==2){ double amt; cout<<"Deposit: "; cin>>amt; v[idx].balance+=amt; save_accounts(v); log_txn(accNo,"Deposit",amt,v[idx].balance,"User deposit"); cout<<"Done\n"; }
        else if(ch==3){ double amt; cout<<"Withdraw: "; cin>>amt; if(amt>v[idx].balance){cout<<"Insufficient\n"; continue;} v[idx].balance-=amt; save_accounts(v); log_txn(accNo,"Withdraw",amt,v[idx].balance,"User withdraw"); cout<<"Done\n"; }
        else if(ch==4){ int toAcc; double amt; cout<<"To Account: "; cin>>toAcc; int tidx=find_account_index(v,toAcc); if(tidx==-1){cout<<"Invalid\n"; continue;} cout<<"Amount: "; cin>>amt; if(amt>v[idx].balance){cout<<"Insufficient\n"; continue;} v[idx].balance-=amt; v[tidx].balance+=amt; save_accounts(v); log_txn(accNo,"Transfer",-amt,v[idx].balance,"To "+to_string(toAcc)); log_txn(toAcc,"Transfer",amt,v[tidx].balance,"From "+to_string(accNo)); cout<<"Transferred\n"; }
        else if(ch==5) show_history(accNo);
        else if(ch==6){ string d; cout<<"Enter date (YYYY-MM-DD) or month (YYYY-MM): "; cin>>d; generate_statement(accNo,d); }
        else if(ch==7) download_full_history(accNo);
        else if(ch==8) break; else cout<<"Invalid\n";
    }
}

// ---------------- Admin Session ----------------
void admin_session(){
    string u,p; cout<<"Username: "; cin>>u; cout<<"Password: "; cin>>p;
    if(u!=ADMIN_USER||p!=ADMIN_PASS){cout<<"Wrong\n"; return;}
    while(true){
        cout<<"\n1.Create 2.Display 3.Download History 4.Delete Account 5.Logout\nChoice: ";
        int ch; cin>>ch;
        if(ch==1) create_account(); 
        else if(ch==2) display_accounts(); 
        else if(ch==3){ int accNo; cout<<"Enter account number to download history: "; cin>>accNo; download_full_history(accNo); }
        else if(ch==4) delete_account();
        else if(ch==5) break; 
        else cout<<"Invalid\n";
    }
}

// ---------------- Main ----------------
int main(){
    while(true){
        cout<<"\n===== Bank System =====\n1.Admin 2.User 3.Exit\nChoice: "; int ch; cin>>ch;
        if(ch==1) admin_session();
        else if(ch==2){ int acc; cout<<"Account No: "; cin>>acc; user_session(acc); }
        else if(ch==3){ cout<<"Goodbye\n"; break; }
        else cout<<"Invalid\n";
    }
    string exit_cmd;
    while(true){
        cout<<"Type 'exit' to close program: ";
        cin >> exit_cmd;
        if(exit_cmd=="exit" || exit_cmd=="EXIT") break;
    }
    return 0;
}
