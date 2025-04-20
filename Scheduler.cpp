#include <iostream>
#include <fstream> 
#include <string>
#include <vector> 
#include <deque>
#include <queue>
#include <cmath>
#include <iomanip>
using namespace std;

double total_IO=0;
pair<int,int>p1(-1,-1);
vector<int>randVals;
int ofs=1;
char sch='P';
int countP = 0;
int maxprios=4;

int myrandom(int burst) { 
    
    int num= 1 + (randVals[ofs] % burst); 
    int size1=randVals.size();
    ofs=(ofs+1)%size1;
    return num;
}

bool checkforS(string& str, const string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

enum State
{
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED,
    PREMPTED
};

enum Transit{
    TRANS_TO_READY,
    TRANS_TO_PREEMPT,
    TRANS_TO_RUN,
    TRANS_TO_BLOCK,
    TRANS_TO_FINISH
};

class SchType{
    public:
        char sch;
        int quantum;
        int numprios;

    SchType(char sch, int quantum, int numprios)
    {
        this->sch=sch;
        this->quantum=quantum;
        this->numprios=numprios;
    }
};

class Process
{

public:
    int process_id, quantum,cpu_burst, static_prio, dynamic_prio, total_IO, next_event_time;
    int Arrival_Time, Total_CPU, CPU_Burst, IO_Burst, remainingTimeC, remainingTimeI, state_ts, FT, TT, CW;
    State state;
    Process(int pid, int AT, int TC, int CB, int IO, int quantum, int prio)
    {
        this->Arrival_Time = AT;
        this->Total_CPU = TC;
        this->CPU_Burst = CB;
        this->IO_Burst = IO;
        this->remainingTimeC = TC;
        this->remainingTimeI = IO;
        this->process_id = pid;
        this->state = CREATED;
        this->state_ts=AT;
        this->FT=0;
        this->CW=0;
        this->total_IO=0;
        this->TT=0;
        this->quantum=quantum;
        this->cpu_burst=0;
        this->static_prio=prio;
        this->dynamic_prio=this->static_prio-1;
        this->next_event_time=0;
    }
};
vector<Process*>Psummary;

class EVENT{
    public:
        int evtTimeStamp;
        Process* evtProcess;
        Transit transition;

    EVENT(int t, Process* p, Transit transition)
    {
        this->evtTimeStamp=t;
        this->evtProcess=p;
        this->transition=transition;
    }
};


deque<EVENT*> eventQ;
int CURRENT_TIME=0;
Process* CURRENT_RUNNING_PROCESS=NULL;



class Scheduler
{
public:
    virtual void addProcess(Process *p1) {}
    virtual Process *get_next_process() { return NULL; }
    virtual bool testPrempt(Process* p1){ return false;}
};

class FCFS_Scheduler : public Scheduler
{
public:
    deque<Process *> runQ;
    void addProcess(Process *p)
    {
        runQ.push_back(p);
    }
    Process *get_next_process()
    {
        if (runQ.empty())
            return NULL;
        Process *p = runQ.front();
        runQ.pop_front();
        return p;
    }
    bool testPrempt(Process* p1)
    {
        return false;
    }

};
class RR_Scheduler : public Scheduler
{
public:
    deque<Process *> runQ;
    void addProcess(Process *p)
    {
        runQ.push_back(p);
        for(auto it=runQ.begin();it!=runQ.end();it++)
        {
            Process* p= *it;
        }
    }
    Process *get_next_process()
    {
        if (runQ.empty())
            return NULL;
        Process *p = runQ.front();
        runQ.pop_front();
        return p;
    }
    bool testPrempt(Process* p1)
    {
        return false;
    }

};
class Priority_Scheduler : public Scheduler
{
    private:
    deque<Process *> activeQ;
    deque<Process *> expiredQ;
    int countActive;
public:

    void addProcess(Process *p)
    {
        int prio=p->dynamic_prio;
        if(p->dynamic_prio==-1)
        {
            bool inserted=false;
            p->dynamic_prio=p->static_prio-1;
            prio=p->dynamic_prio;
            
            if(expiredQ.size()==0)
            {
                expiredQ.push_back(p);
                return;
            }
            
            for(auto it=expiredQ.begin();it!=expiredQ.end();)
            {
                Process* p1= *it;
                if(p->dynamic_prio>p1->dynamic_prio)
                {
                    expiredQ.insert(it,p);
                    inserted=true;
                    return;
                }
                else if(p->dynamic_prio<=p1->dynamic_prio)
                {
                    it++;
                }
                
            }
            if(!inserted)
            {
                expiredQ.push_back(p);
                
                return;
            }
        }
       
        bool inserted=false;
        
        if(activeQ.size()==0){
            activeQ.push_back(p);
            return;
        }
        
        for(auto it=activeQ.begin();it!=activeQ.end();)
        {
            Process* p1= *it;
            if(p->dynamic_prio>p1->dynamic_prio)
            {
                activeQ.insert(it,p);
                inserted=true;
                return;
            }
            else if(p->dynamic_prio<=p1->dynamic_prio)
            {
                it++;
            }
        }
        if(!inserted)
        {
            activeQ.push_back(p);
            return;
        }
    }
    Process *get_next_process()
    {

        if(activeQ.empty())
        {
            swap(activeQ,expiredQ);
        }
        if(activeQ.empty())
            return NULL;

        Process * p1=activeQ.front();
        activeQ.pop_front();
        return p1;

    }
    bool testPrempt(Process* p1)
    {
        return false;
    }

};

class EPriority_Scheduler : public Scheduler
{
public:
    deque<Process *> activeQ;
    deque<Process *> expiredQ;
    int prio;

    void addProcess(Process *p)
    {
        
        if(p->dynamic_prio==-1)
        {
            bool inserted=false;
            p->dynamic_prio=p->static_prio-1;
            if(expiredQ.size()==0)
            {
                expiredQ.push_back(p);
                return;
            }
            for(auto it=expiredQ.begin();it!=expiredQ.end();)
            {
                Process* p1= *it;
                if(p->dynamic_prio>p1->dynamic_prio)
                {
                    expiredQ.insert(it,p);
                    inserted=true;
                    return;
                }
                else if(p->dynamic_prio<=p1->dynamic_prio)
                {
                    it++;
                }
               
            }
            if(!inserted)
            {
                expiredQ.push_back(p);
                return;
            }
        }
        
        bool inserted=false;
       

        if(activeQ.size()==0){
            activeQ.push_back(p);
            return;
        }

        for(auto it=activeQ.begin();it!=activeQ.end();)
        {
            Process* p1= *it;
            if(p->dynamic_prio>p1->dynamic_prio)
            {
                activeQ.insert(it,p);
                inserted=true;
                break;
            }
            else if(p->dynamic_prio<=p1->dynamic_prio)
            {
                it++;
            }
        }
        if(!inserted)
        {
            activeQ.push_back(p);
            return;
        }
    }
    Process *get_next_process()
    {
        if (activeQ.empty())
        {
            swap(activeQ,expiredQ);
        }
        if(activeQ.empty())
            return NULL;
        Process *p = activeQ.front();
        activeQ.pop_front();
        return p;
    }
    bool testPrempt(Process* p1)
    {
        bool ans= (p1->dynamic_prio > CURRENT_RUNNING_PROCESS->dynamic_prio) && (CURRENT_RUNNING_PROCESS->next_event_time!=CURRENT_TIME);
        return ans;
    }

};

class SRTF_Scheduler : public Scheduler
{
public:
    deque<Process *> runQ;
    void addProcess(Process *p)
    {
        if(runQ.size()==0)
            runQ.push_back(p);
        else
        {
            bool inserted=false;
            for(auto it=runQ.begin();it!=runQ.end();it++)
            {
                Process* current= *it;
                if(current->remainingTimeC > p->remainingTimeC)
                {
                    inserted=true;
                    runQ.insert(it,p);
                    break;
                }
            }
            if(!inserted)
                runQ.push_back(p);
        }
        
    }
    Process *get_next_process()
    {
        if (runQ.empty())
            return NULL;
        Process *p = runQ.front();
        runQ.pop_front();
        return p;
    }
    bool testPrempt(Process* p1)
    {
        return false;
    }

};
class LCFS_Scheduler : public Scheduler
{
public:
    deque<Process *> runQ;
    void addProcess(Process *p)
    {
        runQ.push_front(p);
    }
    Process *get_next_process()
    {
        if (runQ.empty())
            return NULL;
        for(auto it=runQ.begin();it!=runQ.end();it++)
        {
            Process* p= *it;
        }
        Process *p = runQ.front();
        runQ.pop_front();

        return p;
    }
    bool testPrempt(Process* p1)
    {
        return false;
    }

};


class Simulation1
{
public:
    bool CALL_SCHEDULER;
    Scheduler* sched;

    Simulation1(Scheduler* sched) : sched(sched), CALL_SCHEDULER(false) {}
    
    EVENT* get_event()
    {
        if(eventQ.empty())
            return NULL;
        EVENT* evt=eventQ.front();
        eventQ.pop_front();
        return evt;
    }
    void addEvent(EVENT* e1)
    {
        //auto it=eventQ.begin();
                bool inserted=false;
                
                
                auto it=eventQ.begin();
                
                for (auto it=eventQ.begin(); it != eventQ.end(); ) 
                {
                    EVENT* e=*it;
                    if(e->evtTimeStamp <= e1->evtTimeStamp)
                        ++it;
                    else if(e->evtTimeStamp == e1->evtTimeStamp )
                    {
                            if(e1->evtProcess->process_id < e->evtProcess->process_id || e1->transition==4)
                                {
                                    eventQ.insert(it,e1);
                                    inserted=true;
                                    break;
                                }
                                else
                                {
                                    it++;
                                }
                    }
                    
                    else    
                        {
                            eventQ.insert(it,e1);
                            inserted=true;
                            break;
                            
                        }
                }
                
                if(!inserted)
                    eventQ.push_back(e1);
                
    }

    void deleteEvents(Process* p1)
    {
        for(auto it=eventQ.begin();it!=eventQ.end();it++)
        {
            EVENT* e1 = *it;
          
            if(e1->evtProcess == p1)
            {
                eventQ.erase(it);
                if(eventQ.size()==0 || it==eventQ.end())
                    return;
            }
        }
        
    }
    void Simulation()
    {
        EVENT *evt;
       
        while ((evt = get_event()))
        {
            Process *proc = evt->evtProcess; // this is the process the event works on
            CURRENT_TIME = evt->evtTimeStamp;
            int transition = evt->transition;
            int timeInPrevState = CURRENT_TIME - proc->state_ts; // for accounting
            //cout<<CURRENT_TIME<<" "<<proc->process_id<<" "<<timeInPrevState<<": ";
            delete evt;
            evt = nullptr; // remove cur event obj and don’t touch anymore
            switch (transition)
            { // encodes where we come from and where we go
            case TRANS_TO_READY:
                // must come from BLOCKED or CREATED'
                // add to run queue, no event created
                {
                    string s1="";
                    if(proc->state==0)
                        s1="CREATED";
                    else
                        s1="BLOCKED";
                    //cout<<s1<<" -> READY "<<endl; 
                    //proc->IT=proc->IT+timeInPrevState;
                    proc->state_ts=CURRENT_TIME;
                    bool ans=false;
                    if(CURRENT_RUNNING_PROCESS!=NULL)
                        ans=sched->testPrempt(proc);

                    if(ans)
                    {
                        deleteEvents(CURRENT_RUNNING_PROCESS);
                        int time_difference=CURRENT_RUNNING_PROCESS->next_event_time-CURRENT_TIME;
                        ////cout<<"Time difference "<<time_difference<<endl;
                        CURRENT_RUNNING_PROCESS->remainingTimeC=CURRENT_RUNNING_PROCESS->remainingTimeC+time_difference;
                        CURRENT_RUNNING_PROCESS->cpu_burst=CURRENT_RUNNING_PROCESS->cpu_burst+time_difference;
                        EVENT* e1=new EVENT(CURRENT_TIME,CURRENT_RUNNING_PROCESS,TRANS_TO_PREEMPT);
                        addEvent(e1);
                        CURRENT_RUNNING_PROCESS=NULL;
                    }
                    sched->addProcess(proc);
                    ////cout<<"Process added in Ready"<<endl;
                    CALL_SCHEDULER = true;
                }
                break;

            case TRANS_TO_PREEMPT: // similar to TRANS_TO_READY
                // must come from RUNNING (preemption)
                // add to runqueue (no event is generated)
                {
                    proc->state_ts=CURRENT_TIME;
                    proc->dynamic_prio=proc->dynamic_prio-1;
                    CURRENT_RUNNING_PROCESS=NULL;
                    sched->addProcess(proc);
                    proc->state=PREMPTED;
                    CALL_SCHEDULER = true;
                }
                break;
            case TRANS_TO_FINISH:
            {
                //cout<<CURRENT_TIME<<" DONE"<<endl;
                CURRENT_RUNNING_PROCESS->FT=CURRENT_TIME;
                CURRENT_RUNNING_PROCESS->TT=CURRENT_TIME-CURRENT_RUNNING_PROCESS->Arrival_Time;
                Psummary[CURRENT_RUNNING_PROCESS->process_id]=CURRENT_RUNNING_PROCESS;
                CURRENT_RUNNING_PROCESS=NULL;
                CALL_SCHEDULER=true;
            }
            break;
            case TRANS_TO_RUN:
               { 
                    ////////cout<<CURRENT_TIME<<" "<<proc->process_id<<endl;
                    int CB=proc->CPU_Burst;
                    int IO=proc->IO_Burst;
                    int cpu_burst=0;
                    // if(proc->state==5 && proc->cpu_burst==0)
                    // {
                    //     EVENT* e1=new EVENT(CURRENT_TIME+cpu_burst,proc,TRANS_TO_BLOCK);
                    //     addEvent(e1);
                    // }
                    if(proc->cpu_burst==0)
                    {
                        //cpu_burst=myrandom(CB);
                        if(proc->quantum==0){
                            proc->cpu_burst=myrandom(CB);
                            cpu_burst=proc->cpu_burst;
                        }
                        else
                        {
                            // int temp=cpu_burst;
                            proc->cpu_burst=myrandom(CB);
                            cpu_burst=(proc->quantum > proc->cpu_burst)?proc->cpu_burst:proc->quantum;
                            //proc->cpu_burst=cpu_burst;
                            //cpu_burst=(cpu_burst >= proc->quantum)? proc->quantum : cpu_burst;
                        }
                    }
                    else
                    {
                            // cpu_burst=proc->cpu_burst;
                            // proc->cpu_burst=(cpu_burst>=proc->quantum)?(cpu_burst-proc->quantum):0;
                            cpu_burst=(proc->cpu_burst >= proc->quantum)? proc->quantum : proc->cpu_burst;
                    }
                    // //////cout<<"CPU Burst:"<<cpu_burst<<endl;
                        if(proc->remainingTimeC > cpu_burst)
                        {
                            //cout<<"READY -> RUNNING"<<endl;
                            proc->CW=proc->CW+timeInPrevState;
                            proc->state_ts=CURRENT_TIME;
                            proc->state=(proc->quantum>0)?READY:BLOCKED;
                            ////cout<<"cb="<<proc->cpu_burst<<" rem="<<proc->remainingTimeC<<" prio="<<proc->dynamic_prio<<endl;
                            proc->cpu_burst=proc->cpu_burst-cpu_burst;
                            proc->remainingTimeC=proc->remainingTimeC-cpu_burst;
                            proc->next_event_time=CURRENT_TIME+cpu_burst;

                            if(proc->quantum>0 && proc->cpu_burst>0)
                            {
                                //////cout<<"CURRENT TIME:"<<CURRENT_TIME<<endl;
                                EVENT* e1=new EVENT(CURRENT_TIME+cpu_burst,proc,TRANS_TO_PREEMPT);
                                addEvent(e1);
                            }
                            else
                            {
                                //////cout<<"CPU Burst=0"<<endl;
                                EVENT* e1=new EVENT(CURRENT_TIME+cpu_burst,proc,TRANS_TO_BLOCK);
                                addEvent(e1);
                            }
                            //eventQ.push(e1);
                        }
                        else{
                            //cout<<"READY TO RUNNING SECOND CONDITION"<<endl;
                            proc->state_ts=CURRENT_TIME;
                            proc->CW=proc->CW+timeInPrevState;
                            proc->state=BLOCKED;
                            cpu_burst=proc->remainingTimeC;
                            proc->remainingTimeC=0;
                            proc->next_event_time=CURRENT_TIME+cpu_burst;
                            // proc->state=BLOCKED;
                            //int nextTime=CURRENT_TIME+cpu_burst;
                            EVENT* e1=new EVENT(CURRENT_TIME+cpu_burst,proc,TRANS_TO_FINISH);
                            //eventQ.push(e1);
                            addEvent(e1);
                            
                        }
                    
               }
                break;
            case TRANS_TO_BLOCK:
                // create an event for when process becomes READY again
                {
                    int IO=proc->IO_Burst;
                    //////cout<<"Offset : "<<randVals[ofs]<<endl;
                    int io_burst=myrandom(IO);
                    proc->total_IO=proc->total_IO+io_burst;
                    //cout<<"RUNNING -> BLOCKED "<<endl;
                    proc->state_ts=CURRENT_TIME;
                    proc->dynamic_prio=proc->static_prio-1;
                    int next_event_time=CURRENT_TIME+io_burst;
                    if(p1.first==-1 || CURRENT_TIME > p1.second)
                    {
                        total_IO=total_IO+(p1.second-p1.first);
                        p1.first=CURRENT_TIME;
                        p1.second=next_event_time;
                    }
                    else
                    {
                        if(CURRENT_TIME>=p1.first && next_event_time<=p1.second)
                        {
                            
                        }
                        else if(CURRENT_TIME>=p1.first && CURRENT_TIME<=p1.second && next_event_time>p1.second)
                        {
                            p1.second=next_event_time;
                        }
                    }
                    //CURRENT_TIME=CURRENT_TIME+io_burst;
                    
                    ////cout<<"io="<<io_burst<<" rem="<<proc->remainingTimeC<<endl;
                    proc->state=BLOCKED;
                    CURRENT_RUNNING_PROCESS=NULL;
                    EVENT* e1=new EVENT(next_event_time,proc,TRANS_TO_READY);
                    addEvent(e1);
                    CALL_SCHEDULER = true;
                }
                break;
            }

            if (CALL_SCHEDULER)
            {
                
                if (get_next_event_time() == CURRENT_TIME){
                    continue;
                }
                    
                CALL_SCHEDULER = false; 
                if (CURRENT_RUNNING_PROCESS == NULL)
                {
                    CURRENT_RUNNING_PROCESS = sched->get_next_process();
                    if (CURRENT_RUNNING_PROCESS == NULL)
                        continue;
                    else
                        {
                            EVENT* e=new EVENT(CURRENT_TIME, CURRENT_RUNNING_PROCESS, TRANS_TO_RUN);
                            addEvent(e);
                        }
                    // create event to make this process runnable for same time.
                }
            }
        
        }
    }
    int get_next_event_time(){
        if (!eventQ.empty()) {
        return eventQ.front()->evtTimeStamp;
    }
    return -1;  // Return -1 if no events are present
    }
};


int readInt(string s1, int &index)
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
            ////cout<<"Len:"<<len<<endl;
            string str1 = s1.substr(start, len);
            ////cout<<"Str1:"<<str1<<endl;
            num = stoi(str1);
            ////cout<<"Num:"<<num<<endl;
            ////cout<<"Index:"<<index<<endl;
            ////cout<<"s1 size:"<<s1.size()<<endl;
            return num;
        }
        else
            index++;
    }

    return num;
}


SchType findSchType(string s1)
{
    sch=s1[0];
    ////cout<<"Scheduler type:"<<sch<<endl;
    if(sch=='F' || sch=='L' || sch=='S')
    {
        ////cout<<"Don't need to read further"<<endl;
        return SchType(sch, 0, 0);
    }
    ////cout<<"s1:"<<s1<<endl;
    int index=1;
    if(index>=s1.size())
    {
         return SchType(sch, 0, 0);
    }
    int quantum=readInt(s1, index);
    int numprios=0;
    if(quantum==0)
    {
        ////cout<<"Quantum: "<<quantum<<endl;
        return SchType(sch, 0, 0);
    }
    ////cout<<"Quantum:"<<quantum<<endl;
    ////cout<<"Index: "<<index<<endl;
    ////cout<<"s1 size:"<<s1.size()<<endl;
    while(index<s1.size())
    {
        if(s1[index]==':')
        {
            index=index+1;
            numprios=readInt(s1,index);
            break;
        }
        else
        {
            while(index<s1.size() && s1[index]!=':')
            {
                index++;
            }
        }
    }
    // numprios=readInt(s1, index);
    
    numprios=(numprios!=0)?numprios:4;
    ////cout<<"Numprios:"<<numprios<<endl;
    return SchType(sch, quantum, numprios);
}


int main(int argc, char* argv[])
{
    string scheduler_spec;
    string input_file;
    string rand_file;
    bool verbose = false;
    bool trace_event = false;

    
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        //////cout<<arg<<" ";

       
        if (arg == "-h") {
            
        } else if (arg == "-v") {
            verbose = true;
        } else if (arg == "-t") {
            trace_event = true;
        } else if (checkforS(arg, "-s")) {
            scheduler_spec = arg.substr(2);  
        } else if (arg[0] != '-') {
            
            if (input_file.empty()) {
                input_file = arg;
            } else if (rand_file.empty()) {
                rand_file = arg;
            }
        }
    }

    // Check if mandatory files are provided
    if (input_file.empty() || rand_file.empty()) {
        std::cerr << "Error: input file and random file must be specified.\n";
        return 1;
    }
    Scheduler* sched=new Scheduler();
    

    SchType sch1=findSchType(scheduler_spec);
    sch=sch1.sch;
    int quantum=sch1.quantum;
        
        
    maxprios=(sch1.numprios!=0)? sch1.numprios: maxprios;
    switch (sch)
    {
    case 'F':
        sched=new FCFS_Scheduler();
        break;
    case 'S':
        sched=new SRTF_Scheduler();
        break;
    case 'L':
        sched=new LCFS_Scheduler();
        break;
    case 'R':
        sched=new RR_Scheduler();
        break;
    case 'P':
        sched=new Priority_Scheduler();
        break;
    case 'E':
        sched=new EPriority_Scheduler();
        break;
    default:
        break;
    }
    //////cout<<"Process spec:"<< sch1.sch<<" "<<sch1.quantum<<" "<<sch1.numprios<<endl;
    Simulation1 s1(sched);
    // Add main program logic here

    std::ifstream rFile(rand_file); // Open the file
    std::ifstream inputFile(input_file);
    // Check if the file opened successfully
    if (!rFile)
    {
        std::cerr << "Error: Rand File could not be opened!" << std::endl;
        return 1; // Return an error code
    }
    if (!inputFile)
    {
        std::cerr << "Error:Input File could not be opened!" << std::endl;
        return 1; // Return an error code
    }
    
    
   
    string line;
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
            randVals[count] = num;
        }
        count++;
    }
    rFile.close(); // Always close the file after use

    countP = 0;
    while (getline(inputFile, line))
    {
        int count = 0;
        int index = 0;
        vector<int> v1(4);
        ////cout<<"Reading Input file";
        while (count < 4)
        {
            ////cout<<"Count:"<<count<<" line:"<<line<<",index"<<index<<endl;
            int num = readInt(line, index);
            v1[count] = num;
            count++;
        }
        //int myrandom(int burst) { return 1 + (randvals[ofs] % burst); }
        
        ////cout<<"Maxprios:"<<maxprios<<endl;
        int static_prio=(myrandom(maxprios));
        Process *p = new Process(countP, v1[0], v1[1], v1[2], v1[3], quantum, static_prio);
        EVENT* e1= new EVENT(v1[0], p, TRANS_TO_READY);
        int size1=randVals.size();

        s1.addEvent(e1);
        countP++;
        //cout << v1[0] << " " << v1[1] << " " << v1[2] << " " << v1[3] << endl;
        //////cout<<(*(eventQ.begin()))->evtProcess->process_id<<" "<<endl;
        //eventQ.push(p);
    }
    //cout<<eventQ.size();
    auto it=eventQ.begin();
    // randVals.resize(num);
    Psummary.resize(countP);
    // while(it!=eventQ.end())
    // {
    //     EVENT* e1=(*it);
    //     //////cout<<e1->evtProcess->process_id<<" "<<e1->evtProcess->Arrival_Time<<endl;
    //     it++;
    // }
    s1.Simulation();
    ////cout<<"countP:"<<countP<<endl;
    ////cout<<"Psummary size: "<<Psummary.size()<<endl;
    ////cout<<Psummary[0]->dynamic_prio<<endl;
    ////cout<<"Scheduler Summary"<<endl;
    double total_CPU=0;
    
    
    // printf(“SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n");

    int total_Ready=0;
    int total_tt=0;
    string algo="";
    switch (sch)
    {
    case 'F':
        cout<<"FCFS"<<endl;
        break;
    case 'S':
        cout<<"SRTF"<<endl;
        break;
    case 'L':
        cout<<"LCFS"<<endl;
        break;
    case 'R':
        cout<<"RR "<<quantum<<endl;
        break;
    case 'P':
        cout<<"PRIO "<<quantum<<endl;
        break;
    case 'E':
        cout<<"PREPRIO "<<quantum<<endl;
        break;
    default:
        break;
    }


    for(int i=0;i<countP;i++)
    {
        CURRENT_RUNNING_PROCESS=Psummary[i];
        total_CPU=total_CPU+CURRENT_RUNNING_PROCESS->Total_CPU;
        //total_IO=total_IO+CURRENT_RUNNING_PROCESS->total_IO;
        total_Ready=total_Ready+CURRENT_RUNNING_PROCESS->CW;
        total_tt=total_tt+CURRENT_RUNNING_PROCESS->TT;
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", CURRENT_RUNNING_PROCESS->process_id,CURRENT_RUNNING_PROCESS->Arrival_Time, CURRENT_RUNNING_PROCESS->Total_CPU, CURRENT_RUNNING_PROCESS->CPU_Burst, CURRENT_RUNNING_PROCESS->IO_Burst, CURRENT_RUNNING_PROCESS->static_prio, CURRENT_RUNNING_PROCESS->FT, CURRENT_RUNNING_PROCESS->TT, CURRENT_RUNNING_PROCESS->total_IO, CURRENT_RUNNING_PROCESS->CW );
        //////cout<<CURRENT_RUNNING_PROCESS->process_id<<" "<<CURRENT_RUNNING_PROCESS->Arrival_Time<<" "<<CURRENT_RUNNING_PROCESS->Total_CPU<<" "<<CURRENT_RUNNING_PROCESS->CPU_Burst<<" "<<CURRENT_RUNNING_PROCESS->IO_Burst<<" "<<CURRENT_RUNNING_PROCESS->static_prio<<" | "<<CURRENT_RUNNING_PROCESS->FT<<" "<<CURRENT_RUNNING_PROCESS->TT<<" "<<CURRENT_RUNNING_PROCESS->total_IO<<" "<<CURRENT_RUNNING_PROCESS->CW<<endl;
    }
    ////cout<<"Total CPU: "<<total_CPU<<endl;
    total_IO=total_IO+(p1.second-p1.first);
    ////cout<<"Total IO: "<<total_IO<<endl;

    double CPU_Util=100.0* (total_CPU/CURRENT_TIME);
    //////cout<<"CPU_Util:"<<CPU_Util<<endl;
    //CPU_Util=(ceil(CPU_Util * 100.00) / 100.00);
    

    double IO_Util=(total_IO/CURRENT_TIME)* 100.0;
    //IO_Util=(ceil(IO_Util * 100.0) / 100.0);
    double av_TT= ((double) total_tt / countP);
    double av_throughput = 100.0 * (countP / (double) CURRENT_TIME);
    double av_cpu_waiting= ((double) total_Ready)/countP;
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", CURRENT_TIME,CPU_Util,IO_Util,av_TT,av_cpu_waiting,av_throughput);
    //av_cpu_waiting=(ceil(av_cpu_waiting * 100.0) / 100.0);
    // ////cout<<"SUM: "<<CURRENT_TIME<<" "<<fixed<< setprecision(2)<<CPU_Util<<" "<<IO_Util<<" "<<av_TT<<" "<<av_cpu_waiting<<" "<<fixed<<setprecision(3)<<av_throughput<<endl;
    
    // while(!eventQ.empty())
    // {
    //     EVENT* e1=eventQ.front();
    //     ////////cout<<"Timestamp:"<< e1->evtTimeStamp<<" Process: "<<e1->evtProcess<<" "<<" Transition:"<<e1->transition<<endl;
    //     ////////cout<<"AT:"<<e1->evtProcess->Arrival_Time<<", TC:"<<e1->evtProcess->Total_CPU<<endl;
    //     eventQ.pop();
    // }
    return 0;
}



