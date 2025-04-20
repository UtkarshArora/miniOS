#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;

const int NUM_PAGES = 64;
// int hand=-1;
unsigned int total_proc_exits=0;
int frame_count=0;
int frames_used=0;
char algo_spec='a';
bool processExit=false;
unsigned int instr = 0;
unsigned long long int instr_count=0;
unsigned long long int contextS=0;
unsigned long long int cost=0;
bool oFlag=false, pFlag=false, fFlag=false, sFlag=false;

int ofs=0;
vector<int>randVals;
int myrandom(int burst) { 
    
    if(ofs >= randVals.size()) ofs = 0;
    int num=(randVals[ofs++] % burst); 
    return num;
}

struct page_t{
   unsigned int Present : 1;
   unsigned int Ref : 1;
   unsigned int Modified : 1;
   unsigned int Write_Prot : 1;
   unsigned int FileMapped : 1;
   unsigned int Paged_out : 1;
   unsigned int extra : 8;
   int hole : 2;
   int frame_num : 16;
} ;

struct frame_t{
    unsigned int extra : 22;
    int proc = 0;
    int page_num = 0;
    unsigned int age : 32;
    unsigned int lastAccessed = 0;
};

//vector<bitset<32> >frameTable;
vector<frame_t>frameTable1;
vector<int>freeEntries;
int nextAv=-1;

class pStats{
            public:
            long unsigned int unmaps,maps,ins,outs,fins,fouts,zeros,segv,segprot,rw;
            pStats()
            {
                unmaps=0;
                maps=0;
                ins=0;
                outs=0;
                fins=0;
                fouts=0;
                zeros=0;
                segv=0;
                segprot=0;
                rw=0;
            }
        };



bool checkforAF(string& str, const string& prefix) {
    return str.rfind(prefix, 0) == 0;
}
bool checkforFlags(string& str, const string& prefix)
{
    return str.find(prefix) != string::npos;
}

class Process{
    public:
        int proc_num, page_start, page_end, readWrite, fileMapped;
        bool exit;
        //vector<bitset<32> >pageT;
        page_t pageTable[NUM_PAGES];
        vector<vector<int> >vmas;
        
        pStats* procStats=new pStats();

    Process(int proc_num, int st_page, int end_page, int readWrite, int fileMapped)
    {
        this->proc_num=proc_num;
        vector<int>v1;
        v1.push_back(st_page);
        v1.push_back(end_page);
        v1.push_back(readWrite);
        v1.push_back(fileMapped);
        vmas.push_back(v1);
        exit=false;

        for(int i=0;i<NUM_PAGES;i++)
        {
            pageTable[i].Present=0;
            pageTable[i].Ref=0;
            pageTable[i].Modified=0;
            pageTable[i].frame_num=0;
            pageTable[i].FileMapped=0;
            pageTable[i].Paged_out=0;
            pageTable[i].Write_Prot=0;
            pageTable[i].extra=0;
            pageTable[i].hole=-1;
        }
        // if(pageT.size()!=NUM_PAGES)
        //     pageT.resize(NUM_PAGES, fte);
    }

};


Process* CURRENT_RUNNING_PROCESS=NULL;
Process* PREV_PROCESS=NULL;
vector<Process*>processes;

void printPageTable(Process* p1)
{
    for(int i=0;i<NUM_PAGES;i++)
    {
        //cout<<"Process num:"<<p1->proc_num<<",page num:"<<i<<",P:"<<p1->pageTable[i].Present<<",R:"<<p1->pageTable[i].Ref<<",M:"<<p1->pageTable[i].Modified<<endl;
    }
}


class Pager{
    public:
    virtual int select_victim_frame(){
        return 0;
    };
};
Pager* p=NULL;

class FIFO_Scheduler: public Pager{

    private:
        int hand;
    
    public:
            FIFO_Scheduler()
            {
                this->hand=-1;
            }
            int select_victim_frame() override{
            int s2=frame_count;
            this->hand=(this->hand+1)%s2;
            return this->hand;
        }
};

class Random_Scheduler: public Pager{
    // private:
    //     int hand;
    public:
        int select_victim_frame(){
            int s1=frame_count;
            int hand=myrandom(s1);
            return hand;
        }
};

class Clock_Scheduler: public Pager{
    private:
        int hand;
    public:
        Clock_Scheduler()
        {
            this->hand=0;
        }
        int select_victim_frame(){
            int s1=frame_count;
            int page_num=frameTable1[hand].page_num;
            int proc=frameTable1[hand].proc;
            while(1){
                
                if(processes[proc]->pageTable[page_num].Ref == 0)
                {
                    int num=hand;
                    hand=(hand+1)%s1;
                    //cout<<"Clock schduler victim frame with ref 0:"<<num<<endl;
                    return num;
                }
                    
                else
                {
                    
                        processes[proc]->pageTable[page_num].Ref = 0;
                        hand=(hand+1)%s1;
                        proc=frameTable1[hand].proc;
                        page_num=frameTable1[hand].page_num;

                    // cout<<"Clock schduler victim frame:"<<hand<<endl;
                    // return hand;
                }
            }

            return hand;
        }
};
class NRU_Scheduler: public Pager{
    private:
        int hand;
        public:
            int comp=48;
            int interval=48;
        NRU_Scheduler()
        {
            hand=0;
        }
        bool checkforlastReset()
        {
            //cout<<"Instr count:"<<instr_count<<", last reset:"<<last_reset<<endl;
    
            if(instr_count>=comp)
            {
                comp=instr_count+interval;
                return true;
            }
            return false;
        }

        int select_victim_frame(){
            vector<int>classIndex(4,-1);
            //count=0;
            //int lowest=4;
            bool reset = false;
            int s1=frame_count;
            int victim=-1;
            reset=checkforlastReset();
            if(reset)
            {
                //cout<<"Last reset:"<<comp<<endl;
            }
            //cout<<"Selecting the victim frame, outside loop";
            for(int i=0;i<s1;i++)
            {
                int proc=frameTable1[hand].proc;
                int page_num=frameTable1[hand].page_num;

                int classVal = 2* processes[proc]->pageTable[page_num].Ref + processes[proc]->pageTable[page_num].Modified;
                //cout<<"Hand"<<hand<<",Frame Table index:"<<i<<", process:"<<proc<<",page_num:"<<page_num<<",classVal:"<<classVal<<endl;
                if(classIndex[classVal] == -1)
                {
                    classIndex[classVal]=hand;
                    //count++;
                }
                
                if(reset)
                {
                    processes[proc]->pageTable[page_num].Ref=0;
                }
                //cout<<"Hand:"<<hand<<",Inside loop,process:"<<proc<<", page num:"<<page_num<<",classVal:"<<classVal<<endl;
                // if(count==4)
                //     break;
                hand=(hand+1)%s1;
            }
            //printPageTable(CURRENT_RUNNING_PROCESS);

            for(int i=0;i<4;i++)
            {
                if(classIndex[i]!=-1)
                {
                    hand=(classIndex[i]+1)%s1;
                    victim= classIndex[i];
                    break;
                }
            }
            return victim;
            
            // int s1=frameTable.size();
            // hand=(hand+1)%s1;
            // bitset<32> temp=frameTable[hand];
            //frameTable[hand]=    assign the current page here
            //incorrect code in next line
            //return hand;
        }
        // Reset()
        // {

        // }
};
class Ageing_Scheduler: public Pager{
    public:
        int hand;
        Ageing_Scheduler()
        {
            hand=0;
        }
        int select_victim_frame(){
            int s1=frame_count;
            int victim = -1;
            for(int i=0;i<s1;i++)
            {
                frameTable1[hand].age = frameTable1[hand].age >> 1 ;
                
                if(processes[frameTable1[hand].proc]->pageTable[frameTable1[hand].page_num].Ref==1)
                {
                    frameTable1[hand].age=(frameTable1[hand].age | 0x80000000);
                    processes[frameTable1[hand].proc]->pageTable[frameTable1[hand].page_num].Ref=0;
                }
                if(victim == -1 || (frameTable1[hand].age < frameTable1[victim].age ))
                {
                    victim = hand;
                }
                hand=(hand+1)%s1;
            }
            hand=(victim+1)%s1;
            return victim;
        }
};
class WorkingSet_Scheduler: public Pager{
    public:
        int hand;
    
    WorkingSet_Scheduler()
    {
        hand=0;
    }
        int select_victim_frame(){
             
            int victim=-1;
            int s1=frame_count;
            
            for(int i=0;i<s1;i++)
            {
                int diff=instr_count-frameTable1[hand].lastAccessed;
                if(processes[frameTable1[hand].proc]->pageTable[frameTable1[hand].page_num].Ref ==1)
                {
                    processes[frameTable1[hand].proc]->pageTable[frameTable1[hand].page_num].Ref = 0;
                    frameTable1[hand].lastAccessed=instr_count;
                    if(victim==-1)
                    {
                        victim=hand;
                    }
                }
                else
                {
                    if(diff >= 50)
                    {
                        victim=hand;
                        break;
                    }
                    else{
                        if(victim==-1 || (frameTable1[hand].lastAccessed < frameTable1[victim].lastAccessed))
                        {
                            victim = hand;
                        }
                    }
                }
                hand=(hand+1)%s1;
            }
            hand=(victim+1)%s1;
            return victim;
        }
};

int readInt(string s1, int index)
{
    int num = 0;
    
    while (index < s1.size())
    {
        if (isdigit(s1[index]))
        {
            int start = index;
            while (index < s1.size() && isdigit(s1[index]))
            {
                index++;
            }
            int len = index - start;
            string str1 = s1.substr(start, len);
            num = stoi(str1);
            return num;
        }
        else
            index++;
    }
    return num;
}

bool checkforVmas(int instr)
{
    if(CURRENT_RUNNING_PROCESS->pageTable[instr].hole!=-1)
    {
        if(CURRENT_RUNNING_PROCESS->pageTable[instr].hole==1)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    vector<vector<int> >v1=CURRENT_RUNNING_PROCESS->vmas;
    bool ans=false;
    for(int i=0;i<v1.size();i++)
    {
        vector<int>v=v1[i];
        if(instr>=v[0] && instr<=v[1])
        {
            ans = true;
            CURRENT_RUNNING_PROCESS->pageTable[instr].hole=0;
            if(v[2]==1)
            {
                CURRENT_RUNNING_PROCESS->pageTable[instr].Write_Prot=1;
            }
            if(v[3]==1)
            {
                CURRENT_RUNNING_PROCESS->pageTable[instr].FileMapped=1;
            }
            return true;
        }
        
    }
    CURRENT_RUNNING_PROCESS->pageTable[instr].hole=1;
    return false;
}



bool checkforFileMapped(Process* p1, int page_num)
{
    bool ans = (p1->pageTable[page_num].FileMapped==1);
    return ans;
}
bool checkforWriteProtected(int page_num)
{
    bool ans = (CURRENT_RUNNING_PROCESS->pageTable[page_num].Write_Prot==1);
    return ans;
}
void printPTE(Process* p1, int page_num){
    
    cout<<"Process:"<<p1->proc_num<<" ,P:"<<p1->pageTable[page_num].Present<<",R:"<<p1->pageTable[page_num].Ref<<",M:"<<p1->pageTable[page_num].Modified<<",Paged_Out:"<<p1->pageTable[page_num].Paged_out<<",Frame Num:"<<p1->pageTable[page_num].frame_num;
    printf("\n");
}
void printFTE(int hand)
{
    frame_t fte = frameTable1[hand];
    cout<<"FTE:"<<fte.proc<<" "<<fte.page_num;
    printf("\n");
}

void pageReplacement(Pager* p, int page_num, char ch)
{
    //cout<<"Inside page replacement"<<endl;
    int frame_number = p->select_victim_frame();
    //cout<<"Victim frame selected:"<<frame_number<<endl;
    //frame_t fte = frameTable1[frame_number];
    int proc_num=frameTable1[frame_number].proc;
    int curr=frameTable1[frame_number].page_num;
    Process* p1=processes[proc_num];

        
    CURRENT_RUNNING_PROCESS->procStats->maps++;

    p1->procStats->unmaps++;
    if(oFlag){
        cout << " UNMAP ";
        cout << proc_num << ":" << curr;
        printf("\n");
    }
    
            //code to check for current page residing in frame Table

            //check for file map of current page in frame table
            
            //p1->pageTable[curr].Paged_out=0;

            bool ans1=checkforFileMapped(p1, curr);
            if(p1->pageTable[curr].Modified==1)
            {
                if(ans1)
                {
                    if(oFlag){
                    cout<< " FOUT" ;
                    printf("\n");
                    }
                    p1->procStats->fouts++;
                    //p1->pageTable[curr].Modified = 0;
                }
                else
                {
                    if(oFlag){
                    cout << " OUT";
                    printf("\n");
                    }
                    //p1->pageTable[curr].Modified = 0;
                    //cout<<p1->proc_num<<" "<<curr<<" paged_out is set"<<endl;
                    p1->pageTable[curr].Paged_out = 1;
                    p1->procStats->outs++;
                }
                
            }
    
            
                p1->pageTable[curr].Present = 0;
                p1->pageTable[curr].Ref = 0;
                p1->pageTable[curr].frame_num=0;
                p1->pageTable[curr].Modified=0;
            
            
            CURRENT_RUNNING_PROCESS->pageTable[page_num].Present=1;
            CURRENT_RUNNING_PROCESS->pageTable[page_num].Ref=1;
            CURRENT_RUNNING_PROCESS->pageTable[page_num].frame_num=frame_number;
            //printPTE(CURRENT_RUNNING_PROCESS,page_num);
            //code for incoming page_num
            bool ans=checkforFileMapped(CURRENT_RUNNING_PROCESS, page_num);
            //code for file mapped page coming in
            
            //code for paged out page coming in
            //printPTE(page_num);
            if(CURRENT_RUNNING_PROCESS->pageTable[page_num].Paged_out == 1)
            {
                if(oFlag){
                    cout << " IN";
                    printf("\n");
                }
                    CURRENT_RUNNING_PROCESS->procStats->ins++;
            }
            else if(ans)
            {
                if(oFlag){
                    cout<<" FIN";
                    printf("\n");
                }
                //CURRENT_RUNNING_PROCESS->pageTable[page_num].Paged_out=0;
                CURRENT_RUNNING_PROCESS->procStats->fins++;
            }
            else
            {
                if(oFlag){
                    cout << " ZERO";
                    printf("\n");
                }
                CURRENT_RUNNING_PROCESS->procStats->zeros++;
            }
        
    if(oFlag){
        cout << " MAP " << frame_number;
        printf("\n");
    }
    
    // fte.page_num=page_num;
    // fte.proc=CURRENT_RUNNING_PROCESS->proc_num;
    frameTable1[frame_number].page_num=page_num;
    frameTable1[frame_number].proc=CURRENT_RUNNING_PROCESS->proc_num;
    frameTable1[frame_number].lastAccessed=instr_count;
    frameTable1[frame_number].age = 0;
    //printFTE(frame_number);
    if(ch=='w'){
        bool WriteP=(CURRENT_RUNNING_PROCESS->pageTable[page_num].Write_Prot==1);
        if(!WriteP){
            CURRENT_RUNNING_PROCESS->pageTable[page_num].Modified=1;
            // if(page_num == 24){
            //     cout<<"Set modified bit for proc "<<CURRENT_RUNNING_PROCESS->proc_num<<"and page"<<page_num<<endl;
            // }
            //cout<<"Set modified bit for proc "<<CURRENT_RUNNING_PROCESS->proc_num<<"and page"<<page_num<<endl;
        }
        else
        {
            if(oFlag){
                cout<<" SEGPROT";
                printf("\n");
            }
            CURRENT_RUNNING_PROCESS->procStats->segprot++;
        }
    }
   
}



void pageAllocation(Pager*p,  char ch, int page_num)
{
    //string page_num_str=findBinary(page_num);
    //cout<<"Inside page allocation"<<endl;
    int target_frame = -1;
    
        if(processExit)
        {
            if(nextAv==-1)
            {
                pageReplacement(p,ch,page_num);
                return;
            }
            else
            {
                target_frame = freeEntries[nextAv];
                nextAv=nextAv+1;
                if(nextAv>=freeEntries.size())
                    {
                        nextAv=-1;
                        processExit=false;
                    }
            }
        }
        else
        {
            target_frame=frames_used;
        }
    

    bool ans=checkforFileMapped(CURRENT_RUNNING_PROCESS, page_num);
    //frame_t fte;
    
    frameTable1[target_frame].page_num=page_num;
    frameTable1[target_frame].proc=CURRENT_RUNNING_PROCESS->proc_num;
    frameTable1[target_frame].age = 0;
    frameTable1[target_frame].lastAccessed=instr_count;
    
    if(ans)
    {
        if(oFlag){
            cout<<" FIN";
            printf("\n");
        }
        CURRENT_RUNNING_PROCESS->procStats->fins++;
    }
    else if(CURRENT_RUNNING_PROCESS->pageTable[page_num].Paged_out==1)
    {
        if(oFlag){
            cout<<" IN";
            printf("\n");
        }
        CURRENT_RUNNING_PROCESS->procStats->ins++;
    }
    else
    {
        if(oFlag){
            cout<<" ZERO";
            printf("\n");
        }
        CURRENT_RUNNING_PROCESS->procStats->zeros++;
    }
    //printPTE(CURRENT_RUNNING_PROCESS,page_num);
    CURRENT_RUNNING_PROCESS->pageTable[page_num].frame_num=target_frame;
    CURRENT_RUNNING_PROCESS->pageTable[page_num].Present=1;
    CURRENT_RUNNING_PROCESS->pageTable[page_num].Ref=1;
   
    if(oFlag){
        cout << " MAP " << target_frame << endl;
    }
    CURRENT_RUNNING_PROCESS->procStats->maps++;
    frames_used++;
    if(ch=='w'){
        bool WriteP=checkforWriteProtected(page_num);
        if(!WriteP){
            CURRENT_RUNNING_PROCESS->pageTable[page_num].Modified=1;
           
        }
        else
        {
            if(oFlag){
                cout<<" SEGPROT";
                printf("\n");
            }
            CURRENT_RUNNING_PROCESS->procStats->segprot++;
        }
    }
    return;
   
}
void printProcStats(Process* p1)
{

    pStats* pstats=p1->procStats;
        
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                     p1->proc_num,
                     pstats->unmaps, pstats->maps, pstats->ins, pstats->outs,
                     pstats->fins, pstats->fouts, pstats->zeros,
                     pstats->segv, pstats->segprot);
        printf("\n");;
}

void clearPageTable(int proc_num)
{
    if(oFlag){
        cout<<"EXIT current process "<<proc_num;
        printf("\n");
    }
    Process* p1=processes[proc_num];
    p1->exit=true;
    for(int i=0;i<=63;i++)
    {
        if(p1->pageTable[i].Present==1)
        {
            p1->procStats->unmaps++;
            if(oFlag){
                cout << " UNMAP ";
                cout << proc_num << ":" << i;
                printf("\n");
            }
    
            //code to check for current page residing in frame Table

            //check for file map of current page in frame table
            
            //p1->pageTable[curr].Paged_out=0;

            bool ans1=checkforFileMapped(p1, i);
            if(p1->pageTable[i].Modified==1)
            {
                if(ans1)
                {
                    if(oFlag){
                        cout<< " FOUT" << endl;
                    }
                    p1->procStats->fouts++;
                }
            }
    
                int frame_num=p1->pageTable[i].frame_num;
                if((nextAv+1)<freeEntries.size())
                {
                    //cout<<"Next Av "<<nextAv<<", freeEntries size:"<<freeEntries.size()<< endl;
                    nextAv=nextAv+1;
                    freeEntries[nextAv]=frame_num;
                    //cout<<"NextAv "<<nextAv<<" "<<freeEntries[nextAv]<<" frame num"<<frame_num<<endl;
                }
                else
                {
                    freeEntries.push_back(frame_num);
                    nextAv++;
                    //cout<<"NextAv "<<(nextAv)<<" "<<freeEntries[nextAv]<<" frame number"<<frame_num<<endl;
                }
                frameTable1[frame_num].proc=-1;
                frameTable1[frame_num].page_num=-1;
                p1->pageTable[i].Present = 0;
                p1->pageTable[i].Ref = 0;
                p1->pageTable[i].frame_num=-1;
                p1->pageTable[i].Modified = 0;
                p1->pageTable[i].Paged_out = 0;
                frames_used = frames_used-1;
                //printPTE(p1,i);
                
        }
    }
    nextAv=0;
    //CURRENT_RUNNING_PROCESS=NULL;
} 

void readInstructions(string line)
{
    char ch=line[0];
    int page_num=readInt(line, 1);
    if(oFlag){
        cout<<instr_count<<": ==> "<<line;
        printf("\n");
    }
    instr_count++;
   // if(CURRENT_RUNNING_PROCESS!=NULL)
     //   cout<<"CURRENT RUNNING PROCESS"<<CURRENT_RUNNING_PROCESS->proc_num<<endl;
    switch (ch)
    {
    case 'c':
        {
            if(CURRENT_RUNNING_PROCESS!=NULL){
                PREV_PROCESS=CURRENT_RUNNING_PROCESS;
                //processChanged=true;
                //printProcStats(PREV_PROCESS);
            }
            CURRENT_RUNNING_PROCESS=processes[page_num];   
            contextS++;
        }
        break;
    case 'r':
        {
            CURRENT_RUNNING_PROCESS->procStats->rw++;
            if(!checkforVmas(page_num))
            {
                if(oFlag){
                    cout<<" SEGV";
                    printf("\n");
                }
                CURRENT_RUNNING_PROCESS->procStats->segv++;
            }

            else if(CURRENT_RUNNING_PROCESS->pageTable[page_num].Present!=1)
            {
                //page fault
                //write code for page fault here
                if(frames_used<frame_count)
                {
                        
                    pageAllocation(p, 'r', page_num);
                        
                }
                else{
                    
                    pageReplacement(p, page_num, 'r');
            }
        }
        else{
            CURRENT_RUNNING_PROCESS->pageTable[page_num].Ref=1;

        }
        break;
    case 'w':
        {
            CURRENT_RUNNING_PROCESS->procStats->rw++;
            if(!checkforVmas(page_num))
            {
                if(oFlag){
                    cout<<" SEGV";
                    printf("\n");
                }
                CURRENT_RUNNING_PROCESS->procStats->segv++;
            }
            // else if(CURRENT_RUNNING_PROCESS->pageT[page_num][0]==1)
            // {
            //     CURRENT_RUNNING_PROCESS->pageT[page_num][1]=1;
            // }
            else if(CURRENT_RUNNING_PROCESS->pageTable[page_num].Present!=1)
            {
                //page fault
                //write code for page fault here
                if(frames_used<frame_count)
                {
                        
                    pageAllocation(p, 'w', page_num);
                           
                }
                else{
                    pageReplacement(p, page_num, 'w');
            }
            }

            else{
                CURRENT_RUNNING_PROCESS->pageTable[page_num].Ref=1;

                if(ch=='w'){
                    bool WriteP=checkforWriteProtected(page_num);
                    if(!WriteP){
                        CURRENT_RUNNING_PROCESS->pageTable[page_num].Modified=1;
                        // if(page_num == 24){
                        //     cout<<"Set modified bit for proc "<<CURRENT_RUNNING_PROCESS->proc_num<<"and page"<<page_num<<endl;
                        // }
                    }
                    else
                    {
                        if(oFlag){
                            cout<<" SEGPROT";
                            printf("\n");
                        }
                        CURRENT_RUNNING_PROCESS->procStats->segprot++;
                    }
                }
                //printPTE(CURRENT_RUNNING_PROCESS,page_num);
                
            }
        }
        break;
    case 'e':
        {
            clearPageTable(page_num);
            processExit=true;
            CURRENT_RUNNING_PROCESS=NULL;
            total_proc_exits++;
        }
        break;
    default:
        break;
    }
}
}
void printProcessTable()
{
    for(int i=0;i<processes.size();i++)
    {
        Process* p1=processes[i];
        //cout<<"Process num:"<<p1->proc_num<<endl;
        vector<vector<int> >v1=p1->vmas;

        for(int i=0;i<v1.size();i++)
        {
            vector<int>v=v1[i];
            //cout<<"VMA "<<i<<endl;
            //cout<<v[0]<<" "<<v[1]<<" "<<v[2]<<" "<<v[3]<<endl;
        }
        
    }
}

int main(int argc, char* argv[]){

    
    string input_file;
    string rand_file;
    unordered_map<string, int> costMap;
    costMap["RW"]=1;
    costMap["CONTEXT"]=130;
    costMap["EXITS"]=1230;
    costMap["MAPS"]=350;
    costMap["UNMAPS"]=410;
    costMap["INS"]=3200;
    costMap["OUTS"]=2750;
    costMap["FINS"]=2350;
    costMap["FOUTS"]= 2800;
    costMap["ZEROS"]=150;
    costMap["SEGV"]=440;
    costMap["SEGP"]=410;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (checkforAF(arg, "-f")) {
            frame_count=stoi(arg.substr(2));
            //cout<<"Frame count:"<<frame_count<<endl;
        } else if (checkforAF(arg, "-a")) {
            algo_spec = arg.substr(2)[0];  
        } else if(checkforAF(arg, "-o"))
        {
            //cout<<"Arg with o"<<arg<<endl;
            
                    oFlag=checkforFlags(arg, "O");
            
                    pFlag=checkforFlags(arg, "P");
          
                    fFlag=checkforFlags(arg, "F");
             
                    sFlag=checkforFlags(arg, "S");

        } 
        
        else if (arg[0] != '-') {
            
            if (input_file.empty()) {
                input_file = arg;
            } else if (rand_file.empty()) {
                rand_file = arg;
            }
        }
    }
    switch (algo_spec)
                    {
                    case 'a':
                        {
                            p=new Ageing_Scheduler();
                        }
                        break;
                    case 'f':
                        {
                            p=new FIFO_Scheduler();
                        }
                        break;
                    case 'r':
                        {
                            p=new Random_Scheduler();
                        }
                        break;
                    case 'c':
                        {
                            p=new Clock_Scheduler();
                        }
                        break;
                    case 'w':
                        {
                            p=new WorkingSet_Scheduler();
                        }
                        break;
                    case 'e':
                        {
                            p=new NRU_Scheduler();
                        }
                    
                    default:
                        break;
                    }


    
    if (input_file.empty() || rand_file.empty()) {
       // cerr << "Error: input file and random file must be specified.\n";
        return 1;
    }
    
    frameTable1.resize(frame_count);
    for(int i=0;i<frame_count;i++)
    {
        frameTable1[i].extra=0;
        frameTable1[i].proc=-1;
        frameTable1[i].page_num=-1;
        //frameTable1[i].age=0;
    }
    ifstream rFile(rand_file); 
    string line;
    if (!rFile)
    {
       //cerr << "Error: Rand File could not be opened!" << endl;
       return 1;
    }
    int count = 0;
    // Read the file line by line
    
    while (getline(rFile, line))
    {
        // int num=readInt(line);
        int num = stoi(line);
        if (count == 0)
        {
            randVals.resize(num);
        }
        else
        {
            randVals[count-1] = num;
        }
        count++;
    }
    rFile.close();

    ifstream inputFile(input_file);

    
    if (!inputFile)
    {
       //cerr << "Error: Input File could not be opened!" << endl;
       return 1;
    }
    //count = 0;
   
    int proc_count=-1;
    int current_VMA=0;
    bool firstVMA=false;
    int var1, var2, var3, var4;
    while (getline(inputFile, line))
    {
       
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
       
        std::istringstream iss(line);

        
        if (proc_count==-1) {
            if(iss >> var1 ) {
                proc_count=var1;
                //cout<<"Proc count"<<proc_count<<endl;
            }
        }
        else if (proc_count>0){
            
            // cout<<"Current VMA:"<<current_VMA<<endl;
            if(current_VMA==0)
            {
                if(iss >> var1)
                {
                    current_VMA=var1;
                    firstVMA=true;
                }
            }

            else if(iss >> var1 >> var2 >> var3 >> var4) {


                if(firstVMA){
                    int proc_num=processes.size();
                    //cout<<"Process Number:"<<proc_num<<",VMA number:"<<current_VMA<<endl;
                    Process* p1=new Process(proc_num, var1, var2, var3, var4);
                    processes.push_back(p1);
                    firstVMA=false;
                    //cout<<"Proc count" << proc_count<<endl;
                }
                else
                    {
                        int index=processes.size()-1;
                        //cout<<"Process num: "<<index<<",VMA number:"<<current_VMA<< endl;
                        
                        Process* p=processes[index];
                        vector<int>v1;
                        v1.push_back(var1);
                        v1.push_back(var2);
                        v1.push_back(var3);
                        v1.push_back(var4);
                        p->vmas.push_back(v1);
                    }
                ////cout<<"Page end:"<<p1->page_end<<endl;
                current_VMA--;
                firstVMA=!(current_VMA>0);
                proc_count=(current_VMA==0)?proc_count-1:proc_count;
                if(proc_count==0)
                {
                    //cout<<"Processes size: "<<processes.size()<<endl;
                    //printProcessTable();
                }
            } 
        }
        else if(proc_count==0){
            readInstructions(line);
        }
    }
    
    inputFile.close();
    //cout<<"Printing VMAS"<<endl;
    if(pFlag){
    for(int i=0;i<processes.size();i++)
    {
        Process* p=processes[i];
        vector<vector<int> >vmas=p->vmas;
        //cout<<"Process ID:"<<i<<endl;
        cout<<"PT["<<i<<"]: ";
        for(int i=0;i<64;i++)
        {
            //printPTE(p,i);
            if(p->exit==true)
            {
                cout<<"*";
            }
            else{

            if(p->pageTable[i].Present==1)
            {
                if(p->pageTable[i].Ref==1)
                {    
                    cout<<i<<":R";
                }
                else
                {
                    cout<<i<<":-";
                }
                if(p->pageTable[i].Modified==1)
                {
                    cout<<"M";
                }
                else
                {    
                    cout<<"-";
                }
                if(p->pageTable[i].Paged_out==1)
                {
                    cout<<"S";
                }
                else
                    cout<<"-";
                }
                
                else if(p->pageTable[i].Paged_out==1){
                        cout<<"#";
                }
                else
                {
                    cout<<"*";
                }
            }
            if(i<63){
                cout<<" ";
            }
            //printf("\n");;
        }
        if(i< (processes.size()-1))
            {
                printf("\n");
            }
    }
    printf("\n");
    //cout<<"FrameTable size:"<<frameTable1.size()<<endl;
    }
    if(fFlag){
    cout<<"FT:";
    for(int i=0;i<frame_count;i++)
    {
        if(frameTable1[i].proc!=-1)
            cout<<" "<<frameTable1[i].proc<<":"<<frameTable1[i].page_num;
        else
            cout<<" *";
    }
    printf("\n");
    }
    if(sFlag){
    for(int i=0;i<processes.size();i++)
    {
        Process* proc=processes[i];
        pStats* pstats=proc->procStats;
        //cout<<"Segprot:"<<pstats->segprot<<endl;
        
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                     proc->proc_num,
                     pstats->unmaps, pstats->maps, pstats->ins, pstats->outs,
                     pstats->fins, pstats->fouts, pstats->zeros,
                     pstats->segv, pstats->segprot);

        cost=cost+pstats->unmaps*costMap["UNMAPS"];
        cost=cost+pstats->maps*costMap["MAPS"];
        cost=cost+pstats->ins*costMap["INS"];
        cost=cost+pstats->outs*costMap["OUTS"];
        cost=cost+pstats->fins*costMap["FINS"];
        cost=cost+pstats->fouts*costMap["FOUTS"];
        cost=cost+pstats->zeros*costMap["ZEROS"];
        cost=cost+pstats->segv*costMap["SEGV"];
        cost=cost+pstats->segprot*costMap["SEGP"];
        //cout<<"Cost :"<<cost<<endl;
        cost=cost+pstats->rw;
}
        cost=cost+contextS*costMap["CONTEXT"];
        cost=cost+total_proc_exits*costMap["EXITS"];

    printf("TOTALCOST %lu %lu %lu %llu %lu\n", instr_count, contextS, total_proc_exits, cost, sizeof(page_t));

    }
return 0;

}

