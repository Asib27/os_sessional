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
    sem_t sem;
    int waiting_at_ps;

    Student(int sid, int group, bool group_lead)
        : group(group), sid(sid), leader(group_lead)
    {
        start = rand() % 10 + 1;
        sem_init(&sem, 0, 0);
        waiting_at_ps = -1;
    }

    ~Student(){
        sem_destroy(&sem);
    }
};
pthread_mutex_t print_mutex;

const int N_PS = 4; // number of print station
const int N_STAFF = 2;
int PS_TIME, BS_TIME, SS_TIME; // printing, binding, submission station time

Timer timer; // for timing

vector<Student*> students; 
pthread_t staff[N_STAFF];
int staff_id[N_STAFF];

pthread_mutex_t ps_mutex[N_PS];
bool ps_empty[N_PS];
sem_t bs_sem;

pthread_rwlock_t ss_rwlock;
int n_submission;


void *studentActivity(void *arg){
    Student *sptr = (Student *)arg;
    Student s = *sptr;

    // ensure randomness in start
    sleep(s.start);

    int ps = s.sid % N_PS;

    // checking lock availability
    pthread_mutex_lock(&ps_mutex[ps]);
    if(ps_empty[ps]){
        sem_post(&sptr->sem);
        ps_empty[ps] = false;
    }else{
        sptr->waiting_at_ps = ps;
    }
    pthread_mutex_unlock(&ps_mutex[ps]);

    // printing 
    pthread_mutex_lock(&print_mutex);
    cout << "Student " << s.sid << " has arrived at the print station " << ps << " at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    sem_wait(&sptr->sem);

    // started printing
    pthread_mutex_lock(&print_mutex);
    cout << "Student " << s.sid << " has started printing at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);
    sleep(PS_TIME);

    // finished printing
    pthread_mutex_lock(&print_mutex);
    cout << "Student " << s.sid << " has finished printing at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);


    pthread_mutex_lock(&ps_mutex[ps]);
    ps_empty[ps] = true;
    sptr->waiting_at_ps = -1;
    bool wakeup_done = false;
    
    // Group mates
    for(auto i: students){
        if(i->group == s.group && i->waiting_at_ps == ps){
            ps_empty[ps] = false;
            i->waiting_at_ps = -1;
            sem_post(&i->sem);
            wakeup_done = true;

            pthread_mutex_lock(&print_mutex);
            cout << "Student " << s.sid << " has called groupmate " << i->sid << endl;
            pthread_mutex_unlock(&print_mutex);

            break;

        }
    }

    // Others at the printing station
    if(!wakeup_done){
        for(auto i: students){
            if(i->group != s.group && i->waiting_at_ps == ps){
                ps_empty[ps] = false;
                i->waiting_at_ps = -1;
                sem_post(&i->sem);

                pthread_mutex_lock(&print_mutex);
                cout << "Student " << s.sid << " has called " << i->sid << endl;
                pthread_mutex_unlock(&print_mutex);

                wakeup_done = true;
                break;

            }
        }
    }

    if(!wakeup_done){
        pthread_mutex_lock(&print_mutex);
        cout << "No one to wake up by " << s.sid << " at ps " << ps << endl;
        pthread_mutex_unlock(&print_mutex);
    }
    pthread_mutex_unlock(&ps_mutex[ps]);


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


    // submission
    pthread_rwlock_wrlock(&ss_rwlock);

    sleep(SS_TIME);
    n_submission++;
    pthread_mutex_lock(&print_mutex);
    cout << "Group " << s.group << " has submitted the report at time " << timer.get_time() << endl;
    pthread_mutex_unlock(&print_mutex);

    pthread_rwlock_unlock(&ss_rwlock);
    return nullptr;
}

void *staff_activity(void * arg){
    int staff = *(int *)arg + 1;
    while(true){
        int random = rand() % 10 + 3;
        sleep(random);

        pthread_rwlock_rdlock(&ss_rwlock);

        pthread_mutex_lock(&print_mutex);
        cout << "Staff " << staff << " has started reading the entry book at time "  
            << timer.get_time() << ". No. of submission = " << n_submission << endl;
        pthread_mutex_unlock(&print_mutex);

        pthread_rwlock_unlock(&ss_rwlock);
    }
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
    pthread_rwlock_init(&ss_rwlock, NULL);

    timer.start();
    for(int i = 0; i < N; i++){
        Student *t = new Student(i, i/M, i % M == M-1);
        students.push_back(t);

        pthread_create(&t->tid, NULL, studentActivity, (void *)t);
    }

    for(int i = 0; i < N_STAFF; i++){
        staff_id[i] = i;
        pthread_create(&staff[i], NULL, staff_activity, &staff_id[i]);
    }

    for(auto i: students){
        if(i->leader)
            pthread_join(i->tid, NULL);
    }
    for(int i = 0; i < N_STAFF; i++){
        pthread_cancel(staff[i]);
    }
    cout << "All thread finished" << endl;
    return 0;
}
