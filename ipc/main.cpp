#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

class Timer{
    time_t start_time;

public:
    Timer(){

    }
    void start(){
        time(&start_time);
    }

    time_t get_time(){
        time_t end_time;
        time(&end_time);
        return end_time - start_time;
    }
};

struct Student{
    int group;
    int sid;
    pthread_t tid;
    int start;
    bool leader;

    Student(int sid, int group, bool group_lead)
        : group(group), sid(sid), leader(group_lead)
    {
        start = rand() % 10 + 1;
    }
};
pthread_mutex_t print_mutex;

const int N_PS = 4; // number of print station
int PS_TIME, BS_TIME, SS_TIME; // printing, binding, submission station time

Timer timer; // for timing

vector<Student*> students; 
pthread_mutex_t ps_mutex[N_PS];
bool ps_empty[N_PS];
sem_t bs_sem;



void *studentActivity(void *arg){
    Student s = *(Student *)arg;

    // ensure randomness in start
    sleep(s.start);

    pthread_mutex_lock(&print_mutex);
    cout << "Student " << s.sid << " has arrived at the print station at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_lock(&ps_mutex[s.group%N_PS]);
    sleep(PS_TIME);

    pthread_mutex_lock(&print_mutex);
    cout << "Student " << s.sid << " has finished printing at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_unlock(&ps_mutex[s.group%N_PS]);

    if(!s.leader)
        return nullptr;
    
    for(auto i: students){
        if(i->sid != s.sid && i->group == s.group){
            pthread_join(i->tid, NULL);
        }
    }

    pthread_mutex_lock(&print_mutex);
    cout << "Group " << s.group << " has finished printing at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    // binding
    sem_wait(&bs_sem);

    pthread_mutex_lock(&print_mutex);
    cout << "Group " << s.group << " has started binding at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    sleep(BS_TIME);
    
    pthread_mutex_lock(&print_mutex);
    cout << "Group " << s.group << " has finished binding at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    sem_post(&bs_sem);
    return nullptr;
}

int main() {
    srand(1927);

    int N, M;
    cin >> N >> M;
    cin >> PS_TIME >> BS_TIME >> SS_TIME;

    for(int i = 0; i < N_PS; i++){
        ps_empty[i] = true;
        pthread_mutex_init(&ps_mutex[i], NULL);
    }

    sem_init(&bs_sem, 0, 2);

    timer.start();
    for(int i = 0; i < N; i++){
        Student *t = new Student(i, i/M, i % M == 0);
        students.push_back(t);

        pthread_create(&t->tid, NULL, studentActivity, (void *)t);
    }

    pthread_exit(NULL);
    return 0;
}
