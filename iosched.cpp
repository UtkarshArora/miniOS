#include<iostream>
#include<fstream>
#include<climits>
#include<sstream>
#include<deque>
#include<queue>
#include<algorithm> 
using namespace std;

class IORequest {
    public:
        int id;         // Request ID
        int arrival;    // Arrival time
        int track;      // Requested track
        int start ; // Start time of disk operation
        int end ;   // End time of disk operation
        int waitTime;
        //int turnAround;
    IORequest(int id, int arrival, int track)
    {
        this->id=id;
        this->arrival=arrival;
        this->track=track;
        start =-1;
        end = -1;
        this->waitTime=0;
        //this->waitTime=0;
        //this->turnAround=0;        
    }
};


IORequest* CURRENT_ACTIVE_REQUEST=NULL;

int currentTime = 0;
int currentTrack = 0;
int requestedTrack = 0;
int totalHeadMovements = 0;
double totalWaitTime = 0;
double totalTurnAround = 0;
int max_waittime = 0;
double totalIO=0;
int total_req=0;
int processedRequests=0;
char direction = '*';
char algo_spec = 'N';
//int activeRequest = -1;
deque<IORequest*> requests;
//vector<IORequest*>sortedRequests;
deque<IORequest*> IOqueue;
vector<IORequest*>completed;

class Scheduler
{
    public:
        virtual IORequest* next_instr()=0;
        virtual void addRequest(IORequest* r){};
        virtual int getQueuesize() = 0;

};

class FIFO_Scheduler : public Scheduler{
    public:
        
        FIFO_Scheduler()
        {
            
        }
        virtual void addRequest (IORequest* r)
        {

        }
        int getQueuesize()
        {   
            return IOqueue.size();
        }
        IORequest* next_instr()
        {
            IORequest* op=IOqueue.front();
            IOqueue.pop_front();
            ////cout<<"Inside FIFO, op id"<<op->id<<endl;
            return op;
        }

};

class SSTF_Scheduler : public Scheduler{
    public:
        int lowest;
        IORequest* op;
        SSTF_Scheduler()
        {
            lowest=INT_MAX;
            op=NULL;
        }
        virtual void addRequest (IORequest* r)
        {
            
        }
        int getQueuesize()
        {   
            return IOqueue.size();
        }
        IORequest* next_instr()
        {
            auto it=IOqueue.begin();
            auto it1=IOqueue.begin();
            lowest=INT_MAX;
            ////cout<<"Queue size: "<<IOqueue.size()<<endl;
            for(; it!=IOqueue.end();it++)
            {
                
                IORequest* io= *(it);
                
                    int diff=abs(currentTrack - io->track);
                    if(diff<lowest)
                    {
                        
                        op = *it;
                        it1=it;
                        lowest=diff;
                        ////cout<<"lowest:"<<lowest<<endl;
                    }
            }
            op=*it1;
            IOqueue.erase(it1);
            ////cout<<"SSTF,id: "<<op->id<<endl;
            return op;
        }
        
            
            // IORequest* op = requests[opIndex];
            // requests[opIndex] = NULL;
            // for(auto it=IOqueue.begin(); it!= IOqueue.end();it++)
            // {

            // }
        };

void checkPositive()
{

}
class LOOK_Scheduler : public Scheduler{
    public:
        int poslowest;
        int neglowest;
        IORequest* op;
        LOOK_Scheduler()
        {
            poslowest=INT_MAX;
            neglowest=INT_MAX;
            op=NULL;
        }
        int getQueuesize()
        {   
            return IOqueue.size();
        }
        IORequest* next_instr()
        {
            auto it=IOqueue.begin();
            auto it1=IOqueue.begin();
            auto it2=IOqueue.begin();
            poslowest=INT_MAX;
            neglowest=INT_MAX;
            ////cout<<"Queue size: "<<IOqueue.size()<<endl;
            if(IOqueue.size()==1)
                {
                    op = *it1;
                    if(op->track > currentTrack)
                    {
                        direction = '+';
                        IOqueue.erase(it1);
                        ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if(op->track < currentTrack)
                    {
                        direction = '-';
                        IOqueue.erase(it1);
                        ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if( op->track == currentTrack )
                    {
                        IOqueue.erase(it1);
                        
                        return op;
                    }
                    
                }


                for(; it!=IOqueue.end();it++)
                {
                    
                    IORequest* io= *(it);
                        
                            if(io->track > currentTrack)
                            {
                                int diff =  io->track - currentTrack;
                                if(diff < poslowest)
                                {
                                    it1=it;
                                    poslowest=diff;
                                } 
                            }
                            else if( io->track < currentTrack )
                            {
                                int diff =  currentTrack - io->track;
                                if(diff < neglowest)
                                {
                                    it2=it;
                                    neglowest=diff;
                                } 
                            }
                            else
                            {
                                ////cout<<"Requested=current, Id: "<<io->id<<endl;
                                IOqueue.erase(it);
                                return io;
                            }
                        
                }
                
            
            if(direction == '+')
            {
                if(poslowest!=INT_MAX)
                {
                    op = *it1;
                    IOqueue.erase(it1);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
                else
                {
                    direction = '-';
                    op = *it2;
                    IOqueue.erase(it2);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
            }
            else
            {
                if(neglowest!=INT_MAX)
                {
                    op = *it2;
                    IOqueue.erase(it2);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
                else
                {
                    direction = '+';
                    op = *it1;
                    IOqueue.erase(it1);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
            }

            //op=*it1;
            
            ////cout<<"SSTF,id: "<<op->id<<endl;

            return op;
        }

};

class CLOOK_Scheduler : public Scheduler{
    public:
        int poslowest; 
        int negfarthest;
        IORequest* op;

        CLOOK_Scheduler()
        {
            poslowest=INT_MAX;
            negfarthest=INT_MIN;
            op=NULL;
        }
        int getQueuesize()
        {   
            return IOqueue.size();
        }
        IORequest* next_instr()
        {
            poslowest=INT_MAX;
            negfarthest=INT_MIN;
            if(direction == '-')
            {
                direction = '+';
            }
            auto it=IOqueue.begin();
            auto it1=IOqueue.begin();
            auto it2=IOqueue.begin();

                if(IOqueue.size()==1)
                {
                    op = *it1;
                    if(op->track > currentTrack)
                    {
                        direction = '+';
                        IOqueue.erase(it1);
                        ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if(op->track < currentTrack)
                    {
                        direction = '-';
                        IOqueue.erase(it1);
                        ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if( op->track == currentTrack )
                    {
                        IOqueue.erase(it1);
                        
                        return op;
                    }
                    
                }

            for( ;it!=IOqueue.end();it++)
            {

                            IORequest* io= *(it);
                        
                            if(io->track > currentTrack)
                            {
                                int diff =  io->track - currentTrack;
                                if(diff < poslowest)
                                {
                                    it1=it;
                                    poslowest=diff;
                                } 
                            }
                            else if( io->track < currentTrack )
                            {
                                int diff =  currentTrack - io->track;
                                if(diff > negfarthest)
                                {
                                    it2=it;
                                    negfarthest=diff;
                                } 
                            }
                            else
                            {
                                ////cout<<"Requested=current, Id: "<<io->id<<endl;
                                IOqueue.erase(it);
                                return io;
                            }
            }

            if(direction == '+')
            {
                if(poslowest!=INT_MAX)
                {
                    op = *it1;
                    IOqueue.erase(it1);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
                else
                {
                    direction = '-';
                    op = *it2;
                    IOqueue.erase(it2);
                    ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
            }
            // else
            // {
            //     if(negfarthest!=INT_MIN)
            //     {
            //         op = *it2;
            //         IOqueue.erase(it2);
            //         ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
            //         return op;
            //     }
            //     else
            //     {
            //         direction = '+';
            //         op = *it1;
            //         IOqueue.erase(it1);
            //         ////cout<<"Direction: "<<direction<<" "<<op->id<<endl;
            //         return op;
            //     }
            // }
	return NULL;
        }

};

class FLOOK_Scheduler : public Scheduler{
    public:
       
        IORequest* op;
        deque<IORequest*>addQ;
        deque<IORequest*>activeQ;
        deque<IORequest*>temp;
        //deque<IORequest*>temp1;
        int poslowest;
        int neglowest;
        //IORequest* op;
        FLOOK_Scheduler()
        {
            poslowest=INT_MAX;
            neglowest=INT_MAX;
            //activeQ=&temp1;
            //addQ=&IOqueue;
            op=NULL;
        }
        int getQueuesize()
        {   
            if (activeQ.size() > 0 || addQ.size() > 0){
                return 1;
            } 
            else   
            {
                return 0;
            } 
                
        }
        void addRequest(IORequest* r)
        {
            //cout<<addQ.size()<<endl;
            //cout<<"Add request"<<endl;
            //cout<<"Id:"<<r->id<<endl;
            addQ.push_back(r);
            //cout<<"AddQ size" << addQ.size()<<endl;
        }

        // int getQueuesize()
        // {
        //     return addQ.size();
        // }
        IORequest* next_instr()
        {
            //cout<<"Current time:"<<currentTime<<", IOQueue size: "<<IOqueue.size()<<",activeQ size:"<<activeQ.size()<<", addQ size:"<<addQ.size()<<endl;
            if(activeQ.empty())
            {
                // deque<IORequest*>*temp;
                // temp=activeQ;
                // activeQ=addQ;
                // addQ=activeQ;
                swap(addQ, activeQ);
                direction='+';
            }
            //cout<<"ActiveQ size "<<activeQ.size()<<endl;
            if(activeQ.size()==0)
                return NULL;
            poslowest=INT_MAX;
            neglowest=INT_MAX;
            auto it=activeQ.begin();
            auto it1=activeQ.begin();
            auto it2=activeQ.begin();

            if(activeQ.size()==1)
                {
                    op = *it1;
                    if(op->track > currentTrack)
                    {
                        direction = '+';
                        activeQ.erase(it1);
                        //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if(op->track < currentTrack)
                    {
                        direction = '-';
                        activeQ.erase(it1);
                        //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                        return op;
                    }
                    else if( op->track == currentTrack )
                    {
                        activeQ.erase(it1);
                        
                        return op;
                    }
                    
                }


                for(; it!=activeQ.end();it++)
                {
                    
                    IORequest* io= *(it);
                        
                            if(io->track > currentTrack)
                            {
                                int diff =  io->track - currentTrack;
                                if(diff < poslowest)
                                {
                                    it1=it;
                                    poslowest=diff;
                                } 
                            }
                            else if( io->track < currentTrack )
                            {
                                int diff =  currentTrack - io->track;
                                if(diff < neglowest)
                                {
                                    it2=it;
                                    neglowest=diff;
                                } 
                            }
                            else
                            {
                                ////cout<<"Requested=current, Id: "<<io->id<<endl;
                                activeQ.erase(it);
                                return io;
                            }
                        
                }
                
            
            if(direction == '+')
            {
                if(poslowest!=INT_MAX)
                {
                    op = *it1;
                    activeQ.erase(it1);
                    //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
                else
                {
                    direction = '-';
                    op = *it2;
                    activeQ.erase(it2);
                    //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
            }
            else
            {
                if(neglowest!=INT_MAX)
                {
                    op = *it2;
                    activeQ.erase(it2);
                    //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
                else
                {
                    direction = '+';
                    op = *it1;
                    activeQ.erase(it1);
                    //cout<<"Direction: "<<direction<<" "<<op->id<<endl;
                    return op;
                }
            }

            //op=*it1;
            
            ////cout<<"SSTF,id: "<<op->id<<endl;

            return op;
        }
            
            

            //op=*it1;
            
            ////cout<<"SSTF,id: "<<op->id<<endl;

            
        };




void parseInput(const std::string &filename) {
    ifstream inputFile(filename);
    if (!inputFile) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        exit(1);
    }

    string line;
    int id = 0;
    while (getline(inputFile, line)) {
        if (line.empty() || line[0] == '#') continue; 
        istringstream iss(line);
        int arrival, track;
        if (iss >> arrival >> track) {
            IORequest* io=new IORequest(id, arrival, track);
            id++;
            requests.push_back(io);
        }
    }
    total_req=requests.size();
    completed.resize(total_req);
    inputFile.close();
}

void RequestCompleted(Scheduler* s)
{
                processedRequests++;
                CURRENT_ACTIVE_REQUEST->end=currentTime;
                int id=CURRENT_ACTIVE_REQUEST->id;
                completed[id]=CURRENT_ACTIVE_REQUEST;
                ////cout<<"REQUEST Id:"<<CURRENT_ACTIVE_REQUEST->id<<" COMPLETED , current time:"<<currentTime<<endl;
                totalIO += (CURRENT_ACTIVE_REQUEST->end - CURRENT_ACTIVE_REQUEST->start);
                totalTurnAround += currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                CURRENT_ACTIVE_REQUEST=NULL; 
                
                if( (!IOqueue.empty() && algo_spec!= 'F') || (s->getQueuesize()>0 && algo_spec=='F' ))
                {
                    IORequest* io = s->next_instr();
                    CURRENT_ACTIVE_REQUEST=io;
                    ////cout<<"io Id:"<<io->id<<",current time:"<<currentTime<<endl;
                    io->start=currentTime;
                    requestedTrack = io->track;
                    int waitTime = currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                    max_waittime = max(waitTime, max_waittime); 
                    totalWaitTime += currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                    if(requestedTrack>currentTrack)
                    {
                        currentTrack++;
                        totalHeadMovements++;
                    }
                    else if(requestedTrack<currentTrack)
                    {
                        currentTrack--;
                        totalHeadMovements++;
                    }
                    else
                    {
                        ////cout<<"ID:"<<CURRENT_ACTIVE_REQUEST->id<<endl;
                        RequestCompleted(s);
                    }
                }
                // else if(s->getQueuesize()>0 && algo_spec=='F')
                // {
                //     IORequest* io = s->next_instr();
                //     CURRENT_ACTIVE_REQUEST=io;
                //     ////cout<<"io Id:"<<io->id<<",current time:"<<currentTime<<endl;
                //     io->start=currentTime;
                //     requestedTrack = io->track;
                //     int waitTime = currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                //     max_waittime = max(waitTime, max_waittime); 
                //     totalWaitTime += currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                //     if(requestedTrack>currentTrack)
                //     {
                //         currentTrack++;
                //         totalHeadMovements++;
                //     }
                //     else if(requestedTrack<currentTrack)
                //     {
                //         currentTrack--;
                //         totalHeadMovements++;
                //     }
                //     else
                //     {
                //         ////cout<<"ID:"<<CURRENT_ACTIVE_REQUEST->id<<endl;
                //         RequestCompleted(s);
                //     }
                // }
                return;
}

void simulation(Scheduler* s)
{
    
    ////cout<<"Inside simulation"<<endl;
    while(1)
    {
        ////cout<<"Current time:"<<currentTime<<endl;
        if(requests.size()>0 && requests.front()->arrival==currentTime)
        {
            if(algo_spec == 'F')
            {
                ////cout<<"Adding request condition"<<endl;
                s->addRequest(requests.front());
            }
            else
            {
                IOqueue.push_back(requests.front());
            }
            ////cout<<"Added 1st element to IO queue:"<<requests.front()->id<<", current time:"<<currentTime<<endl;
            requests.pop_front();
            
        }
        if(CURRENT_ACTIVE_REQUEST==NULL && ( (!IOqueue.empty() && algo_spec!='F') ||  (algo_spec == 'F' && s->getQueuesize()>0)) )
        {
            
                IORequest* io = s->next_instr();
                CURRENT_ACTIVE_REQUEST=io;
                ////cout<<"io Id:"<<io->id<<",current time:"<<currentTime<<endl;
                io->start=currentTime;
                requestedTrack = io->track;
                int waitTime = currentTime - CURRENT_ACTIVE_REQUEST->arrival;
                max_waittime = max(waitTime, max_waittime); 
                totalWaitTime += currentTime - CURRENT_ACTIVE_REQUEST->arrival;

                if(requestedTrack>currentTrack)
                {
                    currentTrack++;
                    totalHeadMovements++;
                }
                else if(requestedTrack<currentTrack)
                {
                    currentTrack--;
                    totalHeadMovements++;
                }
                else
                {
                    RequestCompleted(s);
                }
        }
        // if(CURRENT_ACTIVE_REQUEST == NULL && algo_spec == 'F' && s->getQueuesize()>0)
        // {
        //     IORequest* io = s->next_instr();
        //     // if(io==NULL)
        //     //     continue;
        //     CURRENT_ACTIVE_REQUEST=io;
        //     ////cout<<"io Id:"<<io->id<<",current time:"<<currentTime<<endl;
        //     io->start=currentTime;
        //     requestedTrack = io->track;
        //     int waitTime = currentTime - CURRENT_ACTIVE_REQUEST->arrival;
        //     max_waittime = max(waitTime, max_waittime); 
        //     totalWaitTime += currentTime - CURRENT_ACTIVE_REQUEST->arrival;

        //     if(requestedTrack>currentTrack)
        //     {
        //         currentTrack++;
        //         totalHeadMovements++;
        //     }
        //     else if(requestedTrack<currentTrack)
        //     {
        //         currentTrack--;
        //         totalHeadMovements++;
        //     }
        //     else
        //     {
        //         RequestCompleted(s);
        //     }
        // }
        else if(CURRENT_ACTIVE_REQUEST!=NULL){

            if(currentTrack < requestedTrack)
            {
                totalHeadMovements++;
                currentTrack++;
            }
            else if(requestedTrack < currentTrack)
            {
                totalHeadMovements++;
                currentTrack--;
            }
            else if(currentTrack == requestedTrack)
            {
                RequestCompleted(s);
            }
        }
        
            if(CURRENT_ACTIVE_REQUEST == NULL && requests.empty() && IOqueue.empty())
            {
                ////cout<<"Total IO: "<<totalIO<<endl;
                break;
            }
        
        currentTime++;
    }
}


bool checkforAlgos(string& str, const string& prefix)
{
    return str.find(prefix) != string::npos;
}

void printProcSummary()
{
    for(int i=0;i<completed.size();i++)
    {
        IORequest* req=completed[i];
        printf("%5d: %5d %5d %5d\n", i, req->arrival, req->start, req->end);
    }
}

void printStats()
{
    int total_req=completed.size();
    printf("SUM: %d %d %.4lf %.2lf %.2lf %d\n", currentTime, totalHeadMovements,(totalIO/currentTime),  (totalTurnAround/total_req), (totalWaitTime/total_req), max_waittime);
}

int main(int argc, char* argv[])
{
    string input_file;
    string rand_file;
    //char algo_spec='N';
    ////cout<<"Argc:"<<argc<<endl;
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        ////cout<<"Arg:"<<arg<<endl;
       if (checkforAlgos(arg, "-s")) {
            if(arg.size()>2)
                {
                    algo_spec = arg.substr(2)[0];  
                    ////cout<<"Algo"<<algo_spec;
                }
        } 
        else if (arg[0] != '-') {
            
            if (input_file.empty()) {
                input_file = arg;
            } else if (rand_file.empty()) {
                rand_file = arg;
            }
        }
    }
    Scheduler* s=NULL;
    switch (algo_spec)
                    {
                    case 'S':
                        {
                            s=new SSTF_Scheduler();
                        }
                        break;
                    case 'N':
                        {
                            s=new FIFO_Scheduler();
                        }
                        break;
                    case 'L':
                        {
                            s=new LOOK_Scheduler();
                        }
                        break;
                    case 'C':
                        {
                            s=new CLOOK_Scheduler();
                        }
                        break;
                    case 'F':
                        {
                            s=new FLOOK_Scheduler();
                        }
                        break;
                    
                    default:
                        break;
                    }


    
    if (input_file.empty() ) {
       cerr << "Error: input file must be specified.\n";
        return -1;
    }
    parseInput(input_file);
    // for(auto it=requests.begin();it!=requests.end();it++)
    // {
    //     IORequest* io = *it;
    //     //cout<<"Arrival:"<<io->arrival<<" "<<"Track:"<<io->track<<" "<<"Id:"<<io->id<<" "<<"Start: "<<io->start<<" End:"<<io->end<<endl;
    // }
    // //cout<<"Exited from loop"<<endl;
    //return 0;
    simulation(s);
    printProcSummary();
    printStats();

}
