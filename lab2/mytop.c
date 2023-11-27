#include<stdio.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<string.h>
#include<stdlib.h>
//宏定义最大进程数量和最大进程名
#define MAX_PROCESS_NUMS    256
#define MAX_COMM_LEN    16

int period = 1;//刷新时间的间隔
char name[MAX_COMM_LEN * MAX_PROCESS_NUMS] = {0};
int pid[MAX_PROCESS_NUMS] = {0};
int last_pid[MAX_PROCESS_NUMS] = {0};//记录上一次的pid
int runningstate[MAX_PROCESS_NUMS] = {0};
unsigned long long cputime[MAX_PROCESS_NUMS] = {0};
double time[MAX_PROCESS_NUMS] = {0};//将ns转换为s
double last_time[MAX_PROCESS_NUMS] = {0};//记录上一次的时间
double cpu_usage[MAX_PROCESS_NUMS] = {0};

void swap(int pid[], char name[], int runningstate[], unsigned long long cputime[], double cpu_usage[], double time[],double last_time[], int i, int j)//交换i和j的位置
{
    int tmp = pid[i];//交换pid
    pid[i] = pid[j];
    pid[j] = tmp;
    char tmp2;//交换进程名
    for (int k = 0; k < MAX_COMM_LEN; k++)
    {
        tmp2 = name[MAX_COMM_LEN * i + k];
        name[MAX_COMM_LEN * i + k] = name[MAX_COMM_LEN * j + k];
        name[MAX_COMM_LEN * j + k] = tmp2;
    }
    int tmp3 = runningstate[i];
    runningstate[i] = runningstate[j];
    runningstate[j] = tmp3;
    unsigned long long tmp4 = cputime[i];
    cputime[i] = cputime[j];
    cputime[j] = tmp4;
    double tmp5 = cpu_usage[i];
    cpu_usage[i] = cpu_usage[j];
    cpu_usage[j] = tmp5;
    double tmp6 = time[i];
    time[i] = time[j];
    time[j] = tmp6;
    double tmp7 = last_time[i];
    last_time[i] = last_time[j];
    last_time[j] = tmp7;
}

int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "-d") == 0))//处理输入,判断是否要调整刷新的时间间隔
    {
        if (argc > 3)
        {
            printf("mytop: too many arguments");//参数过多
            exit(-1);
        }
        period = atoi(argv[2]);//更改刷新时间
    }
    int flag = 0;
    while (1)
    {
        int count = 0;
        count = syscall(332, name, pid, runningstate, cputime);//count记录进程数
        for (int i = 0; i < count; i++)
        {
            time[i] = (cputime[i] * 1.0) / 1.0e9;//将cpu时间转换成s为单位
        }
        for (int i = 0; i < count; i++)
        {
            for (int j = i; j < count; j++)
            {
                if (pid[i] == last_pid[j])//如果上一pid组的值和当前pid组的值相等,则上一组的last_time为当前组的last_time,同时还要交换pid
                {
                    double t = last_time[i];
                    last_time[i] = last_time[j];
                    last_time[j] = t;
                    int t1 = last_pid[j];
                    last_pid[j] = last_pid[i];
                    last_pid[i] = t1;
                }
            }
        }
        for (int i = 0; i < count; i++)//计算cpu占用率
        {
            cpu_usage[i] = ((time[i] - last_time[i]) / period) * 100.0;
        }
        for (int i = 0; i < count - 1; i++)//排序
        {
            for (int j = i + 1; j < count; j++)
            {
                if (cpu_usage[i] < cpu_usage[j])
                {
                    swap(pid, name, runningstate, cputime, cpu_usage, time, last_time, i, j);
                }
            }
        }
        if (flag != 0)
        {
            printf("PID\tISRUNNING\tCPU\tTIME\tCOMM\n");//输出cpu占用率前20的进程信息
            for (int i = 0; i < ((count > 20) ? 20 : count); i++)
            {
                printf(" %d\t",pid[i]);
                printf("%d\t", runningstate[i]);
                printf("%lf%%\t", cpu_usage[i]);
                printf("%lf\t", time[i]);
                for (int j = 0; j < MAX_COMM_LEN; j++)
                {
                printf("%c", name[i * MAX_COMM_LEN + j]);
                }
                printf("\n");
            }
        }
        memcpy(last_time, time, MAX_PROCESS_NUMS * sizeof(double));
        memcpy(last_pid, pid , MAX_PROCESS_NUMS * sizeof(int));
        if (flag != 0)
            sleep(period);
        flag = 1;
        system("clear");
    }
}