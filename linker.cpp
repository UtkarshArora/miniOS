#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
using namespace std;


int readInt(int& index, string str)
{
    int num=0;
    
    while(index<str.size())
    {
        if(isdigit(str[index]))
        {
            int start=index;
            int len=0;
            while(index<str.size() && isdigit(str[index]))
            {
                
                len++;
                index++;
            }
            string op=str.substr(start,len);
            num=stoi(op);
            return num;
        }
        else if(isalpha(str[index]))
        {
            return -3;
        }
        else
        {
            index++;
        }
            
    }

    return -1;
}

string readSym(int& index, string str)
{
    if(index==str.size())
    {
        return "end";
    }
    int len=0;
    while(index<str.size())
    {
        if(isalpha(str[index])){
            int start=index;
            while(index<str.size() && isalnum(str[index]))
            {
                index++;
                len++;
            }
            string op=str.substr(start,len);
            
            return op;
        }
        else if(isdigit(str[index]))
        {
            return "found num";
        }
        else
            index++;
    }
   
    return "";
}

//code for PASS 2 below
 void pass2(const std::string& filename, unordered_map<string,int>&defTable, unordered_map<int,int>&moduleBaseTable)
 {

     unordered_map<int, vector<string>>symbolCount;
     unordered_set<string>usedSym;
     int memCount=0; 
     vector<string>useCountIndex;
     vector<bool>accesedValues;
     
     std::ifstream file(filename);
     
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }
    
        std::string str;
        int lineNumber = 0;
        int lastOffset=0;
    
     int defcount=0;
     int instcount=0;
     int usecount=0;
     bool readDefCount=false,readSymbol=false,readInstrCount=false;
   
    bool defList=false, useList=false, progText=false;
    char ch;
    int module_count=0;
    

    while (std::getline(file, str)) {
       
    lineNumber++;
        
    int index=0;
    
    while(index<str.size())
    {
        if(defList == false && index<str.size())
        {
            if(!readDefCount){
                defcount=readInt(index, str);
                if(defcount==0)
                {
                    readDefCount=false;
                    defList=true;
                }
                else if(defcount>0)
                    readDefCount=true;
                
                else if(defcount<0)
                    break;
            }
            for(int i=0;i<defcount;)
            {
                string sym="";
                if(!readSymbol)
                {
                    sym=readSym(index, str);
                    if(sym=="" || sym=="end")
                    {
                        break;
                    }
                    readSymbol=true;
                }
                int val=readInt(index, str);
                if(val>0 || (val==0 && defcount>0))
                {
                        readSymbol=false;
                        defcount--;
                        defList=(defcount==0);
                        if(!symbolCount.count(module_count))
                        {
                            vector<string>s1;
                            s1.push_back(sym);
                            symbolCount[module_count]=s1;
                        }
                        else
                        {
                            symbolCount[module_count].push_back(sym);
                        }
                        
                }
                else if(val == -1){
                    break;
                }
            }
        
    }

        else if(useList==false && index<str.size()){
            
                if(usecount==0)
                    usecount = readInt(index, str);
                
                accesedValues.assign(usecount, false);
                for (int i=0;i<usecount;) {
                    string sym = readSym(index, str); 
                    if(sym=="end" || sym=="")
                        break;
                    else
                        {
                            useCountIndex.push_back(sym);
                            usedSym.insert(sym);
                            usecount--;
                        }
                }
                useList=(usecount==0);
        }

        else if(progText == false && index<str.size())
        {
            
            if(!readInstrCount){
                readSymbol=false;
                instcount = readInt(index, str);
                readInstrCount=true;
            }
            
            for(int i=0;i<instcount;)
            {
                    if(!readSymbol){
                        string str1=readSym(index, str);
                        if(str1=="end")
                        {
                            break;
                        }
                        if(str1!="")
                        {
                            ch=str1[0];
                            readSymbol=true;
                        }
                    }
                  if(readSymbol){
                      int val=readInt(index, str);
                      if(val == -2 || val == -1){
                          break;
                      }
                      int opcode  = val/1000;
                      int operand = val%1000;
                      int outputAdd=0;
                      string s="";
                      string s1=to_string(memCount);
                      if(memCount==0)
                      {
                          cout<<endl<<"Memory Map"<<endl;
                      }
                      
                      while(s1.size()<3)
                        {
                            s1='0'+s1;
                        }
                        if(opcode>=10)
                        {
                            
                            s="9999 Error: Illegal opcode; treated as 9999";
                            string output=s1+": "+s;
                            cout<<output<<endl;
                            memCount++;
                        }
                        else
                        {
                            int val1=opcode;
                            switch(ch) {
                              case 'A':
                                {
                                   if(operand>=512)
                                   {
                                       int temp=(val-operand);
                                       s=(to_string(temp))+" Error: Absolute address exceeds machine size; zero used";
                                   }
                                   else
                                   {
                                       outputAdd=val; 
                                       s=to_string(outputAdd);
                                   }
                                }
                                break;
                              case 'E':
                                {
                                    if(operand>=useCountIndex.size())
                                    {
                                        int temp= (val-operand);
                                        s=to_string(temp)+" Error: External operand exceeds length of uselist; treated as relative=0";
                                    }
                                    else{
                                        if(defTable.count(useCountIndex[operand]))
                                        {
                                            accesedValues[operand]=true;
                                            outputAdd=(val-operand)+defTable[useCountIndex[operand]];
                                            s=to_string(outputAdd);
                                        }
                                        else{
                                            accesedValues[operand]=true;
                                            outputAdd=(val-operand);
                                            s=to_string(outputAdd)+" Error: "+useCountIndex[operand]+" is not defined; zero used";
                                        }
                                    }
                                }
                                break;
                              case 'I':
                                {
                                    if(operand>=900)
                                    {
                                        s="9999 Error: Illegal immediate operand; treated as 999";
                                    }
                                    else
                                    {
                                        outputAdd=val;
                                        s=to_string(outputAdd);
                                    }
                                }
                                break;
                              case 'R':
                                {
                                    int cmp=moduleBaseTable[module_count+1]-moduleBaseTable[module_count];
                                    
                                    if(operand>=cmp)
                                    {
                                        outputAdd=(val-operand)+moduleBaseTable[module_count];
                                        s=to_string(outputAdd)+" Error: Relative address exceeds module size; relative zero used";
                                    }
                                    else{
                                        outputAdd=val+moduleBaseTable[module_count];
                                        s=to_string(outputAdd);
                                    }
                                }
                                break;
                              case 'M':
                                {
                                    if(operand>=(moduleBaseTable.size()-1))
                                    {
                                         
                                        outputAdd=(val-operand);
                                        s=to_string(outputAdd)+" Error: Illegal module operand ; treated as module=0";
                                    }
                                    else
                                    {
                                        outputAdd=(val-operand) + moduleBaseTable[operand];
                                        s=to_string(outputAdd);
                                    }
                                }
                                break;
                        
                              default:
                                {}
                            }
                            while(s.size()<4)
                            {
                                s='0'+s;
                            }
                            cout<<s1<<": "<<s<<endl;
                            memCount++;
                        }
                      
                      instcount--;
                      readSymbol=false;
                  }
            }
            if(instcount==0){
                
                for(int i=0;i<useCountIndex.size();i++)
                {
                    if(accesedValues[i]==false)
                    {
                        cout<<"Warning: Module "<<module_count<<": uselist["<<i<<"]="<<useCountIndex[i]<<" was not used"<<endl;
                    }
                }
                defList=false;
                useList=false;
                progText=false;
                readDefCount=false;
                readInstrCount=false;
                module_count++;
                useCountIndex.clear();
                accesedValues.clear();
            }
            
            if((int)(str[index])==13)
            {
                break;
            }
        }
        index++;
        lastOffset=index;
    }
    
}
    
    for(int i=0;i<module_count;i++)
    {
        vector<string>s1=symbolCount[i];
        for(string s: s1)
        {
            if(!usedSym.count(s))
            {
                cout<<endl;
                cout<<"Warning: Module "<<i<<": "<<s<<" was defined but never used";
            }
        }
    }


}

bool pass1(const std::string& filename, unordered_map<string,int>&defTable, unordered_map<int,int>&moduleBaseTable)
{
  
    unordered_map<string,bool>symbolRedefined;
    vector<string>symbols;
    
    moduleBaseTable.insert({0,0});
    
    int total_instr=0;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return true;
    }
    std::string str;
    int lineNumber = 0;
    int lastOffset=0;
    
    int defcount=0;
    int instcount=0;
    int usecount=0;
    bool readDefCount=false,readSymbol=false,readInstrCount=false;
    bool defList=false, useList=false, progText=false;
    int module_count=0;
    vector<string>symbolTable;
    
    while (std::getline(file, str)) {
       
    lineNumber++;
    int index=0;
        
    while(index<str.size())
    {
        if(defList == false && index<str.size())
        {
            if(!readDefCount){
                defcount=readInt(index, str);
                if(defcount==0)
                {
                    readDefCount=false;
                    defList=true;
                }
                else if(defcount>16)
                {
                    int count_digits=0;
                    while(defcount>0)
                    {
                        defcount=defcount/10;
                        count_digits++;
                    }
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index-count_digits+1)<<": TOO_MANY_DEF_IN_MODULE"<<endl;
                    return true;
                }
                else if(defcount>0)
                    readDefCount=true;
                
                else if(defcount==-1)
                    break;
                    
                else if(defcount== -3)
                {
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": NUM_EXPECTED"<<endl;
                    return true;
                }
            }
           
            for(int i=0;i<defcount;)
            {
                string sym="";
                if(!readSymbol)
                {
                    sym=readSym(index, str);
                    if(sym=="" || sym=="end")
                    {
                        break;
                    }
                    else if(sym=="found num")
                    {
                        cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": SYM_EXPECTED"<<endl;
                        return true;
                    }
                    readSymbol=true;
                }
                    int val=readInt(index, str);
                    if(val>0 || (val==0 && defcount>0))
                    {
                       
                            if(defTable.count(sym))
                            {
                                cout<<"Warning: Module "<<module_count<<": "<<sym<<" redefinition ignored"<<endl;
                                symbolRedefined.insert({sym,true});
                            }
                            else{
                                symbols.push_back(sym);
                                defTable.insert({sym,val});
                                symbolTable.push_back(sym);
                            }

                        readSymbol=false;
                        defcount--;
                        defList=(defcount==0);
                    }
                    else if(val== -3)
                    {
                        cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": NUM_EXPECTED"<<endl;
                        return true;
                    }
                    
                    else if(val == -1){
                        break;
                    }
            }
        
    }
        
        else if(useList==false && index<str.size()){
            
                if(usecount==0)
                    usecount = readInt(index, str);
                
                if( usecount== -3)
                {
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": NUM_EXPECTED"<<endl;
                    return true;
                }
                
                if(usecount== -1)
                {
                    break;
                }
                if(usecount>16 ){
                    int count_digits=0;
                    while(usecount>0)
                    {
                        usecount=usecount/10;
                        count_digits++;
                    }
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index-count_digits+1)<<": TOO_MANY_USE_IN_MODULE"<<endl;
                    return true;
                }
                
                for (int i=0;i<usecount;) {
                    string sym = readSym(index, str); 
                    if(sym=="end" || sym=="")
                        break;
                    else if(sym=="found num")
                        {
                            cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": SYM_EXPECTED"<<endl;
                            return true;
                        }
                        else if(sym.size()>16)
                        {
                            cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": SYM_TOO_LONG"<<endl;
                            return true; 
                        }
                    else
                        {
                            usecount--;
                        }
                }
                useList=(usecount==0);
        }

        else if(progText == false && index<str.size())
        {
            if(!readInstrCount){
                readSymbol=false;
                instcount = readInt(index, str);
                
                if((total_instr+instcount)>512){
                    int count_digits=0;
                    while(instcount>0)
                    {
                        instcount=instcount/10;
                        count_digits++;
                    }
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index-count_digits+1)<<": TOO_MANY_INSTR"<<endl;
                    return true;
                }
                if(instcount == -3)
                {
                    cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": NUM_EXPECTED"<<endl;
                    return true;
                }
                
                    for(string s: symbols)
                    {
                        int value=defTable[s];
                        if(value>instcount)
                        {
                            int temp=instcount-1;
                            cout<<"Warning: Module "<<(module_count)<<": "<<s<<"="<<value<<" valid=[0.."<<temp<<"] assume zero relative"<<endl;
                            value=0;
                        }
                        defTable[s]=(total_instr+value);
                        
                    }
                moduleBaseTable.insert({module_count, total_instr});
                total_instr=total_instr+instcount;
                readInstrCount=true;
            }
            
            for(int i=0;i<instcount;)
            {
                    if(!readSymbol){
                        string str1=readSym(index, str);
                        if(str1=="end")
                        {
                            break;
                        }
                        // if(str1=="found num")
                        // {
                        //     cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": SYM_EXPECTED"<<endl;
                        //     return true;
                        // }
                        if(str1.size()>1)
                        {
                             cout<<"Parse Error line "<<lineNumber<<" offset "<<(index-str.size()+1)<<": MARIE_EXPECTED"<<endl;
                             return true;
                        }
                        if(str1!="")
                        {
                            char ch=str1[0];
                            readSymbol=true;
                        }
                    }
                  if(readSymbol){
                      int val=readInt(index, str);
                      if(val == -1){
                          break;
                      }
                      else if(val==-3)
                      {
                          cout<<"Parse Error line "<<lineNumber<<" offset "<<(index+1)<<": NUM_EXPECTED"<<endl;
                          return true;
                      }
                      instcount--;
                      readSymbol=false;
                  }
            }
            if(instcount==0){
                defList=false;
                useList=false;
                progText=false;
                readDefCount=false;
                readInstrCount=false;
                module_count++;
                symbols.clear();
            }
            
            if((int)(str[index])==13)
            {
                break;
            }
        }
        index++;
        lastOffset=index;
    }
    
}
if(defcount>0)
{
    cout<<"Parse Error line "<<lineNumber<<" offset "<<lastOffset<<": SYM_EXPECTED";
    return true;
}
if(usecount>0)
{
    cout<<"Parse Error line "<<lineNumber<<" offset "<<lastOffset<<": SYM_EXPECTED";
    return true;
}
if(instcount>0)
{
    cout<<"Parse Error line "<<lineNumber<<" offset "<<lastOffset<<": MARIE_EXPECTED";
    return true;
}

moduleBaseTable.insert({module_count, total_instr});


cout<<"Symbol Table"<<endl;
for(int i=0;i<symbolTable.size();i++)
{
    string s1=symbolTable[i];
    if(symbolRedefined[s1]==true)
        cout<<s1<<"="<<defTable[s1]<<" Error: This variable is multiple times defined; first value used"<<endl;
    else
        cout<<s1<<"="<<defTable[s1]<<endl;
}

return false;
}

int main(int argc, char *argv[]) {
   
    unordered_map<string, int>defTable;
    unordered_map<int,int>moduleBaseTable;
    string input=argv[1];
    bool foundError=pass1(input,defTable,moduleBaseTable);
    
    if(!foundError)
    {
        pass2(input,defTable,moduleBaseTable);
    }
    return 0;
}
