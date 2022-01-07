# define RoundRobin 1
# define ShortestJobFirst 2
# define PriorityBased 3
# define MultiLevelFeedbackQueue 4
# define Default 5
# define Scheduling MultiLevelFeedbackQueue // variable for choosing scheduling
# define User_Q_level 7 // queue for user processes

# define QuantimSize 10

/*
* for MultiLevelFeedbackQueue
*/
# define TopUserQueue 6
# define MiddleUserQueue 7
# define LowerUserQueue 8
# define TopUserQueueQuantum 5
# define MiddleUserQueueQuantum 15
/*
* for MultiLevelFeedbackQueue if LowerUserQueueQuantum is set to 0
* LowerUserQueue will use ShortestJobFirst
*/
# define LowerUserQueueQuantum 0
/*
* for MultiLevelFeedbackQueue set rp->p_priority to the process that you wish to enter the first queue  
*/
# define FirstTime 90  

